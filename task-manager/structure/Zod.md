# Task Manager DTO 및 Zod 검증 설계서 v1.0

## 1. 설계 목표

이 문서는 REST API와 백엔드 Service 계층 사이에서 사용되는 데이터 구조를 정의한다.

주요 목적은 다음과 같다.

1. 클라이언트 요청 데이터 검증
2. API 응답 구조 통일
3. 데이터베이스 모델과 API 모델 분리
4. 사용자 입력 필드와 서버 관리 필드 분리
5. TypeScript 타입 안정성 확보
6. OpenAPI 명세 생성 기반 마련

---

# 2. 기본 기술 기준

```text
언어: TypeScript
런타임: Node.js
검증 라이브러리: Zod
API 형식: JSON
날짜·시간: ISO 8601 문자열
식별자: UUID
```

예시 패키지:

```bash
npm install zod
```

---

# 3. DTO 디렉터리 구조

```text
src/
├─ common/
│  ├─ dto/
│  │  ├─ api-response.dto.ts
│  │  ├─ pagination.dto.ts
│  │  └─ common-query.dto.ts
│  ├─ enums/
│  │  ├─ project-status.enum.ts
│  │  ├─ milestone-status.enum.ts
│  │  ├─ task-status.enum.ts
│  │  └─ priority.enum.ts
│  └─ validation/
│     └─ common.schema.ts
│
├─ auth/
│  └─ dto/
│     ├─ signup.dto.ts
│     ├─ login.dto.ts
│     └─ refresh-token.dto.ts
│
├─ users/
│  └─ dto/
│     ├─ update-user.dto.ts
│     └─ change-password.dto.ts
│
├─ categories/
│  └─ dto/
│     ├─ create-category.dto.ts
│     ├─ update-category.dto.ts
│     ├─ category-query.dto.ts
│     └─ reorder-category.dto.ts
│
├─ projects/
│  └─ dto/
│     ├─ create-project.dto.ts
│     ├─ update-project.dto.ts
│     ├─ project-query.dto.ts
│     └─ update-project-status.dto.ts
│
├─ milestones/
│  └─ dto/
│     ├─ create-milestone.dto.ts
│     ├─ update-milestone.dto.ts
│     ├─ milestone-query.dto.ts
│     ├─ update-milestone-status.dto.ts
│     └─ reorder-milestone.dto.ts
│
├─ tasks/
│  └─ dto/
│     ├─ create-task.dto.ts
│     ├─ update-task.dto.ts
│     ├─ task-query.dto.ts
│     ├─ update-task-status.dto.ts
│     ├─ update-task-completion.dto.ts
│     ├─ reorder-task.dto.ts
│     └─ move-task.dto.ts
│
├─ tags/
│  └─ dto/
│     ├─ create-tag.dto.ts
│     ├─ update-tag.dto.ts
│     └─ attach-tags.dto.ts
│
├─ time-records/
│  └─ dto/
│     ├─ create-time-record.dto.ts
│     ├─ start-time-record.dto.ts
│     ├─ stop-time-record.dto.ts
│     ├─ update-time-record.dto.ts
│     └─ time-record-query.dto.ts
│
├─ notes/
│  └─ dto/
│     ├─ create-note.dto.ts
│     └─ update-note.dto.ts
│
├─ project-files/
│  └─ dto/
│     └─ update-project-file.dto.ts
│
└─ notification-settings/
   └─ dto/
      └─ update-notification-setting.dto.ts
```

---

# 4. 공통 Enum

## 4.1 ProjectStatus

```ts
export const PROJECT_STATUSES = [
  "PLANNED",
  "IN_PROGRESS",
  "PAUSED",
  "COMPLETED",
  "CANCELLED",
] as const;

export type ProjectStatus = (typeof PROJECT_STATUSES)[number];
```

## 4.2 MilestoneStatus

```ts
export const MILESTONE_STATUSES = [
  "PLANNED",
  "IN_PROGRESS",
  "COMPLETED",
] as const;

export type MilestoneStatus = (typeof MILESTONE_STATUSES)[number];
```

## 4.3 TaskStatus

```ts
export const TASK_STATUSES = [
  "TODO",
  "IN_PROGRESS",
  "BLOCKED",
  "COMPLETED",
  "CANCELLED",
] as const;

export type TaskStatus = (typeof TASK_STATUSES)[number];
```

## 4.4 Priority

```ts
export const PRIORITIES = [
  "LOW",
  "MEDIUM",
  "HIGH",
  "URGENT",
] as const;

export type Priority = (typeof PRIORITIES)[number];
```

---

# 5. 공통 Zod 스키마

## 5.1 UUID

```ts
import { z } from "zod";

export const uuidSchema = z
  .string()
  .uuid("올바른 UUID 형식이어야 합니다.");
```

## 5.2 날짜

```ts
export const dateSchema = z
  .string()
  .regex(
    /^\d{4}-\d{2}-\d{2}$/,
    "날짜는 YYYY-MM-DD 형식이어야 합니다.",
  );
```

## 5.3 날짜와 시간

```ts
export const dateTimeSchema = z
  .string()
  .datetime({
    offset: true,
    message: "날짜와 시간은 ISO 8601 형식이어야 합니다.",
  });
```

## 5.4 페이지네이션

REST API의 페이지 번호는 클라이언트에서 1부터 시작한다.

```ts
export const paginationQuerySchema = z.object({
  page: z.coerce
    .number()
    .int()
    .min(1)
    .default(1),

  size: z.coerce
    .number()
    .int()
    .min(1)
    .max(100)
    .default(20),
});
```

## 5.5 정렬

```ts
export const sortDirectionSchema = z.enum(["asc", "desc"]);

export const sortSchema = z
  .string()
  .regex(
    /^[a-zA-Z][a-zA-Z0-9]*,(asc|desc)$/,
    "정렬 형식은 field,asc 또는 field,desc여야 합니다.",
  )
  .optional();
```

## 5.6 색상

```ts
export const hexColorSchema = z
  .string()
  .regex(
    /^#[0-9A-Fa-f]{6}$/,
    "색상은 #RRGGBB 형식이어야 합니다.",
  );
```

## 5.7 날짜 범위 검증 함수

```ts
import type { RefinementCtx } from "zod";

export function validateDateRange(
  data: {
    startDate?: string | null;
    dueDate?: string | null;
  },
  ctx: RefinementCtx,
): void {
  if (
    data.startDate &&
    data.dueDate &&
    data.startDate > data.dueDate
  ) {
    ctx.addIssue({
      code: "custom",
      path: ["dueDate"],
      message: "마감일은 시작일보다 빠를 수 없습니다.",
    });
  }
}
```

---

# 6. 공통 응답 DTO

## 6.1 단일 리소스 응답

```ts
export interface ApiResponse<T> {
  data: T;
}
```

## 6.2 페이지네이션 응답

```ts
export interface PaginationMeta {
  page: number;
  size: number;
  totalElements: number;
  totalPages: number;
}
```

```ts
export interface PaginatedResponse<T> {
  data: T[];
  pagination: PaginationMeta;
}
```

## 6.3 오류 응답

```ts
export interface FieldError {
  field: string;
  reason: string;
}
```

```ts
export interface ApiErrorBody {
  code: string;
  message: string;
  details:
    | FieldError[]
    | Record<string, unknown>
    | null;
}
```

```ts
export interface ApiErrorResponse {
  error: ApiErrorBody;
}
```

---

# 7. 인증 DTO

## 7.1 회원가입 요청

### 필드

| 필드            | 타입     | 필수 | 제한              |
| ------------- | ------ | -: | --------------- |
| `email`       | string |  O | 이메일 형식, 최대 255자 |
| `password`    | string |  O | 8~72자           |
| `displayName` | string |  O | 2~50자           |

### Zod 스키마

```ts
import { z } from "zod";

export const signupSchema = z.object({
  email: z
    .string()
    .trim()
    .email("올바른 이메일 형식이어야 합니다.")
    .max(255)
    .transform((value) => value.toLowerCase()),

  password: z
    .string()
    .min(8, "비밀번호는 8자 이상이어야 합니다.")
    .max(72, "비밀번호는 72자 이하여야 합니다.")
    .regex(
      /[A-Za-z]/,
      "비밀번호에는 영문자가 포함되어야 합니다.",
    )
    .regex(
      /\d/,
      "비밀번호에는 숫자가 포함되어야 합니다.",
    ),

  displayName: z
    .string()
    .trim()
    .min(2)
    .max(50),
});

export type SignupDto = z.infer<typeof signupSchema>;
```

특수문자를 반드시 요구할지는 보안 정책 단계에서 결정한다. 초기 버전에서는 문자와 숫자 조합을 필수로 한다.

## 7.2 로그인 요청

```ts
export const loginSchema = z.object({
  email: z
    .string()
    .trim()
    .email()
    .max(255)
    .transform((value) => value.toLowerCase()),

  password: z
    .string()
    .min(1)
    .max(72),
});

export type LoginDto = z.infer<typeof loginSchema>;
```

## 7.3 토큰 재발급 요청

```ts
export const refreshTokenSchema = z.object({
  refreshToken: z
    .string()
    .min(1, "Refresh Token은 필수입니다."),
});

export type RefreshTokenDto =
  z.infer<typeof refreshTokenSchema>;
```

## 7.4 인증 응답

```ts
export interface AuthUserResponse {
  id: string;
  email: string;
  displayName: string;
}
```

```ts
export interface AuthResponse {
  user: AuthUserResponse;
  accessToken: string;
  refreshToken: string;
}
```

---

# 8. User DTO

## 8.1 사용자 정보 수정

```ts
export const updateUserSchema = z
  .object({
    displayName: z
      .string()
      .trim()
      .min(2)
      .max(50)
      .optional(),
  })
  .refine(
    (data) => Object.keys(data).length > 0,
    "수정할 필드가 하나 이상 필요합니다.",
  );

export type UpdateUserDto =
  z.infer<typeof updateUserSchema>;
```

## 8.2 비밀번호 변경

```ts
export const changePasswordSchema = z
  .object({
    currentPassword: z
      .string()
      .min(1)
      .max(72),

    newPassword: z
      .string()
      .min(8)
      .max(72)
      .regex(/[A-Za-z]/)
      .regex(/\d/),
  })
  .refine(
    (data) =>
      data.currentPassword !== data.newPassword,
    {
      path: ["newPassword"],
      message:
        "새 비밀번호는 현재 비밀번호와 달라야 합니다.",
    },
  );

export type ChangePasswordDto =
  z.infer<typeof changePasswordSchema>;
```

---

# 9. Category DTO

## 9.1 Category 생성

### 필드

| 필드            | 타입     | 필수 | 제한                    |
| ------------- | ------ | -: | --------------------- |
| `name`        | string |  O | 1~50자                 |
| `slug`        | string |  O | 영문 소문자·숫자·하이픈, 최대 50자 |
| `description` | string |  X | 최대 255자               |
| `icon`        | string |  X | 최대 50자                |
| `color`       | string |  X | `#RRGGBB`             |

```ts
export const createCategorySchema = z.object({
  name: z
    .string()
    .trim()
    .min(1)
    .max(50),

  slug: z
    .string()
    .trim()
    .min(1)
    .max(50)
    .regex(
      /^[a-z0-9]+(?:-[a-z0-9]+)*$/,
      "slug는 영문 소문자, 숫자, 하이픈만 사용할 수 있습니다.",
    ),

  description: z
    .string()
    .trim()
    .max(255)
    .nullable()
    .optional(),

  icon: z
    .string()
    .trim()
    .max(50)
    .nullable()
    .optional(),

  color: hexColorSchema
    .nullable()
    .optional(),
});

export type CreateCategoryDto =
  z.infer<typeof createCategorySchema>;
```

## 9.2 Category 수정

```ts
export const updateCategorySchema =
  createCategorySchema
    .partial()
    .extend({
      isActive: z.boolean().optional(),
    })
    .refine(
      (data) => Object.keys(data).length > 0,
      "수정할 필드가 하나 이상 필요합니다.",
    );

export type UpdateCategoryDto =
  z.infer<typeof updateCategorySchema>;
```

## 9.3 Category 목록 조회

```ts
export const categoryQuerySchema = z.object({
  includeInactive: z.coerce
    .boolean()
    .default(false),

  sort: sortSchema,
});

export type CategoryQueryDto =
  z.infer<typeof categoryQuerySchema>;
```

## 9.4 Category 순서 변경

```ts
export const reorderCategorySchema = z.object({
  categoryIds: z
    .array(uuidSchema)
    .min(1)
    .refine(
      (ids) => new Set(ids).size === ids.length,
      "중복된 Category ID가 존재합니다.",
    ),
});

export type ReorderCategoryDto =
  z.infer<typeof reorderCategorySchema>;
```

## 9.5 Category 응답

```ts
export interface CategoryResponse {
  id: string;
  name: string;
  slug: string;
  description: string | null;
  icon: string | null;
  color: string | null;
  isActive: boolean;
  position: number;
  createdAt: string;
  updatedAt: string;
}
```

---

# 10. Project DTO

## 10.1 프로젝트 생성

### 필드

| 필드            | 타입     | 필수 | 제한              |
| ------------- | ------ | -: | --------------- |
| `categoryId`  | UUID   |  O | 사용 가능한 Category |
| `title`       | string |  O | 1~100자          |
| `description` | string |  X | 최대 2,000자       |
| `status`      | enum   |  X | 기본값 `PLANNED`   |
| `priority`    | enum   |  X | 기본값 `MEDIUM`    |
| `startDate`   | date   |  X | nullable        |
| `dueDate`     | date   |  X | nullable        |

```ts
import {
  PROJECT_STATUSES,
  PRIORITIES,
} from "../../common/enums";

export const createProjectSchema = z
  .object({
    categoryId: uuidSchema,

    title: z
      .string()
      .trim()
      .min(1)
      .max(100),

    description: z
      .string()
      .trim()
      .max(2000)
      .nullable()
      .optional(),

    status: z
      .enum(PROJECT_STATUSES)
      .default("PLANNED"),

    priority: z
      .enum(PRIORITIES)
      .default("MEDIUM"),

    startDate: dateSchema
      .nullable()
      .optional(),

    dueDate: dateSchema
      .nullable()
      .optional(),
  })
  .superRefine(validateDateRange);

export type CreateProjectDto =
  z.infer<typeof createProjectSchema>;
```

## 10.2 프로젝트 수정

`null`은 기존 값을 제거한다는 의미다.

예시:

```json
{
  "dueDate": null
}
```

```ts
export const updateProjectSchema = z
  .object({
    categoryId: uuidSchema.optional(),

    title: z
      .string()
      .trim()
      .min(1)
      .max(100)
      .optional(),

    description: z
      .string()
      .trim()
      .max(2000)
      .nullable()
      .optional(),

    priority: z
      .enum(PRIORITIES)
      .optional(),

    startDate: dateSchema
      .nullable()
      .optional(),

    dueDate: dateSchema
      .nullable()
      .optional(),
  })
  .superRefine((data, ctx) => {
    if (Object.keys(data).length === 0) {
      ctx.addIssue({
        code: "custom",
        message:
          "수정할 필드가 하나 이상 필요합니다.",
      });
    }

    validateDateRange(data, ctx);
  });

export type UpdateProjectDto =
  z.infer<typeof updateProjectSchema>;
```

부분 수정에서는 DB의 기존 `startDate` 또는 `dueDate`와 합친 뒤 Service 계층에서 최종 날짜 범위를 다시 검증해야 한다.

## 10.3 프로젝트 상태 변경

```ts
export const updateProjectStatusSchema = z.object({
  status: z.enum(PROJECT_STATUSES),
});

export type UpdateProjectStatusDto =
  z.infer<typeof updateProjectStatusSchema>;
```

## 10.4 프로젝트 목록 조회

```ts
export const PROJECT_SORT_FIELDS = [
  "createdAt",
  "updatedAt",
  "title",
  "startDate",
  "dueDate",
  "priority",
  "status",
] as const;
```

```ts
export const projectQuerySchema = z.object({
  categoryId: uuidSchema.optional(),

  status: z
    .enum(PROJECT_STATUSES)
    .optional(),

  priority: z
    .enum(PRIORITIES)
    .optional(),

  isArchived: z.coerce
    .boolean()
    .default(false),

  tagId: uuidSchema.optional(),

  keyword: z
    .string()
    .trim()
    .max(100)
    .optional(),

  dueBefore: dateSchema.optional(),
  dueAfter: dateSchema.optional(),

  sort: z
    .string()
    .regex(
      /^(createdAt|updatedAt|title|startDate|dueDate|priority|status),(asc|desc)$/,
    )
    .optional(),

  page: z.coerce
    .number()
    .int()
    .min(1)
    .default(1),

  size: z.coerce
    .number()
    .int()
    .min(1)
    .max(100)
    .default(20),
}).refine(
  (data) =>
    !data.dueBefore ||
    !data.dueAfter ||
    data.dueAfter <= data.dueBefore,
  {
    path: ["dueBefore"],
    message:
      "dueBefore는 dueAfter보다 빠를 수 없습니다.",
  },
);

export type ProjectQueryDto =
  z.infer<typeof projectQuerySchema>;
```

## 10.5 프로젝트 응답

```ts
export interface ProjectCategorySummary {
  id: string;
  name: string;
  slug: string;
}
```

```ts
export interface TagSummary {
  id: string;
  name: string;
  color: string | null;
}
```

```ts
export interface ProjectTaskSummary {
  total: number;
  todo: number;
  inProgress: number;
  blocked: number;
  completed: number;
  cancelled: number;
}
```

```ts
export interface ProjectResponse {
  id: string;
  category: ProjectCategorySummary;
  title: string;
  description: string | null;
  status: ProjectStatus;
  priority: Priority;
  startDate: string | null;
  dueDate: string | null;
  isArchived: boolean;
  progress: number;
  taskSummary: ProjectTaskSummary;
  tags: TagSummary[];
  createdAt: string;
  updatedAt: string;
}
```

---

# 11. Milestone DTO

## 11.1 Milestone 생성

```ts
export const createMilestoneSchema = z.object({
  title: z
    .string()
    .trim()
    .min(1)
    .max(100),

  description: z
    .string()
    .trim()
    .max(1000)
    .nullable()
    .optional(),

  dueDate: dateSchema
    .nullable()
    .optional(),

  position: z
    .number()
    .int()
    .min(0)
    .optional(),
});

export type CreateMilestoneDto =
  z.infer<typeof createMilestoneSchema>;
```

`status`는 생성 시 서버에서 기본값 `PLANNED`로 설정한다.

## 11.2 Milestone 수정

```ts
export const updateMilestoneSchema =
  createMilestoneSchema
    .partial()
    .refine(
      (data) => Object.keys(data).length > 0,
      "수정할 필드가 하나 이상 필요합니다.",
    );

export type UpdateMilestoneDto =
  z.infer<typeof updateMilestoneSchema>;
```

## 11.3 Milestone 상태 변경

```ts
export const updateMilestoneStatusSchema = z.object({
  status: z.enum(MILESTONE_STATUSES),
});

export type UpdateMilestoneStatusDto =
  z.infer<typeof updateMilestoneStatusSchema>;
```

## 11.4 Milestone 목록 조회

```ts
export const milestoneQuerySchema = z.object({
  status: z
    .enum(MILESTONE_STATUSES)
    .optional(),

  sort: z
    .string()
    .regex(
      /^(position|createdAt|dueDate|title),(asc|desc)$/,
    )
    .optional(),
});

export type MilestoneQueryDto =
  z.infer<typeof milestoneQuerySchema>;
```

## 11.5 Milestone 순서 변경

```ts
export const reorderMilestoneSchema = z.object({
  milestoneIds: z
    .array(uuidSchema)
    .min(1)
    .refine(
      (ids) => new Set(ids).size === ids.length,
      "중복된 Milestone ID가 존재합니다.",
    ),
});

export type ReorderMilestoneDto =
  z.infer<typeof reorderMilestoneSchema>;
```

## 11.6 Milestone 응답

```ts
export interface MilestoneTaskSummary {
  total: number;
  completed: number;
}
```

```ts
export interface MilestoneResponse {
  id: string;
  projectId: string;
  title: string;
  description: string | null;
  status: MilestoneStatus;
  dueDate: string | null;
  position: number;
  progress: number;
  taskSummary: MilestoneTaskSummary;
  createdAt: string;
  updatedAt: string;
}
```

---

# 12. Task DTO

## 12.1 Task 생성

### 필드

| 필드                 | 타입      | 필수 | 제한          |
| ------------------ | ------- | -: | ----------- |
| `milestoneId`      | UUID    |  X | nullable    |
| `parentTaskId`     | UUID    |  X | nullable    |
| `title`            | string  |  O | 1~150자      |
| `description`      | string  |  X | 최대 3,000자   |
| `status`           | enum    |  X | 기본 `TODO`   |
| `priority`         | enum    |  X | 기본 `MEDIUM` |
| `startDate`        | date    |  X | nullable    |
| `dueDate`          | date    |  X | nullable    |
| `estimatedMinutes` | integer |  X | 0~525,600   |
| `position`         | integer |  X | 0 이상        |

```ts
export const createTaskSchema = z
  .object({
    milestoneId: uuidSchema
      .nullable()
      .optional(),

    parentTaskId: uuidSchema
      .nullable()
      .optional(),

    title: z
      .string()
      .trim()
      .min(1)
      .max(150),

    description: z
      .string()
      .trim()
      .max(3000)
      .nullable()
      .optional(),

    status: z
      .enum(TASK_STATUSES)
      .default("TODO"),

    priority: z
      .enum(PRIORITIES)
      .default("MEDIUM"),

    startDate: dateSchema
      .nullable()
      .optional(),

    dueDate: dateSchema
      .nullable()
      .optional(),

    estimatedMinutes: z
      .number()
      .int()
      .min(0)
      .max(525600)
      .nullable()
      .optional(),

    position: z
      .number()
      .int()
      .min(0)
      .optional(),
  })
  .superRefine(validateDateRange);

export type CreateTaskDto =
  z.infer<typeof createTaskSchema>;
```

다음 검증은 데이터베이스 조회가 필요하므로 Service 계층에서 수행한다.

```text
parentTask가 같은 Project에 속하는가
parentTask가 이미 하위 Task인가
milestone이 같은 Project에 속하는가
Project가 보관 상태인가
Task 기간이 Project 기간 정책을 위반하는가
```

## 12.2 Task 수정

```ts
export const updateTaskSchema = z
  .object({
    milestoneId: uuidSchema
      .nullable()
      .optional(),

    title: z
      .string()
      .trim()
      .min(1)
      .max(150)
      .optional(),

    description: z
      .string()
      .trim()
      .max(3000)
      .nullable()
      .optional(),

    priority: z
      .enum(PRIORITIES)
      .optional(),

    startDate: dateSchema
      .nullable()
      .optional(),

    dueDate: dateSchema
      .nullable()
      .optional(),

    estimatedMinutes: z
      .number()
      .int()
      .min(0)
      .max(525600)
      .nullable()
      .optional(),
  })
  .superRefine((data, ctx) => {
    if (Object.keys(data).length === 0) {
      ctx.addIssue({
        code: "custom",
        message:
          "수정할 필드가 하나 이상 필요합니다.",
      });
    }

    validateDateRange(data, ctx);
  });

export type UpdateTaskDto =
  z.infer<typeof updateTaskSchema>;
```

`parentTaskId`와 `position` 변경은 일반 수정 API가 아니라 `/move` API로만 처리한다.

## 12.3 Task 상태 변경

```ts
export const updateTaskStatusSchema = z.object({
  status: z.enum(TASK_STATUSES),
});

export type UpdateTaskStatusDto =
  z.infer<typeof updateTaskStatusSchema>;
```

## 12.4 완료 상태 변경

```ts
export const updateTaskCompletionSchema = z.object({
  isCompleted: z.boolean(),
});

export type UpdateTaskCompletionDto =
  z.infer<typeof updateTaskCompletionSchema>;
```

처리 규칙:

```text
isCompleted = true
→ status = COMPLETED
→ completedAt = 현재 시각

isCompleted = false
→ status = TODO
→ completedAt = null
```

완료 해제 시 기존 상태를 복원하려면 별도의 `previousStatus` 컬럼 또는 상태 이력이 필요하다. 초기 버전에서는 `TODO`로 되돌린다.

## 12.5 Task 목록 조회

```ts
export const taskQuerySchema = z.object({
  milestoneId: uuidSchema.optional(),

  parentTaskId: uuidSchema.optional(),

  rootOnly: z.coerce
    .boolean()
    .optional(),

  status: z
    .enum(TASK_STATUSES)
    .optional(),

  priority: z
    .enum(PRIORITIES)
    .optional(),

  tagId: uuidSchema.optional(),

  dueBefore: dateSchema.optional(),

  dueAfter: dateSchema.optional(),

  keyword: z
    .string()
    .trim()
    .max(100)
    .optional(),

  sort: z
    .string()
    .regex(
      /^(position|createdAt|updatedAt|title|startDate|dueDate|priority|status),(asc|desc)$/,
    )
    .optional(),

  page: z.coerce
    .number()
    .int()
    .min(1)
    .default(1),

  size: z.coerce
    .number()
    .int()
    .min(1)
    .max(100)
    .default(20),
}).superRefine((data, ctx) => {
  if (data.rootOnly && data.parentTaskId) {
    ctx.addIssue({
      code: "custom",
      path: ["parentTaskId"],
      message:
        "rootOnly와 parentTaskId는 함께 사용할 수 없습니다.",
    });
  }

  if (
    data.dueAfter &&
    data.dueBefore &&
    data.dueAfter > data.dueBefore
  ) {
    ctx.addIssue({
      code: "custom",
      path: ["dueBefore"],
      message:
        "dueBefore는 dueAfter보다 빠를 수 없습니다.",
    });
  }
});

export type TaskQueryDto =
  z.infer<typeof taskQuerySchema>;
```

## 12.6 Task 순서 변경

```ts
export const reorderTaskSchema = z.object({
  parentTaskId: uuidSchema
    .nullable(),

  taskIds: z
    .array(uuidSchema)
    .min(1)
    .refine(
      (ids) => new Set(ids).size === ids.length,
      "중복된 Task ID가 존재합니다.",
    ),
});

export type ReorderTaskDto =
  z.infer<typeof reorderTaskSchema>;
```

최상위 Task를 정렬할 때:

```json
{
  "parentTaskId": null,
  "taskIds": []
}
```

하위 Task를 정렬할 때:

```json
{
  "parentTaskId": "parent-task-uuid",
  "taskIds": []
}
```

## 12.7 Task 이동

```ts
export const moveTaskSchema = z.object({
  milestoneId: uuidSchema
    .nullable()
    .optional(),

  parentTaskId: uuidSchema
    .nullable()
    .optional(),

  position: z
    .number()
    .int()
    .min(0),
});

export type MoveTaskDto =
  z.infer<typeof moveTaskSchema>;
```

`undefined`와 `null`의 의미를 구분한다.

```text
undefined
→ 기존 관계를 유지한다.

null
→ 기존 관계를 제거한다.
```

예시:

```json
{
  "parentTaskId": null,
  "position": 2
}
```

위 요청은 Task를 최상위 Task로 이동한다.

## 12.8 Task 응답

```ts
export interface SubtaskSummary {
  total: number;
  completed: number;
}
```

```ts
export interface TaskResponse {
  id: string;
  projectId: string;
  milestoneId: string | null;
  parentTaskId: string | null;
  title: string;
  description: string | null;
  status: TaskStatus;
  priority: Priority;
  startDate: string | null;
  dueDate: string | null;
  estimatedMinutes: number | null;
  actualMinutes: number;
  position: number;
  completedAt: string | null;
  subtaskSummary: SubtaskSummary;
  tags: TagSummary[];
  createdAt: string;
  updatedAt: string;
}
```

상세 응답:

```ts
export interface TaskDetailResponse
  extends TaskResponse {
  subtasks: TaskResponse[];
}
```

---

# 13. Tag DTO

## 13.1 Tag 생성

```ts
export const createTagSchema = z.object({
  name: z
    .string()
    .trim()
    .min(1)
    .max(50),

  color: hexColorSchema
    .nullable()
    .optional(),
});

export type CreateTagDto =
  z.infer<typeof createTagSchema>;
```

Tag 이름은 사용자 범위 안에서 대소문자를 무시하고 유일해야 한다.

```text
C++
c++
C++

→ 동일한 Tag 이름으로 처리
```

## 13.2 Tag 수정

```ts
export const updateTagSchema =
  createTagSchema
    .partial()
    .refine(
      (data) => Object.keys(data).length > 0,
      "수정할 필드가 하나 이상 필요합니다.",
    );

export type UpdateTagDto =
  z.infer<typeof updateTagSchema>;
```

## 13.3 Tag 연결

```ts
export const attachTagsSchema = z.object({
  tagIds: z
    .array(uuidSchema)
    .min(1)
    .max(50)
    .refine(
      (ids) => new Set(ids).size === ids.length,
      "중복된 Tag ID가 존재합니다.",
    ),
});

export type AttachTagsDto =
  z.infer<typeof attachTagsSchema>;
```

## 13.4 Tag 응답

```ts
export interface TagResponse {
  id: string;
  name: string;
  color: string | null;
  projectCount: number;
  taskCount: number;
  createdAt: string;
  updatedAt: string;
}
```

---

# 14. TimeRecord DTO

## 14.1 시간 기록 생성

```ts
export const createTimeRecordSchema = z
  .object({
    taskId: uuidSchema
      .nullable()
      .optional(),

    startedAt: dateTimeSchema,

    endedAt: dateTimeSchema,

    description: z
      .string()
      .trim()
      .max(500)
      .nullable()
      .optional(),
  })
  .refine(
    (data) =>
      new Date(data.startedAt).getTime() <
      new Date(data.endedAt).getTime(),
    {
      path: ["endedAt"],
      message:
        "종료 시각은 시작 시각보다 늦어야 합니다.",
    },
  );

export type CreateTimeRecordDto =
  z.infer<typeof createTimeRecordSchema>;
```

`durationMinutes`는 요청으로 받지 않고 서버에서 계산한다.

```ts
const durationMinutes = Math.floor(
  (
    new Date(endedAt).getTime() -
    new Date(startedAt).getTime()
  ) / 60000,
);
```

## 14.2 타이머 시작

```ts
export const startTimeRecordSchema = z.object({
  taskId: uuidSchema
    .nullable()
    .optional(),

  description: z
    .string()
    .trim()
    .max(500)
    .nullable()
    .optional(),
});

export type StartTimeRecordDto =
  z.infer<typeof startTimeRecordSchema>;
```

`startedAt`은 서버 현재 시각으로 설정한다.

## 14.3 타이머 종료

```ts
export const stopTimeRecordSchema = z.object({
  endedAt: dateTimeSchema.optional(),
});

export type StopTimeRecordDto =
  z.infer<typeof stopTimeRecordSchema>;
```

`endedAt`을 생략하면 서버 현재 시각을 사용한다.

## 14.4 시간 기록 수정

```ts
export const updateTimeRecordSchema = z
  .object({
    taskId: uuidSchema
      .nullable()
      .optional(),

    startedAt: dateTimeSchema.optional(),

    endedAt: dateTimeSchema.optional(),

    description: z
      .string()
      .trim()
      .max(500)
      .nullable()
      .optional(),
  })
  .refine(
    (data) => Object.keys(data).length > 0,
    "수정할 필드가 하나 이상 필요합니다.",
  )
  .superRefine((data, ctx) => {
    if (
      data.startedAt &&
      data.endedAt &&
      new Date(data.startedAt).getTime() >=
        new Date(data.endedAt).getTime()
    ) {
      ctx.addIssue({
        code: "custom",
        path: ["endedAt"],
        message:
          "종료 시각은 시작 시각보다 늦어야 합니다.",
      });
    }
  });

export type UpdateTimeRecordDto =
  z.infer<typeof updateTimeRecordSchema>;
```

기존 DB 값과 합친 최종 시간 범위는 Service 계층에서 다시 검증한다.

## 14.5 시간 기록 목록 조회

```ts
export const timeRecordQuerySchema = z.object({
  startDate: dateSchema.optional(),
  endDate: dateSchema.optional(),

  page: z.coerce
    .number()
    .int()
    .min(1)
    .default(1),

  size: z.coerce
    .number()
    .int()
    .min(1)
    .max(100)
    .default(20),
}).refine(
  (data) =>
    !data.startDate ||
    !data.endDate ||
    data.startDate <= data.endDate,
  {
    path: ["endDate"],
    message:
      "종료일은 시작일보다 빠를 수 없습니다.",
  },
);

export type TimeRecordQueryDto =
  z.infer<typeof timeRecordQuerySchema>;
```

## 14.6 시간 기록 응답

```ts
export interface TimeRecordResponse {
  id: string;
  projectId: string;
  taskId: string | null;
  startedAt: string;
  endedAt: string | null;
  durationMinutes: number | null;
  description: string | null;
  createdAt: string;
  updatedAt: string;
}
```

---

# 15. Note DTO

## 15.1 Note 생성

```ts
export const createNoteSchema = z.object({
  title: z
    .string()
    .trim()
    .min(1)
    .max(100),

  content: z
    .string()
    .max(20000),

  isPinned: z
    .boolean()
    .default(false),
});

export type CreateNoteDto =
  z.infer<typeof createNoteSchema>;
```

노트 생성 경로에 따라 관계가 결정된다.

```text
POST /projects/{projectId}/notes
→ projectId 설정
→ taskId = null

POST /projects/{projectId}/tasks/{taskId}/notes
→ projectId 설정
→ taskId 설정
```

클라이언트는 본문에 `projectId`나 `taskId`를 전달하지 않는다.

## 15.2 Note 수정

```ts
export const updateNoteSchema = z
  .object({
    title: z
      .string()
      .trim()
      .min(1)
      .max(100)
      .optional(),

    content: z
      .string()
      .max(20000)
      .optional(),

    isPinned: z
      .boolean()
      .optional(),
  })
  .refine(
    (data) => Object.keys(data).length > 0,
    "수정할 필드가 하나 이상 필요합니다.",
  );

export type UpdateNoteDto =
  z.infer<typeof updateNoteSchema>;
```

## 15.3 Note 응답

```ts
export interface NoteResponse {
  id: string;
  projectId: string;
  taskId: string | null;
  title: string;
  content: string;
  isPinned: boolean;
  createdAt: string;
  updatedAt: string;
}
```

---

# 16. ProjectFile DTO

파일 업로드 자체는 `multipart/form-data`로 처리하므로 일반 JSON DTO와 분리한다.

## 16.1 업로드 메타데이터

```ts
export const uploadProjectFileMetadataSchema =
  z.object({
    description: z
      .string()
      .trim()
      .max(500)
      .nullable()
      .optional(),
  });
```

## 16.2 파일 정보 수정

```ts
export const updateProjectFileSchema = z.object({
  description: z
    .string()
    .trim()
    .max(500)
    .nullable(),
});

export type UpdateProjectFileDto =
  z.infer<typeof updateProjectFileSchema>;
```

## 16.3 서버 파일 검증

```ts
export const ALLOWED_FILE_MIME_TYPES = [
  "application/pdf",
  "image/png",
  "image/jpeg",
  "image/webp",
  "text/plain",
  "application/zip",
  "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
  "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
  "application/vnd.openxmlformats-officedocument.presentationml.presentation",
] as const;
```

```ts
export const MAX_FILE_SIZE_BYTES =
  20 * 1024 * 1024;
```

초기 최대 크기:

```text
20MB
```

확장자만 검사하지 않고 MIME 타입, 파일 크기, 실제 파일 시그니처를 함께 검사하는 것이 바람직하다.

## 16.4 파일 응답

```ts
export interface ProjectFileResponse {
  id: string;
  projectId: string;
  originalName: string;
  mimeType: string;
  sizeBytes: number;
  description: string | null;
  downloadUrl: string;
  createdAt: string;
  updatedAt: string;
}
```

실제 저장 경로나 스토리지 키는 API 응답에 노출하지 않는다.

---

# 17. NotificationSetting DTO

## 17.1 알림 설정 수정

```ts
export const notificationSettingSchema = z.object({
  emailEnabled: z.boolean(),

  pushEnabled: z.boolean(),

  dueDateReminderEnabled: z.boolean(),

  dueDateReminderMinutes: z
    .number()
    .int()
    .min(0)
    .max(10080),

  dailySummaryEnabled: z.boolean(),

  dailySummaryTime: z
    .string()
    .regex(
      /^([01]\d|2[0-3]):[0-5]\d$/,
      "시간은 HH:mm 형식이어야 합니다.",
    ),

  timezone: z
    .string()
    .min(1)
    .max(100),
}).superRefine((data, ctx) => {
  if (
    data.dueDateReminderEnabled &&
    data.dueDateReminderMinutes <= 0
  ) {
    ctx.addIssue({
      code: "custom",
      path: ["dueDateReminderMinutes"],
      message:
        "마감 알림이 활성화된 경우 알림 시간을 지정해야 합니다.",
    });
  }
});

export const updateNotificationSettingSchema =
  notificationSettingSchema
    .partial()
    .refine(
      (data) => Object.keys(data).length > 0,
      "수정할 필드가 하나 이상 필요합니다.",
    );

export type UpdateNotificationSettingDto =
  z.infer<
    typeof updateNotificationSettingSchema
  >;
```

부분 수정이므로 최종 설정 조합의 유효성은 기존 값과 병합한 뒤 Service 계층에서 다시 검사한다.

## 17.2 알림 설정 응답

```ts
export interface NotificationSettingResponse {
  emailEnabled: boolean;
  pushEnabled: boolean;
  dueDateReminderEnabled: boolean;
  dueDateReminderMinutes: number;
  dailySummaryEnabled: boolean;
  dailySummaryTime: string;
  timezone: string;
  updatedAt: string;
}
```

---

# 18. Dashboard Query DTO

## 18.1 대시보드 조회

```ts
export const dashboardQuerySchema = z.object({
  startDate: dateSchema.optional(),
  endDate: dateSchema.optional(),
  categoryId: uuidSchema.optional(),
}).refine(
  (data) =>
    !data.startDate ||
    !data.endDate ||
    data.startDate <= data.endDate,
  {
    path: ["endDate"],
    message:
      "종료일은 시작일보다 빠를 수 없습니다.",
  },
);

export type DashboardQueryDto =
  z.infer<typeof dashboardQuerySchema>;
```

## 18.2 미완료 Task 조회

```ts
export const incompleteTaskQuerySchema = z.object({
  categoryId: uuidSchema.optional(),
  projectId: uuidSchema.optional(),
  milestoneId: uuidSchema.optional(),

  priority: z
    .enum(PRIORITIES)
    .optional(),

  dueBefore: dateSchema.optional(),

  sort: z
    .string()
    .regex(
      /^(dueDate|priority|createdAt|title),(asc|desc)$/,
    )
    .optional(),

  page: z.coerce
    .number()
    .int()
    .min(1)
    .default(1),

  size: z.coerce
    .number()
    .int()
    .min(1)
    .max(100)
    .default(20),
});

export type IncompleteTaskQueryDto =
  z.infer<typeof incompleteTaskQuerySchema>;
```

## 18.3 시간 통계 조회

```ts
export const TIME_STATISTIC_GROUPS = [
  "day",
  "week",
  "month",
  "category",
  "project",
] as const;
```

```ts
export const timeStatisticsQuerySchema = z
  .object({
    startDate: dateSchema,
    endDate: dateSchema,

    groupBy: z.enum(
      TIME_STATISTIC_GROUPS,
    ),
  })
  .refine(
    (data) => data.startDate <= data.endDate,
    {
      path: ["endDate"],
      message:
        "종료일은 시작일보다 빠를 수 없습니다.",
    },
  );

export type TimeStatisticsQueryDto =
  z.infer<typeof timeStatisticsQuerySchema>;
```

---

# 19. Path Parameter 스키마

## 19.1 Project ID

```ts
export const projectIdParamSchema = z.object({
  projectId: uuidSchema,
});
```

## 19.2 Project와 Task ID

```ts
export const projectTaskParamSchema = z.object({
  projectId: uuidSchema,
  taskId: uuidSchema,
});
```

## 19.3 Project와 Milestone ID

```ts
export const projectMilestoneParamSchema =
  z.object({
    projectId: uuidSchema,
    milestoneId: uuidSchema,
  });
```

## 19.4 Project, Task, Tag ID

```ts
export const projectTaskTagParamSchema =
  z.object({
    projectId: uuidSchema,
    taskId: uuidSchema,
    tagId: uuidSchema,
  });
```

URL 파라미터도 요청 본문과 동일하게 반드시 검증해야 한다.

---

# 20. Express Zod 검증 미들웨어

```ts
import type {
  NextFunction,
  Request,
  Response,
} from "express";
import type { ZodType } from "zod";

type RequestPart =
  | "body"
  | "query"
  | "params";

export function validateRequest(
  schema: ZodType,
  part: RequestPart,
) {
  return (
    req: Request,
    res: Response,
    next: NextFunction,
  ): void => {
    const result = schema.safeParse(req[part]);

    if (!result.success) {
      res.status(400).json({
        error: {
          code: "VALIDATION_ERROR",
          message:
            "요청 데이터가 올바르지 않습니다.",
          details: result.error.issues.map(
            (issue) => ({
              field: issue.path.join("."),
              reason: issue.message,
            }),
          ),
        },
      });

      return;
    }

    req[part] = result.data;
    next();
  };
}
```

사용 예시:

```ts
router.post(
  "/projects",
  authenticate,
  validateRequest(
    createProjectSchema,
    "body",
  ),
  projectController.create,
);
```

```ts
router.get(
  "/projects",
  authenticate,
  validateRequest(
    projectQuerySchema,
    "query",
  ),
  projectController.findAll,
);
```

```ts
router.patch(
  "/projects/:projectId/tasks/:taskId",
  authenticate,
  validateRequest(
    projectTaskParamSchema,
    "params",
  ),
  validateRequest(
    updateTaskSchema,
    "body",
  ),
  taskController.update,
);
```

---

# 21. Controller 입력 타입

Express의 기본 `Request` 타입을 그대로 사용하면 DTO 타입 정보가 약해질 수 있다.

다음과 같은 타입을 사용할 수 있다.

```ts
import type {
  Request,
} from "express";

export type TypedRequest<
  TParams = Record<string, string>,
  TBody = unknown,
  TQuery = Record<string, string>,
> = Request<
  TParams,
  unknown,
  TBody,
  TQuery
>;
```

사용 예시:

```ts
type CreateProjectRequest =
  TypedRequest<
    Record<string, never>,
    CreateProjectDto
  >;
```

```ts
export async function create(
  req: CreateProjectRequest,
  res: Response,
): Promise<void> {
  const userId = req.user.id;

  const project =
    await projectService.create(
      userId,
      req.body,
    );

  res.status(201).json({
    data: project,
  });
}
```

---

# 22. Service 계층에서 수행할 검증

Zod는 단일 요청의 형식과 값 범위를 검증한다. 데이터베이스 상태가 필요한 검증은 Service 계층에서 수행해야 한다.

## 22.1 Project

```text
Category가 존재하는가
Category가 현재 사용자에게 허용되는가
Project가 현재 사용자 소유인가
Project가 보관된 상태인가
상태 전이가 허용되는가
```

## 22.2 Milestone

```text
Milestone이 Project에 속하는가
Project가 사용자 소유인가
Milestone의 dueDate가 Project 일정 정책에 맞는가
연결된 Task가 있는 상태에서 삭제 가능한가
```

## 22.3 Task

```text
Task가 Project에 속하는가
Milestone이 같은 Project에 속하는가
부모 Task가 같은 Project에 속하는가
부모 Task가 최상위 Task인가
자기 자신을 부모로 지정하지 않았는가
자신의 하위 Task를 부모로 지정하지 않았는가
최대 깊이 1단계를 초과하지 않는가
상태 전이가 허용되는가
```

## 22.4 Tag

```text
Tag가 현재 사용자 소유인가
Tag 이름이 사용자 범위에서 중복되는가
연결 대상 Project 또는 Task가 사용자 소유인가
이미 연결된 Tag인가
```

## 22.5 TimeRecord

```text
Task가 같은 Project에 속하는가
종료 시간이 시작 시간보다 늦은가
동시에 활성화된 타이머가 존재하는가
사용자가 해당 기록을 수정할 권한이 있는가
```

## 22.6 Note와 File

```text
Project가 사용자 소유인가
Task가 Project에 속하는가
파일 타입과 크기가 허용되는가
스토리지 파일과 DB 레코드가 일치하는가
```

---

# 23. null과 undefined 처리 규칙

DTO 설계에서 `null`과 `undefined`는 다르게 처리한다.

```text
undefined
→ 요청에 필드가 없음
→ 기존 값을 유지

null
→ 관계 또는 값을 명시적으로 제거
```

예시:

```json
{
  "description": null,
  "dueDate": null
}
```

위 요청은 설명과 마감일을 제거한다.

다음 요청은:

```json
{
  "title": "변경된 제목"
}
```

설명과 마감일을 변경하지 않는다.

---

# 24. 서버 관리 필드

다음 필드는 클라이언트 요청으로 받지 않는다.

```text
id
userId
createdAt
updatedAt
completedAt
actualMinutes
durationMinutes
progress
isArchived
```

예외:

```text
isArchived
→ 전용 archive API에서 서버가 변경

completedAt
→ Task 상태 변경 시 서버가 변경

durationMinutes
→ startedAt과 endedAt에서 서버가 계산

progress
→ Task 상태에서 서버가 계산
```

클라이언트가 서버 관리 필드를 전달해도 Zod Object 기본 설정에 따라 제거하거나 strict 모드에서 오류 처리할 수 있다.

보안을 명확히 하기 위해 다음 방식을 권장한다.

```ts
export const createProjectSchema = z
  .object({
    // 허용 필드
  })
  .strict();
```

`strict()`를 사용하면 정의되지 않은 필드를 보낸 요청은 오류가 된다.

---

# 25. 프로젝트 진행률 응답 규칙

프로젝트 진행률은 정수 백분율로 반환한다.

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

진행률 범위:

```text
0~100
```

Task 계산 규칙:

```text
CANCELLED Task 제외
하위 Task가 없는 Task는 직접 계산
하위 Task가 있는 부모 Task는 제외
해당 부모의 하위 Task만 계산
```

Milestone도 같은 방식으로 연결된 계산 대상 Task를 기준으로 진행률을 반환한다.

---

# 26. DTO 구현 우선순위

## 1차 구현

```text
공통 스키마
Auth DTO
User DTO
Category DTO
Project DTO
Task DTO
```

이 단계까지 구현하면 기본 회원가입, 로그인, 프로젝트 생성, 작업 관리가 가능하다.

## 2차 구현

```text
Milestone DTO
Tag DTO
Project–Tag DTO
Task–Tag DTO
```

## 3차 구현

```text
TimeRecord DTO
Note DTO
ProjectFile DTO
NotificationSetting DTO
Dashboard Query DTO
```

---

# 27. 이번 단계 확정 사항

1. 요청 데이터 검증은 Zod로 수행한다.
2. UUID, 날짜, 날짜·시간, 페이지네이션 검증은 공통 스키마로 분리한다.
3. DTO는 생성, 수정, 상태 변경, 목록 조회 용도로 분리한다.
4. 상태 변경은 일반 수정 DTO와 분리한다.
5. Task 이동과 순서 변경도 일반 수정 DTO와 분리한다.
6. `null`은 값 제거, `undefined`는 기존 값 유지로 해석한다.
7. 사용자 ID와 서버 관리 필드는 요청 본문으로 받지 않는다.
8. 관계와 소유권 검증은 Service 계층에서 수행한다.
9. 날짜 범위는 Zod와 Service 계층에서 이중 검증한다.
10. Task 최대 깊이 제한은 Service 계층에서 검증한다.
11. API 응답에는 DB 내부 필드와 스토리지 경로를 직접 노출하지 않는다.
12. 프로젝트 진행률과 시간 합계는 계산된 응답 필드로 제공한다.
13. 정의되지 않은 요청 필드는 `strict()`를 통해 거부하는 방식을 권장한다.
14. DTO 구조는 이후 OpenAPI 스키마와 백엔드 코드에 재사용한다.
