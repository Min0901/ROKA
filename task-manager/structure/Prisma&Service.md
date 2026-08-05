# Task Manager Prisma 매핑 및 Service 계층 설계서 v1.0

## 1. 설계 목적

이 문서는 확정된 REST API와 DTO 설계를 실제 백엔드 애플리케이션 구조로 연결한다.

주요 설계 대상은 다음과 같다.

1. Prisma 모델과 API DTO 간 매핑
2. Controller, Service, Repository의 책임 분리
3. 사용자 소유권 검증
4. Project–Milestone–Task 관계 검증
5. Task 최대 깊이 제한
6. 상태 전이 검증
7. 프로젝트와 Milestone 진행률 계산
8. 순서 변경 및 이동 트랜잭션
9. Tag 다대다 관계 처리
10. TimeRecord 타이머 처리
11. Note와 ProjectFile 소유권 처리
12. 공통 오류 처리

---

# 2. 백엔드 계층 구조

```text
HTTP Request
    ↓
Middleware
    ├─ 인증
    ├─ Path Parameter 검증
    ├─ Query 검증
    └─ Body 검증
    ↓
Controller
    ↓
Service
    ├─ 비즈니스 규칙
    ├─ 소유권 검증
    ├─ 상태 전이 검증
    └─ 트랜잭션 처리
    ↓
Repository
    ↓
Prisma Client
    ↓
Database
```

각 계층의 책임을 명확하게 분리한다.

## 2.1 Controller

Controller는 다음 작업만 담당한다.

```text
요청 데이터 수신
인증된 userId 추출
Service 호출
HTTP 상태 코드 결정
응답 DTO 반환
```

Controller에서 다음 작업은 하지 않는다.

```text
Prisma 직접 호출
소유권 검사
상태 전이 판단
진행률 계산
트랜잭션 처리
```

## 2.2 Service

Service는 애플리케이션의 핵심 비즈니스 규칙을 담당한다.

```text
리소스 존재 여부 검사
사용자 소유권 검사
Project와 하위 리소스 관계 검사
상태 전이 검사
Task 깊이 검사
순서 변경
이동
진행률 계산
트랜잭션 실행
Repository 결과를 응답 DTO로 변환
```

## 2.3 Repository

Repository는 데이터베이스 접근만 담당한다.

```text
findUnique
findMany
create
update
delete
count
aggregate
transaction 내부 쿼리
```

Repository는 HTTP 상태 코드나 사용자 메시지를 결정하지 않는다.

---

# 3. 권장 디렉터리 구조

```text
src/
├─ app.ts
├─ server.ts
│
├─ config/
│  └─ env.ts
│
├─ database/
│  ├─ prisma.ts
│  └─ transaction.types.ts
│
├─ common/
│  ├─ errors/
│  │  ├─ app-error.ts
│  │  ├─ error-codes.ts
│  │  └─ error-handler.ts
│  ├─ middleware/
│  │  ├─ authenticate.ts
│  │  ├─ validate-request.ts
│  │  └─ not-found.ts
│  ├─ dto/
│  ├─ enums/
│  ├─ mapper/
│  └─ utils/
│
├─ auth/
│  ├─ auth.controller.ts
│  ├─ auth.service.ts
│  ├─ auth.repository.ts
│  ├─ auth.routes.ts
│  └─ dto/
│
├─ users/
├─ categories/
├─ projects/
├─ milestones/
├─ tasks/
├─ tags/
├─ time-records/
├─ notes/
├─ project-files/
├─ notification-settings/
└─ dashboard/
```

각 도메인 모듈은 다음 구성을 따른다.

```text
projects/
├─ project.controller.ts
├─ project.service.ts
├─ project.repository.ts
├─ project.mapper.ts
├─ project.routes.ts
├─ project.types.ts
└─ dto/
```

---

# 4. Prisma Client 설정

```ts
import { PrismaClient } from "@prisma/client";

export const prisma = new PrismaClient({
  log:
    process.env.NODE_ENV === "development"
      ? ["query", "warn", "error"]
      : ["warn", "error"],
});
```

애플리케이션 종료 시 연결을 정리한다.

```ts
async function shutdown(): Promise<void> {
  await prisma.$disconnect();
  process.exit(0);
}

process.on("SIGINT", shutdown);
process.on("SIGTERM", shutdown);
```

---

# 5. Prisma 모델과 API 필드 매핑

Prisma에서는 일반적으로 데이터베이스 컬럼을 `snake_case`, TypeScript 필드를 `camelCase`로 유지한다.

예시:

```prisma
model Project {
  id          String   @id @default(uuid()) @db.Uuid
  userId      String   @map("user_id") @db.Uuid
  categoryId  String   @map("category_id") @db.Uuid
  title       String   @db.VarChar(100)
  description String?  @db.Text
  status      ProjectStatus @default(PLANNED)
  priority    Priority @default(MEDIUM)
  startDate   DateTime? @map("start_date") @db.Date
  dueDate     DateTime? @map("due_date") @db.Date
  isArchived  Boolean  @default(false) @map("is_archived")
  createdAt   DateTime @default(now()) @map("created_at")
  updatedAt   DateTime @updatedAt @map("updated_at")

  @@map("projects")
}
```

API 응답에서는 다음처럼 반환한다.

```json
{
  "startDate": "2026-07-20",
  "dueDate": "2026-08-31",
  "isArchived": false,
  "createdAt": "2026-07-18T16:30:00.000Z"
}
```

`Date` 컬럼은 `YYYY-MM-DD` 문자열로 변환한다.

---

# 6. 날짜 매핑 유틸리티

Prisma는 `DATE` 타입도 JavaScript `Date` 객체로 반환한다. API에서는 날짜만 반환하도록 변환한다.

```ts
export function toDateOnly(
  value: Date | null,
): string | null {
  if (!value) {
    return null;
  }

  return value.toISOString().slice(0, 10);
}
```

입력 날짜 변환:

```ts
export function fromDateOnly(
  value: string | null | undefined,
): Date | null | undefined {
  if (value === undefined) {
    return undefined;
  }

  if (value === null) {
    return null;
  }

  return new Date(`${value}T00:00:00.000Z`);
}
```

날짜와 시간은 그대로 ISO 문자열로 변환한다.

```ts
export function toIsoDateTime(
  value: Date | null,
): string | null {
  return value?.toISOString() ?? null;
}
```

---

# 7. 공통 오류 클래스

```ts
export class AppError extends Error {
  constructor(
    public readonly statusCode: number,
    public readonly code: string,
    message: string,
    public readonly details: unknown = null,
  ) {
    super(message);
    this.name = "AppError";
  }
}
```

오류 생성 예시:

```ts
throw new AppError(
  404,
  "PROJECT_NOT_FOUND",
  "프로젝트를 찾을 수 없습니다.",
);
```

## 7.1 공통 오류 코드

```ts
export const ERROR_CODES = {
  VALIDATION_ERROR: "VALIDATION_ERROR",
  AUTHENTICATION_REQUIRED:
    "AUTHENTICATION_REQUIRED",
  INVALID_ACCESS_TOKEN:
    "INVALID_ACCESS_TOKEN",

  CATEGORY_NOT_FOUND:
    "CATEGORY_NOT_FOUND",
  CATEGORY_IN_USE:
    "CATEGORY_IN_USE",

  PROJECT_NOT_FOUND:
    "PROJECT_NOT_FOUND",
  PROJECT_ARCHIVED:
    "PROJECT_ARCHIVED",
  INVALID_PROJECT_STATUS_TRANSITION:
    "INVALID_PROJECT_STATUS_TRANSITION",

  MILESTONE_NOT_FOUND:
    "MILESTONE_NOT_FOUND",
  MILESTONE_PROJECT_MISMATCH:
    "MILESTONE_PROJECT_MISMATCH",
  MILESTONE_HAS_TASKS:
    "MILESTONE_HAS_TASKS",

  TASK_NOT_FOUND:
    "TASK_NOT_FOUND",
  TASK_PROJECT_MISMATCH:
    "TASK_PROJECT_MISMATCH",
  TASK_DEPTH_LIMIT_EXCEEDED:
    "TASK_DEPTH_LIMIT_EXCEEDED",
  TASK_HAS_SUBTASKS:
    "TASK_HAS_SUBTASKS",
  INVALID_TASK_ORDER:
    "INVALID_TASK_ORDER",
} as const;
```

---

# 8. 전역 오류 처리

```ts
import type {
  NextFunction,
  Request,
  Response,
} from "express";
import { Prisma } from "@prisma/client";

export function errorHandler(
  error: unknown,
  _req: Request,
  res: Response,
  _next: NextFunction,
): void {
  if (error instanceof AppError) {
    res.status(error.statusCode).json({
      error: {
        code: error.code,
        message: error.message,
        details: error.details,
      },
    });

    return;
  }

  if (
    error instanceof
    Prisma.PrismaClientKnownRequestError
  ) {
    if (error.code === "P2002") {
      res.status(409).json({
        error: {
          code: "DUPLICATE_RESOURCE",
          message:
            "이미 존재하는 데이터입니다.",
          details: error.meta ?? null,
        },
      });

      return;
    }

    if (error.code === "P2025") {
      res.status(404).json({
        error: {
          code: "RESOURCE_NOT_FOUND",
          message:
            "요청한 데이터를 찾을 수 없습니다.",
          details: null,
        },
      });

      return;
    }
  }

  console.error(error);

  res.status(500).json({
    error: {
      code: "INTERNAL_SERVER_ERROR",
      message:
        "서버 내부 오류가 발생했습니다.",
      details: null,
    },
  });
}
```

---

# 9. 트랜잭션 타입

Repository 메서드가 일반 Prisma Client와 트랜잭션 Client를 모두 사용할 수 있도록 타입을 정의한다.

```ts
import type {
  Prisma,
  PrismaClient,
} from "@prisma/client";

export type DatabaseClient =
  | PrismaClient
  | Prisma.TransactionClient;
```

Repository 생성자에서 Client를 주입한다.

```ts
export class TaskRepository {
  constructor(
    private readonly db: DatabaseClient,
  ) {}
}
```

트랜잭션 안에서는 새로운 Repository를 생성한다.

```ts
await prisma.$transaction(async (tx) => {
  const taskRepository =
    new TaskRepository(tx);

  // 트랜잭션 작업
});
```

---

# 10. Project Repository 설계

## 10.1 기본 조회

```ts
import type {
  Prisma,
  Project,
} from "@prisma/client";

export class ProjectRepository {
  constructor(
    private readonly db: DatabaseClient,
  ) {}

  findOwnedById(
    userId: string,
    projectId: string,
  ) {
    return this.db.project.findFirst({
      where: {
        id: projectId,
        userId,
      },
    });
  }

  findOwnedDetail(
    userId: string,
    projectId: string,
  ) {
    return this.db.project.findFirst({
      where: {
        id: projectId,
        userId,
      },
      include: {
        category: true,
        projectTags: {
          include: {
            tag: true,
          },
        },
      },
    });
  }
}
```

다른 사용자의 프로젝트와 존재하지 않는 프로젝트를 구분하지 않기 위해 `id + userId`로 한 번에 조회한다.

## 10.2 프로젝트 생성

```ts
create(
  userId: string,
  data: Prisma.ProjectUncheckedCreateInput,
) {
  return this.db.project.create({
    data: {
      ...data,
      userId,
    },
    include: {
      category: true,
    },
  });
}
```

## 10.3 프로젝트 수정

```ts
update(
  projectId: string,
  data: Prisma.ProjectUpdateInput,
) {
  return this.db.project.update({
    where: {
      id: projectId,
    },
    data,
    include: {
      category: true,
      projectTags: {
        include: {
          tag: true,
        },
      },
    },
  });
}
```

---

# 11. Project Mapper

DB 결과를 그대로 응답하면 중간 테이블이나 내부 필드가 노출될 수 있다. Mapper에서 응답 구조를 확정한다.

```ts
export function mapProjectCategory(
  category: {
    id: string;
    name: string;
    slug: string;
  },
) {
  return {
    id: category.id,
    name: category.name,
    slug: category.slug,
  };
}
```

```ts
export function mapTagSummary(
  tag: {
    id: string;
    name: string;
    color: string | null;
  },
) {
  return {
    id: tag.id,
    name: tag.name,
    color: tag.color,
  };
}
```

```ts
export function mapProjectResponse(
  project: ProjectDetailEntity,
  statistics: ProjectStatistics,
): ProjectResponse {
  return {
    id: project.id,
    category:
      mapProjectCategory(project.category),
    title: project.title,
    description: project.description,
    status: project.status,
    priority: project.priority,
    startDate:
      toDateOnly(project.startDate),
    dueDate:
      toDateOnly(project.dueDate),
    isArchived: project.isArchived,
    progress: statistics.progress,
    taskSummary: statistics.taskSummary,
    tags: project.projectTags.map(
      ({ tag }) => mapTagSummary(tag),
    ),
    createdAt:
      project.createdAt.toISOString(),
    updatedAt:
      project.updatedAt.toISOString(),
  };
}
```

---

# 12. Project Service 생성 로직

```ts
export class ProjectService {
  constructor(
    private readonly projectRepository:
      ProjectRepository,
    private readonly categoryRepository:
      CategoryRepository,
    private readonly taskRepository:
      TaskRepository,
  ) {}

  async create(
    userId: string,
    dto: CreateProjectDto,
  ): Promise<ProjectResponse> {
    const category =
      await this.categoryRepository
        .findAvailableById(
          userId,
          dto.categoryId,
        );

    if (!category) {
      throw new AppError(
        404,
        "CATEGORY_NOT_FOUND",
        "카테고리를 찾을 수 없습니다.",
      );
    }

    const project =
      await this.projectRepository.create(
        userId,
        {
          categoryId: dto.categoryId,
          title: dto.title,
          description:
            dto.description ?? null,
          status: dto.status,
          priority: dto.priority,
          startDate:
            fromDateOnly(dto.startDate),
          dueDate:
            fromDateOnly(dto.dueDate),
          isArchived: false,
        },
      );

    return {
      id: project.id,
      category:
        mapProjectCategory(
          project.category,
        ),
      title: project.title,
      description: project.description,
      status: project.status,
      priority: project.priority,
      startDate:
        toDateOnly(project.startDate),
      dueDate:
        toDateOnly(project.dueDate),
      isArchived: project.isArchived,
      progress: 0,
      taskSummary: {
        total: 0,
        todo: 0,
        inProgress: 0,
        blocked: 0,
        completed: 0,
        cancelled: 0,
      },
      tags: [],
      createdAt:
        project.createdAt.toISOString(),
      updatedAt:
        project.updatedAt.toISOString(),
    };
  }
}
```

---

# 13. Project 소유권 검증 공통 메서드

여러 Service에서 프로젝트 소유권을 반복해서 확인하므로 공통 메서드로 분리한다.

```ts
export async function requireOwnedProject(
  repository: ProjectRepository,
  userId: string,
  projectId: string,
) {
  const project =
    await repository.findOwnedById(
      userId,
      projectId,
    );

  if (!project) {
    throw new AppError(
      404,
      "PROJECT_NOT_FOUND",
      "프로젝트를 찾을 수 없습니다.",
    );
  }

  return project;
}
```

수정 가능한 프로젝트인지 검사한다.

```ts
export function assertProjectEditable(
  project: {
    isArchived: boolean;
  },
): void {
  if (project.isArchived) {
    throw new AppError(
      409,
      "PROJECT_ARCHIVED",
      "보관된 프로젝트는 수정할 수 없습니다.",
    );
  }
}
```

사용 예시:

```ts
const project =
  await requireOwnedProject(
    projectRepository,
    userId,
    projectId,
  );

assertProjectEditable(project);
```

---

# 14. Project 상태 전이

## 14.1 허용 상태 전이

```ts
import type {
  ProjectStatus,
} from "@prisma/client";

const PROJECT_STATUS_TRANSITIONS:
  Record<ProjectStatus, ProjectStatus[]> = {
    PLANNED: [
      "IN_PROGRESS",
      "PAUSED",
      "CANCELLED",
    ],

    IN_PROGRESS: [
      "PAUSED",
      "COMPLETED",
      "CANCELLED",
    ],

    PAUSED: [
      "IN_PROGRESS",
      "COMPLETED",
      "CANCELLED",
    ],

    COMPLETED: [
      "IN_PROGRESS",
    ],

    CANCELLED: [
      "PLANNED",
    ],
  };
```

## 14.2 검증 함수

```ts
export function assertProjectStatusTransition(
  currentStatus: ProjectStatus,
  nextStatus: ProjectStatus,
): void {
  if (currentStatus === nextStatus) {
    return;
  }

  const allowed =
    PROJECT_STATUS_TRANSITIONS[
      currentStatus
    ];

  if (!allowed.includes(nextStatus)) {
    throw new AppError(
      422,
      "INVALID_PROJECT_STATUS_TRANSITION",
      `${currentStatus}에서 ${nextStatus}(으)로 변경할 수 없습니다.`,
      {
        currentStatus,
        requestedStatus: nextStatus,
        allowedStatuses: allowed,
      },
    );
  }
}
```

---

# 15. Project 상태 변경 Service

```ts
async updateStatus(
  userId: string,
  projectId: string,
  dto: UpdateProjectStatusDto,
): Promise<ProjectResponse> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  assertProjectEditable(project);

  assertProjectStatusTransition(
    project.status,
    dto.status,
  );

  await this.projectRepository.update(
    projectId,
    {
      status: dto.status,
    },
  );

  return this.findDetail(
    userId,
    projectId,
  );
}
```

프로젝트를 `COMPLETED`로 변경할 때 미완료 Task 존재를 허용할지 정책이 필요하다.

초기 버전에서는 허용하되, 경고 정보를 응답하지 않고 상태만 변경한다. 추후 엄격한 정책이 필요하면 다음 검증을 추가한다.

```text
미완료 Task가 있으면 COMPLETED 변경 거부
```

---

# 16. Project 보관 처리

```ts
async archive(
  userId: string,
  projectId: string,
): Promise<void> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  if (project.isArchived) {
    return;
  }

  await this.projectRepository.update(
    projectId,
    {
      isArchived: true,
    },
  );
}
```

보관 해제:

```ts
async unarchive(
  userId: string,
  projectId: string,
): Promise<void> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  if (!project.isArchived) {
    return;
  }

  await this.projectRepository.update(
    projectId,
    {
      isArchived: false,
    },
  );
}
```

보관과 보관 해제는 멱등성을 보장한다.

```text
이미 보관된 프로젝트 보관 → 성공
이미 활성화된 프로젝트 보관 해제 → 성공
```

---

# 17. Task Repository 설계

```ts
export class TaskRepository {
  constructor(
    private readonly db: DatabaseClient,
  ) {}

  findById(
    taskId: string,
  ) {
    return this.db.task.findUnique({
      where: {
        id: taskId,
      },
    });
  }

  findInProject(
    projectId: string,
    taskId: string,
  ) {
    return this.db.task.findFirst({
      where: {
        id: taskId,
        projectId,
      },
    });
  }

  findDetailInProject(
    projectId: string,
    taskId: string,
  ) {
    return this.db.task.findFirst({
      where: {
        id: taskId,
        projectId,
      },
      include: {
        milestone: true,
        subtasks: {
          orderBy: {
            position: "asc",
          },
        },
        taskTags: {
          include: {
            tag: true,
          },
        },
      },
    });
  }

  countSubtasks(
    taskId: string,
  ) {
    return this.db.task.count({
      where: {
        parentTaskId: taskId,
      },
    });
  }
}
```

---

# 18. Task 생성 검증

Task 생성 시 다음 순서로 검증한다.

```text
1. Project 소유권 확인
2. Project 보관 여부 확인
3. milestoneId 존재 시 Milestone 소속 확인
4. parentTaskId 존재 시 부모 Task 소속 확인
5. 부모 Task가 최상위 Task인지 확인
6. 날짜 범위 검증
7. 마지막 position 계산
8. Task 생성
```

## 18.1 Milestone 관계 검증

```ts
async function requireMilestoneInProject(
  repository: MilestoneRepository,
  projectId: string,
  milestoneId: string,
) {
  const milestone =
    await repository.findInProject(
      projectId,
      milestoneId,
    );

  if (!milestone) {
    throw new AppError(
      404,
      "MILESTONE_NOT_FOUND",
      "마일스톤을 찾을 수 없습니다.",
    );
  }

  return milestone;
}
```

## 18.2 부모 Task 검증

```ts
async function requireValidParentTask(
  repository: TaskRepository,
  projectId: string,
  parentTaskId: string,
) {
  const parentTask =
    await repository.findInProject(
      projectId,
      parentTaskId,
    );

  if (!parentTask) {
    throw new AppError(
      404,
      "TASK_NOT_FOUND",
      "상위 작업을 찾을 수 없습니다.",
    );
  }

  if (parentTask.parentTaskId !== null) {
    throw new AppError(
      422,
      "TASK_DEPTH_LIMIT_EXCEEDED",
      "하위 작업은 한 단계까지만 생성할 수 있습니다.",
    );
  }

  return parentTask;
}
```

---

# 19. Task 생성 Service

```ts
async create(
  userId: string,
  projectId: string,
  dto: CreateTaskDto,
): Promise<TaskResponse> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  assertProjectEditable(project);

  if (dto.milestoneId) {
    await requireMilestoneInProject(
      this.milestoneRepository,
      projectId,
      dto.milestoneId,
    );
  }

  if (dto.parentTaskId) {
    await requireValidParentTask(
      this.taskRepository,
      projectId,
      dto.parentTaskId,
    );
  }

  const position =
    dto.position ??
    await this.taskRepository
      .findNextPosition(
        projectId,
        dto.parentTaskId ?? null,
      );

  const task =
    await this.taskRepository.create({
      projectId,
      milestoneId:
        dto.milestoneId ?? null,
      parentTaskId:
        dto.parentTaskId ?? null,
      title: dto.title,
      description:
        dto.description ?? null,
      status: dto.status,
      priority: dto.priority,
      startDate:
        fromDateOnly(dto.startDate),
      dueDate:
        fromDateOnly(dto.dueDate),
      estimatedMinutes:
        dto.estimatedMinutes ?? null,
      position,
      completedAt:
        dto.status === "COMPLETED"
          ? new Date()
          : null,
    });

  return mapTaskResponse(
    task,
    {
      actualMinutes: 0,
      subtaskSummary: {
        total: 0,
        completed: 0,
      },
      tags: [],
    },
  );
}
```

---

# 20. Task 상태 전이

## 20.1 허용 상태 전이

```ts
const TASK_STATUS_TRANSITIONS:
  Record<TaskStatus, TaskStatus[]> = {
    TODO: [
      "IN_PROGRESS",
      "BLOCKED",
      "COMPLETED",
      "CANCELLED",
    ],

    IN_PROGRESS: [
      "TODO",
      "BLOCKED",
      "COMPLETED",
      "CANCELLED",
    ],

    BLOCKED: [
      "TODO",
      "IN_PROGRESS",
      "COMPLETED",
      "CANCELLED",
    ],

    COMPLETED: [
      "TODO",
      "IN_PROGRESS",
    ],

    CANCELLED: [
      "TODO",
    ],
  };
```

## 20.2 완료 시각 처리

```ts
function getCompletedAtForStatus(
  nextStatus: TaskStatus,
): Date | null {
  return nextStatus === "COMPLETED"
    ? new Date()
    : null;
}
```

이미 완료된 Task에 동일한 `COMPLETED` 요청이 들어오면 기존 `completedAt`을 유지해야 한다.

```ts
function resolveCompletedAt(
  currentStatus: TaskStatus,
  currentCompletedAt: Date | null,
  nextStatus: TaskStatus,
): Date | null {
  if (
    currentStatus === "COMPLETED" &&
    nextStatus === "COMPLETED"
  ) {
    return currentCompletedAt;
  }

  return nextStatus === "COMPLETED"
    ? new Date()
    : null;
}
```

---

# 21. 상위 Task 완료 정책

상위 Task에 하위 Task가 있을 때 완료 처리 정책을 확정해야 한다.

초기 버전에서는 다음 규칙을 적용한다.

```text
하위 Task가 모두 COMPLETED 또는 CANCELLED인 경우에만
상위 Task를 COMPLETED로 변경할 수 있다.
```

검증:

```ts
async function assertSubtasksClosable(
  repository: TaskRepository,
  taskId: string,
): Promise<void> {
  const incompleteCount =
    await repository.count({
      parentTaskId: taskId,
      status: {
        notIn: [
          "COMPLETED",
          "CANCELLED",
        ],
      },
    });

  if (incompleteCount > 0) {
    throw new AppError(
      409,
      "TASK_HAS_INCOMPLETE_SUBTASKS",
      "미완료 하위 작업이 있어 완료할 수 없습니다.",
      {
        incompleteSubtaskCount:
          incompleteCount,
      },
    );
  }
}
```

---

# 22. Task 완료 토글 Service

```ts
async updateCompletion(
  userId: string,
  projectId: string,
  taskId: string,
  dto: UpdateTaskCompletionDto,
): Promise<TaskResponse> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  assertProjectEditable(project);

  const task =
    await this.taskRepository
      .findInProject(
        projectId,
        taskId,
      );

  if (!task) {
    throw new AppError(
      404,
      "TASK_NOT_FOUND",
      "작업을 찾을 수 없습니다.",
    );
  }

  const nextStatus =
    dto.isCompleted
      ? "COMPLETED"
      : "TODO";

  if (nextStatus === "COMPLETED") {
    await assertSubtasksClosable(
      this.taskRepository,
      taskId,
    );
  }

  await this.taskRepository.update(
    taskId,
    {
      status: nextStatus,
      completedAt:
        dto.isCompleted
          ? new Date()
          : null,
    },
  );

  return this.findDetail(
    userId,
    projectId,
    taskId,
  );
}
```

---

# 23. Task 순서 변경 트랜잭션

순서 변경은 여러 행을 동시에 수정하므로 반드시 트랜잭션으로 처리한다.

## 23.1 검증 규칙

```text
모든 Task가 같은 Project에 속해야 한다.
모든 Task의 parentTaskId가 요청과 같아야 한다.
Task ID 중복이 없어야 한다.
정렬 대상 그룹의 모든 Task가 포함되어야 한다.
position은 0부터 연속된 정수로 설정한다.
```

## 23.2 Service

```ts
async reorder(
  userId: string,
  projectId: string,
  dto: ReorderTaskDto,
): Promise<void> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  assertProjectEditable(project);

  await prisma.$transaction(
    async (tx) => {
      const repository =
        new TaskRepository(tx);

      const tasks =
        await repository.findByIds(
          dto.taskIds,
        );

      if (
        tasks.length !==
        dto.taskIds.length
      ) {
        throw new AppError(
          400,
          "INVALID_TASK_ORDER",
          "존재하지 않는 작업이 포함되어 있습니다.",
        );
      }

      const invalidTask =
        tasks.find(
          (task) =>
            task.projectId !==
              projectId ||
            task.parentTaskId !==
              dto.parentTaskId,
        );

      if (invalidTask) {
        throw new AppError(
          400,
          "INVALID_TASK_ORDER",
          "같은 작업 그룹만 정렬할 수 있습니다.",
        );
      }

      const groupCount =
        await repository.countGroup(
          projectId,
          dto.parentTaskId,
        );

      if (
        groupCount !==
        dto.taskIds.length
      ) {
        throw new AppError(
          400,
          "INVALID_TASK_ORDER",
          "정렬 대상 작업 전체를 전달해야 합니다.",
        );
      }

      await Promise.all(
        dto.taskIds.map(
          (taskId, index) =>
            repository.update(
              taskId,
              {
                position: index,
              },
            ),
        ),
      );
    },
  );
}
```

프로젝트 규모가 커지면 `Promise.all` 다중 UPDATE 대신 raw SQL의 `CASE WHEN` 일괄 갱신을 고려할 수 있다.

---

# 24. Task 이동 트랜잭션

Task 이동은 다음 값을 동시에 변경할 수 있다.

```text
milestoneId
parentTaskId
position
```

## 24.1 추가 검증

```text
자기 자신을 부모로 지정할 수 없다.
자신의 하위 Task를 부모로 지정할 수 없다.
하위 Task가 있는 Task를 다른 Task의 하위로 이동할 수 없다.
부모 Task는 최상위 Task여야 한다.
대상 Milestone은 같은 Project에 속해야 한다.
기존 그룹과 새 그룹의 position을 재정렬해야 한다.
```

## 24.2 순환 참조 방지

Task 깊이가 최대 1단계이므로 다음 검사로 충분하다.

```ts
if (dto.parentTaskId === taskId) {
  throw new AppError(
    422,
    "INVALID_TASK_PARENT",
    "작업 자신을 상위 작업으로 지정할 수 없습니다.",
  );
}
```

하위 Task가 있는 Task를 다른 Task 아래로 이동하지 않는다.

```ts
const subtaskCount =
  await repository.countSubtasks(
    taskId,
  );

if (
  dto.parentTaskId &&
  subtaskCount > 0
) {
  throw new AppError(
    422,
    "TASK_DEPTH_LIMIT_EXCEEDED",
    "하위 작업을 가진 작업은 다른 작업의 하위로 이동할 수 없습니다.",
  );
}
```

---

# 25. Task 삭제

초기 정책에서는 하위 Task가 있으면 삭제를 거부한다.

```ts
async remove(
  userId: string,
  projectId: string,
  taskId: string,
): Promise<void> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  assertProjectEditable(project);

  const task =
    await this.taskRepository
      .findInProject(
        projectId,
        taskId,
      );

  if (!task) {
    throw new AppError(
      404,
      "TASK_NOT_FOUND",
      "작업을 찾을 수 없습니다.",
    );
  }

  const subtaskCount =
    await this.taskRepository
      .countSubtasks(taskId);

  if (subtaskCount > 0) {
    throw new AppError(
      409,
      "TASK_HAS_SUBTASKS",
      "하위 작업이 있는 작업은 삭제할 수 없습니다.",
      {
        subtaskCount,
      },
    );
  }

  await prisma.$transaction(
    async (tx) => {
      const repository =
        new TaskRepository(tx);

      await repository.delete(taskId);

      await repository.compactPositions(
        projectId,
        task.parentTaskId,
        task.position,
      );
    },
  );
}
```

삭제 시 관련 데이터 처리:

```text
task_tags → Cascade 삭제
task Note → Cascade 또는 taskId null 정책
time_records → 유지 또는 Cascade 정책 확정 필요
```

권장 정책:

```text
task_tags: Cascade
task 전용 notes: Cascade
time_records: Task 삭제 후에도 기록 보존을 위해 taskId = null
```

---

# 26. 프로젝트 진행률 계산

진행률 계산 규칙은 다음과 같다.

```text
CANCELLED Task는 제외한다.
하위 Task가 없는 Task는 직접 계산한다.
하위 Task가 있는 부모 Task는 계산에서 제외한다.
부모의 하위 Task만 계산한다.
```

## 26.1 계산 대상 조건

Prisma 조건:

```ts
const progressEligibleWhere = {
  projectId,
  status: {
    not: "CANCELLED",
  },
  OR: [
    {
      parentTaskId: {
        not: null,
      },
    },
    {
      subtasks: {
        none: {},
      },
    },
  ],
} satisfies Prisma.TaskWhereInput;
```

해석:

```text
하위 Task이거나
하위 Task가 없는 최상위 Task
```

## 26.2 통계 Repository

```ts
async getProjectTaskStatistics(
  projectId: string,
) {
  const [
    total,
    completed,
    statusGroups,
  ] = await Promise.all([
    this.db.task.count({
      where: progressEligibleWhere,
    }),

    this.db.task.count({
      where: {
        ...progressEligibleWhere,
        status: "COMPLETED",
      },
    }),

    this.db.task.groupBy({
      by: ["status"],
      where: {
        projectId,
      },
      _count: {
        _all: true,
      },
    }),
  ]);

  return {
    total,
    completed,
    statusGroups,
  };
}
```

## 26.3 계산 함수

```ts
export function calculateProgress(
  completed: number,
  total: number,
): number {
  if (total === 0) {
    return 0;
  }

  return Math.round(
    (completed / total) * 100,
  );
}
```

## 26.4 Task Summary 생성

```ts
export function buildTaskSummary(
  groups: Array<{
    status: TaskStatus;
    _count: {
      _all: number;
    };
  }>,
): ProjectTaskSummary {
  const summary = {
    total: 0,
    todo: 0,
    inProgress: 0,
    blocked: 0,
    completed: 0,
    cancelled: 0,
  };

  for (const group of groups) {
    const count =
      group._count._all;

    summary.total += count;

    switch (group.status) {
      case "TODO":
        summary.todo = count;
        break;

      case "IN_PROGRESS":
        summary.inProgress = count;
        break;

      case "BLOCKED":
        summary.blocked = count;
        break;

      case "COMPLETED":
        summary.completed = count;
        break;

      case "CANCELLED":
        summary.cancelled = count;
        break;
    }
  }

  return summary;
}
```

`taskSummary.total`은 전체 Task 수이고, `progress` 분모는 진행률 계산 대상 Task 수라는 점을 구분해야 한다.

---

# 27. Milestone 진행률 계산

Milestone도 Project와 같은 규칙을 적용하되 해당 `milestoneId`를 가진 Task만 계산한다.

```ts
async getMilestoneProgress(
  milestoneId: string,
): Promise<number> {
  const where:
    Prisma.TaskWhereInput = {
      milestoneId,
      status: {
        not: "CANCELLED",
      },
      OR: [
        {
          parentTaskId: {
            not: null,
          },
        },
        {
          subtasks: {
            none: {},
          },
        },
      ],
    };

  const [total, completed] =
    await Promise.all([
      prisma.task.count({
        where,
      }),

      prisma.task.count({
        where: {
          ...where,
          status: "COMPLETED",
        },
      }),
    ]);

  return calculateProgress(
    completed,
    total,
  );
}
```

---

# 28. Milestone 삭제 정책

Milestone에 Task가 연결되어 있으면 삭제를 거부한다.

```ts
async remove(
  userId: string,
  projectId: string,
  milestoneId: string,
): Promise<void> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  assertProjectEditable(project);

  const milestone =
    await this.milestoneRepository
      .findInProject(
        projectId,
        milestoneId,
      );

  if (!milestone) {
    throw new AppError(
      404,
      "MILESTONE_NOT_FOUND",
      "마일스톤을 찾을 수 없습니다.",
    );
  }

  const taskCount =
    await this.taskRepository.count({
      milestoneId,
    });

  if (taskCount > 0) {
    throw new AppError(
      409,
      "MILESTONE_HAS_TASKS",
      "연결된 작업이 있는 마일스톤은 삭제할 수 없습니다.",
      {
        taskCount,
      },
    );
  }

  await this.milestoneRepository
    .delete(milestoneId);
}
```

---

# 29. Tag 다대다 관계 처리

## 29.1 Project Tag 연결

```ts
async attachToProject(
  userId: string,
  projectId: string,
  dto: AttachTagsDto,
): Promise<TagSummary[]> {
  await requireOwnedProject(
    this.projectRepository,
    userId,
    projectId,
  );

  const tags =
    await this.tagRepository
      .findOwnedByIds(
        userId,
        dto.tagIds,
      );

  if (
    tags.length !==
    dto.tagIds.length
  ) {
    throw new AppError(
      404,
      "TAG_NOT_FOUND",
      "존재하지 않는 태그가 포함되어 있습니다.",
    );
  }

  await this.projectTagRepository
    .attachMany(
      projectId,
      dto.tagIds,
    );

  return tags.map(mapTagSummary);
}
```

Prisma의 `createMany`와 `skipDuplicates`를 사용한다.

```ts
attachMany(
  projectId: string,
  tagIds: string[],
) {
  return this.db.projectTag.createMany({
    data: tagIds.map((tagId) => ({
      projectId,
      tagId,
    })),
    skipDuplicates: true,
  });
}
```

## 29.2 연결 해제

```ts
delete({
  projectId,
  tagId,
}: {
  projectId: string;
  tagId: string;
}) {
  return this.db.projectTag.deleteMany({
    where: {
      projectId,
      tagId,
    },
  });
}
```

이미 연결되어 있지 않아도 성공 처리하여 멱등성을 보장한다.

---

# 30. Tag 이름 중복 처리

사용자별로 대소문자를 무시한 Tag 이름 중복을 방지해야 한다.

권장 방법은 정규화 컬럼을 추가하는 것이다.

```prisma
model Tag {
  id             String @id @default(uuid()) @db.Uuid
  userId         String @map("user_id") @db.Uuid
  name           String @db.VarChar(50)
  normalizedName String @map("normalized_name") @db.VarChar(50)
  color          String? @db.VarChar(7)

  @@unique([userId, normalizedName])
  @@map("tags")
}
```

정규화:

```ts
export function normalizeTagName(
  value: string,
): string {
  return value
    .trim()
    .toLocaleLowerCase("ko-KR");
}
```

생성:

```ts
{
  userId,
  name: dto.name.trim(),
  normalizedName:
    normalizeTagName(dto.name),
  color: dto.color ?? null,
}
```

---

# 31. TimeRecord Service

## 31.1 수동 시간 기록 생성

```ts
async create(
  userId: string,
  projectId: string,
  dto: CreateTimeRecordDto,
): Promise<TimeRecordResponse> {
  const project =
    await requireOwnedProject(
      this.projectRepository,
      userId,
      projectId,
    );

  assertProjectEditable(project);

  if (dto.taskId) {
    const task =
      await this.taskRepository
        .findInProject(
          projectId,
          dto.taskId,
        );

    if (!task) {
      throw new AppError(
        404,
        "TASK_NOT_FOUND",
        "작업을 찾을 수 없습니다.",
      );
    }
  }

  const startedAt =
    new Date(dto.startedAt);

  const endedAt =
    new Date(dto.endedAt);

  const durationMinutes =
    calculateDurationMinutes(
      startedAt,
      endedAt,
    );

  const record =
    await this.timeRecordRepository
      .create({
        projectId,
        taskId: dto.taskId ?? null,
        startedAt,
        endedAt,
        durationMinutes,
        description:
          dto.description ?? null,
      });

  return mapTimeRecordResponse(
    record,
  );
}
```

## 31.2 시간 계산

```ts
export function calculateDurationMinutes(
  startedAt: Date,
  endedAt: Date,
): number {
  const difference =
    endedAt.getTime() -
    startedAt.getTime();

  if (difference <= 0) {
    throw new AppError(
      422,
      "INVALID_TIME_RANGE",
      "종료 시각은 시작 시각보다 늦어야 합니다.",
    );
  }

  return Math.floor(
    difference / 60_000,
  );
}
```

---

# 32. 타이머 시작

사용자별 활성 타이머는 하나만 허용한다.

활성 타이머 조건:

```text
endedAt IS NULL
```

```ts
async start(
  userId: string,
  projectId: string,
  dto: StartTimeRecordDto,
): Promise<TimeRecordResponse> {
  await requireOwnedProject(
    this.projectRepository,
    userId,
    projectId,
  );

  const active =
    await this.timeRecordRepository
      .findActiveByUserId(userId);

  if (active) {
    throw new AppError(
      409,
      "ACTIVE_TIMER_ALREADY_EXISTS",
      "이미 실행 중인 타이머가 있습니다.",
      {
        timeRecordId: active.id,
        startedAt:
          active.startedAt.toISOString(),
      },
    );
  }

  if (dto.taskId) {
    const task =
      await this.taskRepository
        .findInProject(
          projectId,
          dto.taskId,
        );

    if (!task) {
      throw new AppError(
        404,
        "TASK_NOT_FOUND",
        "작업을 찾을 수 없습니다.",
      );
    }
  }

  const record =
    await this.timeRecordRepository
      .create({
        userId,
        projectId,
        taskId: dto.taskId ?? null,
        startedAt: new Date(),
        endedAt: null,
        durationMinutes: null,
        description:
          dto.description ?? null,
      });

  return mapTimeRecordResponse(
    record,
  );
}
```

DB에서도 사용자별 활성 타이머 하나만 존재하도록 보장하는 것이 안전하다. PostgreSQL을 사용한다면 부분 유니크 인덱스를 고려한다.

```sql
CREATE UNIQUE INDEX
unique_active_timer_per_user
ON time_records (user_id)
WHERE ended_at IS NULL;
```

---

# 33. 타이머 종료 트랜잭션

동일한 타이머에 종료 요청이 동시에 들어올 수 있으므로 트랜잭션 내에서 현재 상태를 확인한다.

```ts
async stop(
  userId: string,
  timeRecordId: string,
  dto: StopTimeRecordDto,
): Promise<TimeRecordResponse> {
  return prisma.$transaction(
    async (tx) => {
      const repository =
        new TimeRecordRepository(tx);

      const record =
        await repository
          .findOwnedById(
            userId,
            timeRecordId,
          );

      if (!record) {
        throw new AppError(
          404,
          "TIME_RECORD_NOT_FOUND",
          "시간 기록을 찾을 수 없습니다.",
        );
      }

      if (record.endedAt) {
        return mapTimeRecordResponse(
          record,
        );
      }

      const endedAt =
        dto.endedAt
          ? new Date(dto.endedAt)
          : new Date();

      const durationMinutes =
        calculateDurationMinutes(
          record.startedAt,
          endedAt,
        );

      const updated =
        await repository.update(
          timeRecordId,
          {
            endedAt,
            durationMinutes,
          },
        );

      return mapTimeRecordResponse(
        updated,
      );
    },
  );
}
```

이미 종료된 타이머에 대한 종료 요청은 기존 결과를 반환하는 멱등 방식으로 처리한다.

---

# 34. Note 소유권 처리

Note를 ID만으로 조회하면 다른 사용자의 Note 존재 여부가 노출될 수 있다.

Repository 조회 시 Project 소유권까지 함께 조건으로 검사한다.

```ts
findOwnedById(
  userId: string,
  noteId: string,
) {
  return this.db.note.findFirst({
    where: {
      id: noteId,
      project: {
        userId,
      },
    },
  });
}
```

Task Note 생성 시 확인 순서:

```text
Project 소유권 확인
Task가 Project에 속하는지 확인
Note 생성
```

요청 본문에서 `projectId`, `taskId`, `userId`를 받지 않는다.

---

# 35. ProjectFile 처리

파일 업로드는 다음 순서로 처리한다.

```text
1. Project 소유권 확인
2. Project 보관 여부 확인
3. 파일 크기 검증
4. MIME 타입 검증
5. 실제 파일 시그니처 검증
6. 스토리지 업로드
7. DB 레코드 생성
```

DB 생성이 실패하면 이미 업로드된 스토리지 객체를 삭제해야 한다.

```ts
let storageKey: string | null = null;

try {
  const uploaded =
    await storage.upload(file);

  storageKey = uploaded.key;

  return await repository.create({
    projectId,
    originalName: file.originalname,
    storageKey: uploaded.key,
    mimeType: uploaded.mimeType,
    sizeBytes: uploaded.size,
    description:
      dto.description ?? null,
  });
} catch (error) {
  if (storageKey) {
    await storage
      .delete(storageKey)
      .catch(console.error);
  }

  throw error;
}
```

파일 삭제는 반대로 처리한다.

```text
1. 소유권 확인
2. DB 레코드 삭제
3. 스토리지 파일 삭제
```

스토리지 삭제 실패를 고려하면 Outbox 또는 삭제 재시도 큐가 가장 안전하지만 초기 버전에서는 오류 로그와 재시도 대상으로 관리한다.

---

# 36. NotificationSetting Upsert

사용자당 알림 설정은 하나만 존재한다.

조회 시 설정이 없으면 기본값을 반환하거나 생성한다.

```ts
async getOrCreate(
  userId: string,
): Promise<NotificationSettingResponse> {
  const setting =
    await this.repository.upsert({
      where: {
        userId,
      },
      create: {
        userId,
        emailEnabled: true,
        pushEnabled: false,
        dueDateReminderEnabled: true,
        dueDateReminderMinutes: 1440,
        dailySummaryEnabled: false,
        dailySummaryTime: "20:00",
        timezone: "Asia/Seoul",
      },
      update: {},
    });

  return mapNotificationSetting(
    setting,
  );
}
```

부분 수정은 기존 설정과 DTO를 병합한 뒤 최종 조합을 검증한다.

```ts
const merged = {
  ...current,
  ...dto,
};

notificationSettingSchema.parse(
  merged,
);
```

---

# 37. Dashboard Service

Dashboard는 여러 통계 쿼리를 병렬로 실행한다.

```ts
async getDashboard(
  userId: string,
  query: DashboardQueryDto,
): Promise<DashboardResponse> {
  const [
    projectStatusGroups,
    taskStatusGroups,
    progressData,
    timeSummary,
    categoryProgress,
    weeklyCompletedTasks,
    upcomingDeadlines,
  ] = await Promise.all([
    this.repository
      .groupProjectsByStatus(
        userId,
        query,
      ),

    this.repository
      .groupTasksByStatus(
        userId,
        query,
      ),

    this.repository
      .getProjectProgressData(
        userId,
        query,
      ),

    this.repository
      .getTimeSummary(
        userId,
        query,
      ),

    this.repository
      .getCategoryProgress(
        userId,
        query,
      ),

    this.repository
      .getWeeklyCompletedTasks(
        userId,
        query,
      ),

    this.repository
      .getUpcomingDeadlines(
        userId,
        query,
      ),
  ]);

  return {
    projectSummary:
      buildProjectSummary(
        projectStatusGroups,
      ),

    taskSummary:
      buildTaskSummary(
        taskStatusGroups,
      ),

    averageProjectProgress:
      calculateAverageProgress(
        progressData,
      ),

    timeSummary,
    categoryProgress,
    weeklyCompletedTasks,
    upcomingDeadlines,
    recentActivities: [],
  };
}
```

대시보드 조회는 읽기 전용이므로 하나의 트랜잭션으로 묶을 필요가 없다. 완전히 동일한 시점의 통계가 필수라면 읽기 트랜잭션을 사용할 수 있다.

---

# 38. Controller 예시

## 38.1 Project Controller

```ts
import type {
  Request,
  Response,
} from "express";

export class ProjectController {
  constructor(
    private readonly service:
      ProjectService,
  ) {}

  create = async (
    req: Request,
    res: Response,
  ): Promise<void> => {
    const project =
      await this.service.create(
        req.user.id,
        req.body,
      );

    res.status(201).json({
      data: project,
    });
  };

  findDetail = async (
    req: Request,
    res: Response,
  ): Promise<void> => {
    const project =
      await this.service.findDetail(
        req.user.id,
        req.params.projectId,
      );

    res.status(200).json({
      data: project,
    });
  };

  archive = async (
    req: Request,
    res: Response,
  ): Promise<void> => {
    await this.service.archive(
      req.user.id,
      req.params.projectId,
    );

    res.status(204).send();
  };
}
```

비동기 오류 처리를 위해 Express 5를 사용하거나 `asyncHandler`를 적용한다.

```ts
export function asyncHandler(
  handler: RequestHandler,
): RequestHandler {
  return async (
    req,
    res,
    next,
  ) => {
    try {
      await handler(
        req,
        res,
        next,
      );
    } catch (error) {
      next(error);
    }
  };
}
```

---

# 39. Route 조립 예시

```ts
router.post(
  "/projects",
  authenticate,
  validateRequest(
    createProjectSchema.strict(),
    "body",
  ),
  asyncHandler(
    projectController.create,
  ),
);
```

```ts
router.patch(
  "/projects/:projectId/tasks/order",
  authenticate,
  validateRequest(
    projectIdParamSchema,
    "params",
  ),
  validateRequest(
    reorderTaskSchema.strict(),
    "body",
  ),
  asyncHandler(
    taskController.reorder,
  ),
);
```

```ts
router.patch(
  "/projects/:projectId/tasks/:taskId/move",
  authenticate,
  validateRequest(
    projectTaskParamSchema,
    "params",
  ),
  validateRequest(
    moveTaskSchema.strict(),
    "body",
  ),
  asyncHandler(
    taskController.move,
  ),
);
```

---

# 40. Repository에서 피해야 할 패턴

다음과 같이 ID만으로 수정하면 안 된다.

```ts
await prisma.project.update({
  where: {
    id: projectId,
  },
  data,
});
```

그 전에 반드시 Service에서 사용자 소유권을 확인해야 한다.

더 강한 방식을 원한다면 `updateMany`를 사용할 수 있다.

```ts
const result =
  await prisma.project.updateMany({
    where: {
      id: projectId,
      userId,
    },
    data,
  });

if (result.count === 0) {
  throw new AppError(
    404,
    "PROJECT_NOT_FOUND",
    "프로젝트를 찾을 수 없습니다.",
  );
}
```

단, `updateMany`는 수정된 레코드를 반환하지 않으므로 이후 재조회가 필요하다.

---

# 41. N+1 쿼리 방지

프로젝트 목록에서 각 프로젝트마다 Task 통계를 별도로 조회하면 N+1 문제가 발생한다.

피해야 할 구조:

```text
Project 20개 조회
각 Project마다 Task 통계 조회 20회
총 21회 이상 쿼리
```

초기 구현에서는 다음 방법 중 하나를 사용한다.

```text
Prisma _count로 단순 개수 조회
groupBy로 여러 Project 통계 일괄 조회
Raw SQL로 프로젝트별 상태 통계 조회
```

예시:

```ts
const taskGroups =
  await prisma.task.groupBy({
    by: [
      "projectId",
      "status",
    ],
    where: {
      projectId: {
        in: projectIds,
      },
    },
    _count: {
      _all: true,
    },
  });
```

메모리에서 프로젝트별 통계로 변환한다.

---

# 42. 인덱스 권장 사항

```prisma
model Project {
  // ...

  @@index([userId, isArchived])
  @@index([userId, status])
  @@index([categoryId])
  @@index([dueDate])
}
```

```prisma
model Task {
  // ...

  @@index([projectId, parentTaskId, position])
  @@index([projectId, status])
  @@index([milestoneId])
  @@index([dueDate])
  @@index([completedAt])
}
```

```prisma
model TimeRecord {
  // ...

  @@index([userId, startedAt])
  @@index([projectId, startedAt])
  @@index([taskId])
}
```

```prisma
model Note {
  // ...

  @@index([projectId])
  @@index([taskId])
  @@index([isPinned])
}
```

중간 테이블:

```prisma
model ProjectTag {
  projectId String @db.Uuid
  tagId     String @db.Uuid

  @@id([projectId, tagId])
  @@index([tagId])
  @@map("project_tags")
}
```

```prisma
model TaskTag {
  taskId String @db.Uuid
  tagId  String @db.Uuid

  @@id([taskId, tagId])
  @@index([tagId])
  @@map("task_tags")
}
```

---

# 43. 트랜잭션이 필요한 작업

다음 작업은 반드시 트랜잭션으로 처리한다.

```text
Task 순서 변경
Task 이동
Task 삭제 후 position 재정렬
Milestone 순서 변경
Category 순서 변경
Project 영구 삭제
Tag 삭제와 관계 해제
타이머 종료 상태 갱신
회원 탈퇴와 관련 데이터 처리
```

일반 단일 행 수정에는 트랜잭션이 필수는 아니다.

---

# 44. 멱등성 정책

다음 요청은 여러 번 수행해도 같은 결과가 되도록 한다.

```text
프로젝트 보관
프로젝트 보관 해제
Tag 연결
Tag 연결 해제
타이머 종료
Task 완료 토글
```

예시:

```text
이미 보관된 Project에 archive 요청
→ 204 No Content

이미 연결된 Tag 연결 요청
→ 성공하며 중복 행 생성 안 함

이미 종료된 타이머 종료 요청
→ 기존 종료된 기록 반환
```

---

# 45. 테스트 우선순위

## 45.1 Project Service

```text
존재하지 않는 Category로 생성 실패
다른 사용자의 Project 조회 실패
보관된 Project 수정 실패
허용된 상태 전이 성공
허용되지 않은 상태 전이 실패
```

## 45.2 Task Service

```text
정상적인 최상위 Task 생성
정상적인 하위 Task 생성
하위 Task 아래 Task 생성 실패
다른 Project의 Milestone 연결 실패
다른 Project의 부모 Task 연결 실패
하위 Task가 있는 Task 삭제 실패
미완료 하위 Task가 있는 부모 완료 실패
순서 변경 누락 실패
Task 이동 후 양쪽 그룹 position 정상화
```

## 45.3 TimeRecord Service

```text
종료 시각이 시작 시각보다 빠르면 실패
다른 Project의 Task 연결 실패
활성 타이머 중복 시작 실패
이미 종료된 타이머 재종료 성공
durationMinutes 계산 정확성
```

## 45.4 Tag Service

```text
사용자별 이름 중복 실패
다른 사용자 Tag 연결 실패
중복 연결 시 행 추가 안 됨
연결되지 않은 Tag 해제 성공
```

---

# 46. 구현 순서

## 1단계: 공통 기반

```text
Prisma Client
환경 변수
AppError
전역 오류 처리
인증 Middleware
Zod 검증 Middleware
날짜 Mapper
페이지네이션 유틸리티
```

## 2단계: 기본 프로젝트 기능

```text
Category Repository·Service
Project Repository·Service
Project Mapper
Project CRUD
Project 상태 변경
Project 보관
```

## 3단계: 작업 관리

```text
Milestone CRUD
Task CRUD
Task 상태 변경
Task 완료 처리
Task 순서 변경
Task 이동
Task 진행률 계산
```

## 4단계: 부가 기능

```text
Tag
TimeRecord
Note
ProjectFile
NotificationSetting
```

## 5단계: 통계 및 최적화

```text
Dashboard
N+1 제거
인덱스 확인
쿼리 최적화
통합 테스트
```

---

# 47. 이번 단계 확정 사항

1. Controller는 요청과 응답 처리만 담당한다.
2. Service는 모든 비즈니스 규칙과 소유권 검증을 담당한다.
3. Repository는 Prisma 데이터 접근만 담당한다.
4. 다른 사용자의 리소스는 `404 Not Found`로 처리한다.
5. 보관된 Project는 기본적으로 수정할 수 없다.
6. Task의 최대 깊이는 1단계다.
7. Task 순서 변경과 이동은 트랜잭션으로 처리한다.
8. Milestone과 부모 Task는 반드시 동일한 Project에 속해야 한다.
9. 미완료 하위 Task가 있으면 부모 Task를 완료할 수 없다.
10. 프로젝트 진행률은 부모·하위 Task의 이중 집계를 방지한다.
11. `CANCELLED` Task는 진행률 계산에서 제외한다.
12. 프로젝트 목록 통계는 일괄 조회하여 N+1 문제를 방지한다.
13. Tag 이름은 사용자 범위에서 정규화 후 유일하게 관리한다.
14. 사용자별 활성 타이머는 하나만 허용한다.
15. TimeRecord는 Task 삭제 후에도 보존하는 방향을 권장한다.
16. API 응답은 Mapper를 통해 DB 구조와 분리한다.
17. 파일 업로드와 DB 생성 실패 사이의 보상 처리를 적용한다.
18. Category, Milestone, Task 순서 변경은 전체 ID 배열 방식으로 처리한다.
19. 보관·Tag 연결·타이머 종료 API는 가능한 범위에서 멱등성을 보장한다.
20. 구현은 공통 기반 → Project → Task → 부가 기능 → Dashboard 순서로 진행한다.
