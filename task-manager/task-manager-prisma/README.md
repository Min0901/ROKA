# Task Manager Prisma schema

This bundle targets:

- Prisma ORM 7.x
- PostgreSQL
- Node.js with ESM
- The 12 finalized domain tables:
  `users`, `categories`, `projects`, `milestones`, `tasks`, `tags`,
  `time_records`, `notes`, `project_files`, `notification_settings`,
  `project_tags`, `task_tags`

## Files

- `prisma/schema.prisma`: Complete Prisma data model, relations, referential
  actions, enums, unique constraints, and indexes.
- `prisma/sql/domain_constraints.sql`: PostgreSQL CHECK constraints and
  cross-table validation triggers that cannot be fully represented in Prisma
  Schema Language.
- `prisma.config.ts`: Prisma ORM 7 CLI configuration.
- `.env.example`: Example PostgreSQL connection string.

## Apply

```bash
npm install prisma @prisma/client @prisma/adapter-pg pg dotenv
npm install -D tsx typescript

cp .env.example .env

npx prisma format
npx prisma validate
npx prisma migrate dev --name init --create-only
```

Append `prisma/sql/domain_constraints.sql` to the generated initial
`migration.sql`, then apply:

```bash
npx prisma migrate dev
npx prisma generate
```

## Important application rules

The database schema enforces ownership and hierarchy consistency where
practical, but the service layer must still enforce:

- valid Project, Milestone, and Task status transitions;
- rejection of explicit Milestone deletion when tasks are attached;
- rejection of explicit parent Task deletion when subtasks exist;
- complete-array validation for reorder operations;
- archived Project edit restrictions;
- project/milestone progress calculation;
- file MIME signature checks and storage compensation;
- password hashing and refresh-token persistence strategy.

Progress and actual task minutes are intentionally derived values and are not
stored in the database.
