-- Supplemental PostgreSQL constraints and triggers for Task Manager.
--
-- Recommended workflow:
-- 1. Generate the initial migration:
--      npx prisma migrate dev --name init --create-only
-- 2. Append this file to the generated migration.sql, or apply it as a second
--    manually-created migration after the tables have been created.
-- 3. Run:
--      npx prisma migrate dev
--
-- Prisma represents relations, enums, indexes, and the active-timer partial
-- unique index. This file adds cross-row and CHECK constraints that are not
-- fully expressible in Prisma Schema Language.

-- ---------------------------------------------------------------------------
-- Scalar and range constraints
-- ---------------------------------------------------------------------------

ALTER TABLE "users"
  ADD CONSTRAINT "users_email_lowercase_check"
  CHECK ("email" = lower(btrim("email")));

ALTER TABLE "categories"
  ADD CONSTRAINT "categories_position_nonnegative_check"
  CHECK ("position" >= 0),
  ADD CONSTRAINT "categories_slug_format_check"
  CHECK ("slug" ~ '^[a-z0-9]+(?:-[a-z0-9]+)*$'),
  ADD CONSTRAINT "categories_color_format_check"
  CHECK ("color" IS NULL OR "color" ~ '^#[0-9A-Fa-f]{6}$');

ALTER TABLE "projects"
  ADD CONSTRAINT "projects_date_range_check"
  CHECK (
    "start_date" IS NULL
    OR "due_date" IS NULL
    OR "start_date" <= "due_date"
  );

ALTER TABLE "milestones"
  ADD CONSTRAINT "milestones_position_nonnegative_check"
  CHECK ("position" >= 0);

ALTER TABLE "tasks"
  ADD CONSTRAINT "tasks_position_nonnegative_check"
  CHECK ("position" >= 0),
  ADD CONSTRAINT "tasks_estimated_minutes_nonnegative_check"
  CHECK ("estimated_minutes" IS NULL OR "estimated_minutes" >= 0),
  ADD CONSTRAINT "tasks_date_range_check"
  CHECK (
    "start_date" IS NULL
    OR "due_date" IS NULL
    OR "start_date" <= "due_date"
  ),
  ADD CONSTRAINT "tasks_completed_at_status_check"
  CHECK (
    ("status" = 'COMPLETED' AND "completed_at" IS NOT NULL)
    OR
    ("status" <> 'COMPLETED' AND "completed_at" IS NULL)
  );

ALTER TABLE "tags"
  ADD CONSTRAINT "tags_normalized_name_check"
  CHECK (
    "normalized_name" = lower(btrim("normalized_name"))
    AND length("normalized_name") > 0
  ),
  ADD CONSTRAINT "tags_color_format_check"
  CHECK ("color" IS NULL OR "color" ~ '^#[0-9A-Fa-f]{6}$');

ALTER TABLE "time_records"
  ADD CONSTRAINT "time_records_range_and_duration_check"
  CHECK (
    (
      "ended_at" IS NULL
      AND "duration_minutes" IS NULL
    )
    OR
    (
      "ended_at" IS NOT NULL
      AND "duration_minutes" IS NOT NULL
      AND "ended_at" > "started_at"
      AND "duration_minutes" >= 0
    )
  );

ALTER TABLE "project_files"
  ADD CONSTRAINT "project_files_size_nonnegative_check"
  CHECK ("size_bytes" >= 0);

ALTER TABLE "notification_settings"
  ADD CONSTRAINT "notification_settings_reminder_minutes_check"
  CHECK (
    "due_date_reminder_minutes" >= 0
    AND "due_date_reminder_minutes" <= 10080
  ),
  ADD CONSTRAINT "notification_settings_daily_time_check"
  CHECK (
    "daily_summary_time" ~ '^([01][0-9]|2[0-3]):[0-5][0-9]$'
  ),
  ADD CONSTRAINT "notification_settings_enabled_reminder_check"
  CHECK (
    NOT "due_date_reminder_enabled"
    OR "due_date_reminder_minutes" > 0
  );

-- ---------------------------------------------------------------------------
-- Project ↔ Category ownership
-- Ensures a project cannot reference another user's category.
-- ---------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION validate_project_category_owner()
RETURNS trigger
LANGUAGE plpgsql
AS $$
DECLARE
  category_user_id uuid;
BEGIN
  SELECT "user_id"
    INTO category_user_id
    FROM "categories"
   WHERE "id" = NEW."category_id";

  IF category_user_id IS NULL THEN
    RAISE EXCEPTION
      USING ERRCODE = '23503',
            MESSAGE = 'Referenced category does not exist';
  END IF;

  IF category_user_id <> NEW."user_id" THEN
    RAISE EXCEPTION
      USING ERRCODE = '23514',
            MESSAGE = 'Project and category must belong to the same user';
  END IF;

  RETURN NEW;
END;
$$;

CREATE TRIGGER "projects_validate_category_owner_trigger"
BEFORE INSERT OR UPDATE OF "user_id", "category_id"
ON "projects"
FOR EACH ROW
EXECUTE FUNCTION validate_project_category_owner();

-- ---------------------------------------------------------------------------
-- Task hierarchy and Task ↔ Milestone project consistency
-- - parent task must be in the same project
-- - hierarchy depth is limited to one subtask level
-- - a task with children cannot be moved under another task
-- - milestone must be in the same project
-- ---------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION validate_task_relations()
RETURNS trigger
LANGUAGE plpgsql
AS $$
DECLARE
  parent_project_id uuid;
  parent_parent_task_id uuid;
  milestone_project_id uuid;
BEGIN
  IF NEW."parent_task_id" IS NOT NULL THEN
    IF NEW."parent_task_id" = NEW."id" THEN
      RAISE EXCEPTION
        USING ERRCODE = '23514',
              MESSAGE = 'A task cannot be its own parent';
    END IF;

    SELECT "project_id", "parent_task_id"
      INTO parent_project_id, parent_parent_task_id
      FROM "tasks"
     WHERE "id" = NEW."parent_task_id";

    IF parent_project_id IS NULL THEN
      RAISE EXCEPTION
        USING ERRCODE = '23503',
              MESSAGE = 'Referenced parent task does not exist';
    END IF;

    IF parent_project_id <> NEW."project_id" THEN
      RAISE EXCEPTION
        USING ERRCODE = '23514',
              MESSAGE = 'Parent task must belong to the same project';
    END IF;

    IF parent_parent_task_id IS NOT NULL THEN
      RAISE EXCEPTION
        USING ERRCODE = '23514',
              MESSAGE = 'Task hierarchy depth cannot exceed one subtask level';
    END IF;

    IF EXISTS (
      SELECT 1
        FROM "tasks"
       WHERE "parent_task_id" = NEW."id"
    ) THEN
      RAISE EXCEPTION
        USING ERRCODE = '23514',
              MESSAGE = 'A task with subtasks cannot become a subtask';
    END IF;
  END IF;

  IF NEW."milestone_id" IS NOT NULL THEN
    SELECT "project_id"
      INTO milestone_project_id
      FROM "milestones"
     WHERE "id" = NEW."milestone_id";

    IF milestone_project_id IS NULL THEN
      RAISE EXCEPTION
        USING ERRCODE = '23503',
              MESSAGE = 'Referenced milestone does not exist';
    END IF;

    IF milestone_project_id <> NEW."project_id" THEN
      RAISE EXCEPTION
        USING ERRCODE = '23514',
              MESSAGE = 'Task and milestone must belong to the same project';
    END IF;
  END IF;

  RETURN NEW;
END;
$$;

CREATE TRIGGER "tasks_validate_relations_trigger"
BEFORE INSERT OR UPDATE OF "project_id", "milestone_id", "parent_task_id"
ON "tasks"
FOR EACH ROW
EXECUTE FUNCTION validate_task_relations();

-- Prevent converting an existing parent into a subtask by updating the child
-- hierarchy in a way that bypasses the row being updated.
CREATE OR REPLACE FUNCTION prevent_grandchild_tasks()
RETURNS trigger
LANGUAGE plpgsql
AS $$
BEGIN
  IF NEW."parent_task_id" IS NOT NULL AND EXISTS (
    SELECT 1
      FROM "tasks" AS parent
     WHERE parent."id" = NEW."parent_task_id"
       AND parent."parent_task_id" IS NOT NULL
  ) THEN
    RAISE EXCEPTION
      USING ERRCODE = '23514',
            MESSAGE = 'Task hierarchy depth cannot exceed one subtask level';
  END IF;

  RETURN NEW;
END;
$$;

CREATE TRIGGER "tasks_prevent_grandchild_trigger"
BEFORE INSERT OR UPDATE OF "parent_task_id"
ON "tasks"
FOR EACH ROW
EXECUTE FUNCTION prevent_grandchild_tasks();

-- ---------------------------------------------------------------------------
-- Notes must reference a task in the same project.
-- ---------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION validate_note_task_project()
RETURNS trigger
LANGUAGE plpgsql
AS $$
DECLARE
  task_project_id uuid;
BEGIN
  IF NEW."task_id" IS NULL THEN
    RETURN NEW;
  END IF;

  SELECT "project_id"
    INTO task_project_id
    FROM "tasks"
   WHERE "id" = NEW."task_id";

  IF task_project_id IS NULL THEN
    RAISE EXCEPTION
      USING ERRCODE = '23503',
            MESSAGE = 'Referenced task does not exist';
  END IF;

  IF task_project_id <> NEW."project_id" THEN
    RAISE EXCEPTION
      USING ERRCODE = '23514',
            MESSAGE = 'Note task must belong to the same project';
  END IF;

  RETURN NEW;
END;
$$;

CREATE TRIGGER "notes_validate_task_project_trigger"
BEFORE INSERT OR UPDATE OF "project_id", "task_id"
ON "notes"
FOR EACH ROW
EXECUTE FUNCTION validate_note_task_project();

-- ---------------------------------------------------------------------------
-- Time record ownership and Task ↔ Project consistency
-- ---------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION validate_time_record_relations()
RETURNS trigger
LANGUAGE plpgsql
AS $$
DECLARE
  project_user_id uuid;
  task_project_id uuid;
BEGIN
  SELECT "user_id"
    INTO project_user_id
    FROM "projects"
   WHERE "id" = NEW."project_id";

  IF project_user_id IS NULL THEN
    RAISE EXCEPTION
      USING ERRCODE = '23503',
            MESSAGE = 'Referenced project does not exist';
  END IF;

  IF project_user_id <> NEW."user_id" THEN
    RAISE EXCEPTION
      USING ERRCODE = '23514',
            MESSAGE = 'Time record and project must belong to the same user';
  END IF;

  IF NEW."task_id" IS NOT NULL THEN
    SELECT "project_id"
      INTO task_project_id
      FROM "tasks"
     WHERE "id" = NEW."task_id";

    IF task_project_id IS NULL THEN
      RAISE EXCEPTION
        USING ERRCODE = '23503',
              MESSAGE = 'Referenced task does not exist';
    END IF;

    IF task_project_id <> NEW."project_id" THEN
      RAISE EXCEPTION
        USING ERRCODE = '23514',
              MESSAGE = 'Time record task must belong to the same project';
    END IF;
  END IF;

  RETURN NEW;
END;
$$;

CREATE TRIGGER "time_records_validate_relations_trigger"
BEFORE INSERT OR UPDATE OF "user_id", "project_id", "task_id"
ON "time_records"
FOR EACH ROW
EXECUTE FUNCTION validate_time_record_relations();

-- ---------------------------------------------------------------------------
-- Tag ownership consistency
-- ---------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION validate_project_tag_owner()
RETURNS trigger
LANGUAGE plpgsql
AS $$
DECLARE
  project_user_id uuid;
  tag_user_id uuid;
BEGIN
  SELECT "user_id" INTO project_user_id
    FROM "projects"
   WHERE "id" = NEW."project_id";

  SELECT "user_id" INTO tag_user_id
    FROM "tags"
   WHERE "id" = NEW."tag_id";

  IF project_user_id IS NULL OR tag_user_id IS NULL THEN
    RAISE EXCEPTION
      USING ERRCODE = '23503',
            MESSAGE = 'Referenced project or tag does not exist';
  END IF;

  IF project_user_id <> tag_user_id THEN
    RAISE EXCEPTION
      USING ERRCODE = '23514',
            MESSAGE = 'Project and tag must belong to the same user';
  END IF;

  RETURN NEW;
END;
$$;

CREATE TRIGGER "project_tags_validate_owner_trigger"
BEFORE INSERT OR UPDATE OF "project_id", "tag_id"
ON "project_tags"
FOR EACH ROW
EXECUTE FUNCTION validate_project_tag_owner();

CREATE OR REPLACE FUNCTION validate_task_tag_owner()
RETURNS trigger
LANGUAGE plpgsql
AS $$
DECLARE
  task_user_id uuid;
  tag_user_id uuid;
BEGIN
  SELECT p."user_id"
    INTO task_user_id
    FROM "tasks" t
    JOIN "projects" p ON p."id" = t."project_id"
   WHERE t."id" = NEW."task_id";

  SELECT "user_id"
    INTO tag_user_id
    FROM "tags"
   WHERE "id" = NEW."tag_id";

  IF task_user_id IS NULL OR tag_user_id IS NULL THEN
    RAISE EXCEPTION
      USING ERRCODE = '23503',
            MESSAGE = 'Referenced task or tag does not exist';
  END IF;

  IF task_user_id <> tag_user_id THEN
    RAISE EXCEPTION
      USING ERRCODE = '23514',
            MESSAGE = 'Task and tag must belong to the same user';
  END IF;

  RETURN NEW;
END;
$$;

CREATE TRIGGER "task_tags_validate_owner_trigger"
BEFORE INSERT OR UPDATE OF "task_id", "tag_id"
ON "task_tags"
FOR EACH ROW
EXECUTE FUNCTION validate_task_tag_owner();
