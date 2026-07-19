# Task Manager REST API 명세서 v1.0

## 1. 설계 기준

### 1.1 핵심 리소스 구조

```text
User
 ├─ Category
 ├─ Project
 │   ├─ Milestone
 │   ├─ Task
 │   │   ├─ Subtask
 │   │   ├─ TimeRecord
 │   │   ├─ Note
 │   │   └─ Tag
 │   ├─ Note
 │   ├─ File
 │   └─ Tag
 └─ NotificationSetting
```

### 1.2 최종 테이블

```text
users
categories
projects
milestones
tasks
tags
time_records
notes
project_files
notification_settings
project_tags
task_tags
```

### 1.3 기본 원칙

1. 모든 API 경로는 `/api/v1`로 시작한다.
2. 주요 리소스 ID는 UUID를 사용한다.
3. 사용자는 자신이 소유한 데이터에만 접근할 수 있다.
4. 프로젝트 진행률은 DB에 저장하지 않고 Task 상태를 기준으로 계산한다.
5. 프로젝트 삭제보다 보관을 우선한다.
6. 프로젝트 보관 상태는 `isArchived`로 표현한다.
7. Task는 최대 한 단계의 하위 Task만 허용한다.
8. Milestone과 Task를 연결할 때 두 리소스는 같은 Project에 속해야 한다.
9. 프로젝트와 Task의 Tag 관계는 중간 테이블로 관리한다.
10. 프로젝트, Task, 시간 기록, 노트, 파일 변경 시 소유권을 항상 검증한다.

---

# 2. 공통 규칙

## 2.1 Base URL

```http
/api/v1
```

## 2.2 요청 헤더

```http
Content-Type: application/json
Authorization: Bearer {accessToken}
```

## 2.3 날짜와 시간

날짜:

```text
YYYY-MM-DD
```

날짜와 시간:

```text
ISO 8601
```

예시:

```json
{
  "startDate": "2026-07-20",
  "createdAt": "2026-07-18T16:30:00+09:00"
}
```

## 2.4 성공 응답

단일 리소스:

```json
{
  "data": {
    "id": "resource-uuid"
  }
}
```

목록:

```json
{
  "data": [],
  "pagination": {
    "page": 1,
    "size": 20,
    "totalElements": 42,
    "totalPages": 3
  }
}
```

## 2.5 오류 응답

```json
{
  "error": {
    "code": "PROJECT_NOT_FOUND",
    "message": "프로젝트를 찾을 수 없습니다.",
    "details": null
  }
}
```

유효성 검사 오류:

```json
{
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "요청 데이터가 올바르지 않습니다.",
    "details": [
      {
        "field": "title",
        "reason": "제목은 필수입니다."
      }
    ]
  }
}
```

## 2.6 HTTP 상태 코드

| 코드                          | 의미             |
| --------------------------- | -------------- |
| `200 OK`                    | 조회 또는 수정 성공    |
| `201 Created`               | 생성 성공          |
| `204 No Content`            | 삭제 또는 상태 변경 성공 |
| `400 Bad Request`           | 요청 형식 오류       |
| `401 Unauthorized`          | 인증 실패          |
| `403 Forbidden`             | 권한 없음          |
| `404 Not Found`             | 리소스를 찾을 수 없음   |
| `409 Conflict`              | 현재 상태 또는 관계 충돌 |
| `422 Unprocessable Entity`  | 비즈니스 규칙 위반     |
| `500 Internal Server Error` | 서버 내부 오류       |

---

# 3. 인증 API

## 3.1 회원가입

```http
POST /api/v1/auth/signup
```

요청:

```json
{
  "email": "user@example.com",
  "password": "password123!",
  "displayName": "충민"
}
```

응답:

```http
201 Created
```

```json
{
  "data": {
    "user": {
      "id": "user-uuid",
      "email": "user@example.com",
      "displayName": "충민"
    },
    "accessToken": "access-token",
    "refreshToken": "refresh-token"
  }
}
```

## 3.2 로그인

```http
POST /api/v1/auth/login
```

요청:

```json
{
  "email": "user@example.com",
  "password": "password123!"
}
```

## 3.3 토큰 재발급

```http
POST /api/v1/auth/refresh
```

```json
{
  "refreshToken": "refresh-token"
}
```

## 3.4 로그아웃

```http
POST /api/v1/auth/logout
```

응답:

```http
204 No Content
```

---

# 4. 사용자 API

## 4.1 내 정보 조회

```http
GET /api/v1/users/me
```

응답:

```json
{
  "data": {
    "id": "user-uuid",
    "email": "user@example.com",
    "displayName": "충민",
    "createdAt": "2026-07-18T16:30:00+09:00",
    "updatedAt": "2026-07-18T16:30:00+09:00"
  }
}
```

## 4.2 내 정보 수정

```http
PATCH /api/v1/users/me
```

```json
{
  "displayName": "새 이름"
}
```

## 4.3 비밀번호 변경

```http
PATCH /api/v1/users/me/password
```

```json
{
  "currentPassword": "old-password",
  "newPassword": "new-password"
}
```

## 4.4 회원 탈퇴

```http
DELETE /api/v1/users/me
```

초기 정책에서는 즉시 물리 삭제하지 않고 계정을 비활성화하거나 탈퇴 시각을 기록한다.

---

# 5. Category API

Category는 공부, 운동, 과제, 포트폴리오와 같은 프로젝트 분류를 나타낸다.

## 5.1 Category 목록 조회

```http
GET /api/v1/categories
```

쿼리 파라미터:

| 이름                | 설명                 |
| ----------------- | ------------------ |
| `includeInactive` | 비활성 Category 포함 여부 |
| `sort`            | 정렬 기준              |

응답:

```json
{
  "data": [
    {
      "id": "category-study-uuid",
      "name": "공부",
      "slug": "study",
      "description": "학습 프로젝트",
      "icon": "book",
      "color": "#7C3AED",
      "isActive": true,
      "position": 0
    }
  ]
}
```

## 5.2 Category 생성

```http
POST /api/v1/categories
```

사용자별 Category 추가를 허용하는 경우에만 제공한다.

```json
{
  "name": "독서",
  "slug": "reading",
  "description": "책 읽기 프로젝트",
  "icon": "book-open",
  "color": "#2563EB"
}
```

## 5.3 Category 상세 조회

```http
GET /api/v1/categories/{categoryId}
```

## 5.4 Category 수정

```http
PATCH /api/v1/categories/{categoryId}
```

```json
{
  "name": "개인 학습",
  "color": "#4F46E5",
  "isActive": true
}
```

## 5.5 Category 순서 변경

```http
PATCH /api/v1/categories/order
```

```json
{
  "categoryIds": [
    "category-uuid-2",
    "category-uuid-1",
    "category-uuid-3"
  ]
}
```

## 5.6 Category 삭제

```http
DELETE /api/v1/categories/{categoryId}
```

해당 Category를 참조하는 Project가 존재하면 삭제하지 않는다.

```http
409 Conflict
```

```json
{
  "error": {
    "code": "CATEGORY_IN_USE",
    "message": "프로젝트에서 사용 중인 카테고리는 삭제할 수 없습니다.",
    "details": {
      "projectCount": 3
    }
  }
}
```

---

# 6. Project API

## 6.1 프로젝트 생성

```http
POST /api/v1/projects
```

요청:

```json
{
  "categoryId": "category-study-uuid",
  "title": "자료구조 공부",
  "description": "복학 전 자료구조 핵심 개념 학습",
  "status": "PLANNED",
  "priority": "HIGH",
  "startDate": "2026-07-20",
  "dueDate": "2026-08-31"
}
```

서버에서 설정하는 필드:

```text
userId
isArchived
createdAt
updatedAt
```

응답:

```http
201 Created
```

```json
{
  "data": {
    "id": "project-uuid",
    "category": {
      "id": "category-study-uuid",
      "name": "공부",
      "slug": "study"
    },
    "title": "자료구조 공부",
    "description": "복학 전 자료구조 핵심 개념 학습",
    "status": "PLANNED",
    "priority": "HIGH",
    "startDate": "2026-07-20",
    "dueDate": "2026-08-31",
    "isArchived": false,
    "progress": 0,
    "createdAt": "2026-07-18T16:30:00+09:00",
    "updatedAt": "2026-07-18T16:30:00+09:00"
  }
}
```

## 6.2 프로젝트 목록 조회

```http
GET /api/v1/projects
```

쿼리 파라미터:

| 이름           | 설명          | 예시            |
| ------------ | ----------- | ------------- |
| `categoryId` | Category 필터 | UUID          |
| `status`     | 프로젝트 상태     | `IN_PROGRESS` |
| `priority`   | 우선순위        | `HIGH`        |
| `isArchived` | 보관 여부       | `false`       |
| `tagId`      | Tag 필터      | UUID          |
| `keyword`    | 제목·설명 검색    | `자료구조`        |
| `dueBefore`  | 특정 날짜 이전 마감 | `2026-08-31`  |
| `dueAfter`   | 특정 날짜 이후 마감 | `2026-07-20`  |
| `sort`       | 정렬          | `dueDate,asc` |
| `page`       | 페이지         | `1`           |
| `size`       | 페이지 크기      | `20`          |

예시:

```http
GET /api/v1/projects?categoryId={categoryId}&status=IN_PROGRESS&isArchived=false&sort=dueDate,asc
```

응답:

```json
{
  "data": [
    {
      "id": "project-uuid",
      "category": {
        "id": "category-study-uuid",
        "name": "공부"
      },
      "title": "자료구조 공부",
      "status": "IN_PROGRESS",
      "priority": "HIGH",
      "startDate": "2026-07-20",
      "dueDate": "2026-08-31",
      "isArchived": false,
      "progress": 45,
      "taskSummary": {
        "total": 20,
        "todo": 5,
        "inProgress": 4,
        "completed": 10,
        "blocked": 1
      },
      "milestoneSummary": {
        "total": 4,
        "completed": 1
      },
      "tags": [
        {
          "id": "tag-uuid",
          "name": "C++"
        }
      ]
    }
  ],
  "pagination": {
    "page": 1,
    "size": 20,
    "totalElements": 1,
    "totalPages": 1
  }
}
```

## 6.3 프로젝트 상세 조회

```http
GET /api/v1/projects/{projectId}
```

응답:

```json
{
  "data": {
    "id": "project-uuid",
    "category": {
      "id": "category-study-uuid",
      "name": "공부",
      "slug": "study"
    },
    "title": "자료구조 공부",
    "description": "복학 전 자료구조 핵심 개념 학습",
    "status": "IN_PROGRESS",
    "priority": "HIGH",
    "startDate": "2026-07-20",
    "dueDate": "2026-08-31",
    "isArchived": false,
    "progress": 45,
    "taskSummary": {
      "total": 20,
      "completed": 9,
      "inProgress": 5,
      "todo": 5,
      "blocked": 1
    },
    "timeSummary": {
      "totalMinutes": 870,
      "thisWeekMinutes": 240
    },
    "milestoneSummary": {
      "total": 4,
      "completed": 1
    },
    "tags": [
      {
        "id": "tag-uuid",
        "name": "C++",
        "color": "#2563EB"
      }
    ],
    "createdAt": "2026-07-18T16:30:00+09:00",
    "updatedAt": "2026-07-22T10:00:00+09:00"
  }
}
```

## 6.4 프로젝트 수정

```http
PATCH /api/v1/projects/{projectId}
```

```json
{
  "categoryId": "new-category-uuid",
  "title": "자료구조 및 알고리즘",
  "description": "자료구조 학습과 문제 풀이",
  "priority": "MEDIUM",
  "startDate": "2026-07-20",
  "dueDate": "2026-09-07"
}
```

변경할 필드만 전달한다.

## 6.5 프로젝트 상태 변경

```http
PATCH /api/v1/projects/{projectId}/status
```

```json
{
  "status": "IN_PROGRESS"
}
```

프로젝트 상태:

```text
PLANNED
IN_PROGRESS
PAUSED
COMPLETED
CANCELLED
```

상태 전이는 Service 계층에서 검증한다.

## 6.6 프로젝트 보관

```http
POST /api/v1/projects/{projectId}/archive
```

처리:

```text
isArchived = true
```

응답:

```http
204 No Content
```

## 6.7 프로젝트 복구

```http
DELETE /api/v1/projects/{projectId}/archive
```

처리:

```text
isArchived = false
```

## 6.8 프로젝트 삭제

```http
DELETE /api/v1/projects/{projectId}
```

기본 정책은 보관을 우선한다. 영구 삭제를 제공할 경우 프로젝트의 Milestone, Task, TimeRecord, Note, File, Tag 연결 처리 정책을 명확히 적용해야 한다.

---

# 7. Milestone API

## 7.1 프로젝트 Milestone 목록 조회

```http
GET /api/v1/projects/{projectId}/milestones
```

쿼리 파라미터:

```text
status
sort
```

응답:

```json
{
  "data": [
    {
      "id": "milestone-uuid",
      "projectId": "project-uuid",
      "title": "자료구조 기초 완료",
      "description": "배열부터 트리까지 기초 학습 완료",
      "status": "IN_PROGRESS",
      "dueDate": "2026-08-10",
      "position": 0,
      "progress": 60,
      "taskSummary": {
        "total": 10,
        "completed": 6
      }
    }
  ]
}
```

## 7.2 Milestone 생성

```http
POST /api/v1/projects/{projectId}/milestones
```

```json
{
  "title": "자료구조 기초 완료",
  "description": "배열부터 트리까지 기초 학습 완료",
  "dueDate": "2026-08-10",
  "position": 0
}
```

## 7.3 Milestone 상세 조회

```http
GET /api/v1/projects/{projectId}/milestones/{milestoneId}
```

## 7.4 Milestone 수정

```http
PATCH /api/v1/projects/{projectId}/milestones/{milestoneId}
```

```json
{
  "title": "선형 자료구조 완료",
  "description": "배열, 연결 리스트, 스택, 큐",
  "dueDate": "2026-08-05"
}
```

## 7.5 Milestone 상태 변경

```http
PATCH /api/v1/projects/{projectId}/milestones/{milestoneId}/status
```

```json
{
  "status": "COMPLETED"
}
```

Milestone 상태:

```text
PLANNED
IN_PROGRESS
COMPLETED
```

## 7.6 Milestone 순서 변경

```http
PATCH /api/v1/projects/{projectId}/milestones/order
```

```json
{
  "milestoneIds": [
    "milestone-uuid-2",
    "milestone-uuid-1",
    "milestone-uuid-3"
  ]
}
```

## 7.7 Milestone 삭제

```http
DELETE /api/v1/projects/{projectId}/milestones/{milestoneId}
```

Milestone을 참조하는 Task가 존재할 경우 다음 중 하나의 정책을 적용한다.

```text
Task의 milestoneId를 null로 변경
또는
Milestone 삭제를 거부
```

초기 API에서는 안전하게 삭제를 거부한다.

```http
409 Conflict
```

```json
{
  "error": {
    "code": "MILESTONE_HAS_TASKS",
    "message": "연결된 작업이 있는 마일스톤은 삭제할 수 없습니다."
  }
}
```

---

# 8. Task API

## 8.1 프로젝트 Task 목록 조회

```http
GET /api/v1/projects/{projectId}/tasks
```

쿼리 파라미터:

| 이름             | 설명                |
| -------------- | ----------------- |
| `milestoneId`  | Milestone 필터      |
| `parentTaskId` | 특정 상위 Task의 하위 작업 |
| `rootOnly`     | 최상위 Task만 조회      |
| `status`       | Task 상태           |
| `priority`     | 우선순위              |
| `tagId`        | Tag 필터            |
| `assignee`     | 향후 협업 확장용         |
| `dueBefore`    | 특정 날짜 이전 마감       |
| `keyword`      | 제목·설명 검색          |
| `sort`         | 정렬                |
| `page`         | 페이지               |
| `size`         | 페이지 크기            |

응답:

```json
{
  "data": [
    {
      "id": "task-uuid",
      "projectId": "project-uuid",
      "milestoneId": "milestone-uuid",
      "parentTaskId": null,
      "title": "연결 리스트 학습",
      "description": "단일 및 이중 연결 리스트 학습",
      "status": "IN_PROGRESS",
      "priority": "HIGH",
      "startDate": "2026-07-22",
      "dueDate": "2026-07-25",
      "position": 0,
      "completedAt": null,
      "subtaskSummary": {
        "total": 3,
        "completed": 1
      },
      "tags": [
        {
          "id": "tag-uuid",
          "name": "C++"
        }
      ]
    }
  ],
  "pagination": {
    "page": 1,
    "size": 20,
    "totalElements": 20,
    "totalPages": 1
  }
}
```

## 8.2 Task 생성

```http
POST /api/v1/projects/{projectId}/tasks
```

최상위 Task:

```json
{
  "milestoneId": "milestone-uuid",
  "title": "연결 리스트 학습",
  "description": "단일 및 이중 연결 리스트 학습",
  "status": "TODO",
  "priority": "HIGH",
  "startDate": "2026-07-22",
  "dueDate": "2026-07-25",
  "estimatedMinutes": 180,
  "position": 0
}
```

하위 Task:

```json
{
  "parentTaskId": "parent-task-uuid",
  "milestoneId": "milestone-uuid",
  "title": "단일 연결 리스트 구현",
  "status": "TODO",
  "priority": "MEDIUM",
  "position": 0
}
```

검증 규칙:

```text
1. parentTaskId가 있으면 부모 Task가 같은 Project에 속해야 한다.
2. 부모 Task가 이미 다른 Task의 하위 Task이면 생성할 수 없다.
3. 따라서 Task 깊이는 최대 1단계다.
4. milestoneId가 있으면 Milestone이 같은 Project에 속해야 한다.
5. Task의 dueDate는 Project의 허용 기간을 벗어나지 않도록 검증할 수 있다.
```

깊이 제한 위반:

```http
422 Unprocessable Entity
```

```json
{
  "error": {
    "code": "TASK_DEPTH_LIMIT_EXCEEDED",
    "message": "하위 작업은 한 단계까지만 생성할 수 있습니다."
  }
}
```

## 8.3 Task 상세 조회

```http
GET /api/v1/projects/{projectId}/tasks/{taskId}
```

응답:

```json
{
  "data": {
    "id": "task-uuid",
    "projectId": "project-uuid",
    "milestone": {
      "id": "milestone-uuid",
      "title": "자료구조 기초 완료"
    },
    "parentTaskId": null,
    "title": "연결 리스트 학습",
    "description": "단일 및 이중 연결 리스트 학습",
    "status": "IN_PROGRESS",
    "priority": "HIGH",
    "startDate": "2026-07-22",
    "dueDate": "2026-07-25",
    "estimatedMinutes": 180,
    "actualMinutes": 95,
    "position": 0,
    "completedAt": null,
    "subtasks": [
      {
        "id": "subtask-uuid",
        "title": "단일 연결 리스트 구현",
        "status": "COMPLETED",
        "position": 0
      }
    ],
    "tags": [],
    "createdAt": "2026-07-22T09:00:00+09:00",
    "updatedAt": "2026-07-22T12:00:00+09:00"
  }
}
```

## 8.4 Task 수정

```http
PATCH /api/v1/projects/{projectId}/tasks/{taskId}
```

```json
{
  "milestoneId": "new-milestone-uuid",
  "title": "연결 리스트 구현 및 문제 풀이",
  "description": "직접 구현 후 문제 3개 풀이",
  "priority": "MEDIUM",
  "dueDate": "2026-07-27",
  "estimatedMinutes": 240
}
```

Milestone 변경 시 Task와 새 Milestone이 같은 Project에 속하는지 확인한다.

## 8.5 Task 상태 변경

```http
PATCH /api/v1/projects/{projectId}/tasks/{taskId}/status
```

```json
{
  "status": "COMPLETED"
}
```

Task 상태:

```text
TODO
IN_PROGRESS
BLOCKED
COMPLETED
CANCELLED
```

완료 처리:

```text
status = COMPLETED
completedAt = 현재 시각
```

완료 해제:

```text
status = TODO 또는 IN_PROGRESS
completedAt = null
```

## 8.6 Task 완료 토글

체크박스 형태의 단순 완료 변경을 위한 편의 API다.

```http
PATCH /api/v1/projects/{projectId}/tasks/{taskId}/completion
```

```json
{
  "isCompleted": true
}
```

## 8.7 Task 순서 변경

```http
PATCH /api/v1/projects/{projectId}/tasks/order
```

```json
{
  "parentTaskId": null,
  "taskIds": [
    "task-uuid-3",
    "task-uuid-1",
    "task-uuid-2"
  ]
}
```

하위 Task 순서 변경:

```json
{
  "parentTaskId": "parent-task-uuid",
  "taskIds": [
    "subtask-uuid-2",
    "subtask-uuid-1"
  ]
}
```

검증:

```text
1. 모든 Task가 같은 Project에 속해야 한다.
2. 모든 Task의 parentTaskId가 요청의 parentTaskId와 같아야 한다.
3. ID 중복이 없어야 한다.
4. position은 0부터 다시 부여한다.
```

## 8.8 Task 이동

다른 Milestone으로 이동:

```http
PATCH /api/v1/projects/{projectId}/tasks/{taskId}/move
```

```json
{
  "milestoneId": "target-milestone-uuid",
  "parentTaskId": null,
  "position": 2
}
```

Task를 하위 Task로 바꿀 때도 최대 깊이 제한을 검증한다.

## 8.9 Task 삭제

```http
DELETE /api/v1/projects/{projectId}/tasks/{taskId}
```

하위 Task가 있는 상위 Task 삭제 시 기본 정책:

```text
삭제 거부
```

```http
409 Conflict
```

```json
{
  "error": {
    "code": "TASK_HAS_SUBTASKS",
    "message": "하위 작업이 있는 작업은 삭제할 수 없습니다."
  }
}
```

---

# 9. Tag API

## 9.1 Tag 목록 조회

```http
GET /api/v1/tags
```

쿼리:

```text
keyword
sort
```

응답:

```json
{
  "data": [
    {
      "id": "tag-uuid",
      "name": "C++",
      "color": "#2563EB",
      "projectCount": 2,
      "taskCount": 8
    }
  ]
}
```

## 9.2 Tag 생성

```http
POST /api/v1/tags
```

```json
{
  "name": "C++",
  "color": "#2563EB"
}
```

사용자 범위 안에서 Tag 이름은 중복되지 않도록 처리한다.

## 9.3 Tag 수정

```http
PATCH /api/v1/tags/{tagId}
```

```json
{
  "name": "C++ 학습",
  "color": "#1D4ED8"
}
```

## 9.4 Tag 삭제

```http
DELETE /api/v1/tags/{tagId}
```

중간 테이블의 연결을 먼저 해제한 후 Tag를 삭제한다.

---

# 10. Project–Tag API

## 10.1 프로젝트 Tag 조회

```http
GET /api/v1/projects/{projectId}/tags
```

## 10.2 프로젝트에 Tag 연결

```http
POST /api/v1/projects/{projectId}/tags
```

```json
{
  "tagIds": [
    "tag-uuid-1",
    "tag-uuid-2"
  ]
}
```

이미 연결된 Tag는 중복 생성하지 않는다.

## 10.3 프로젝트 Tag 연결 해제

```http
DELETE /api/v1/projects/{projectId}/tags/{tagId}
```

---

# 11. Task–Tag API

## 11.1 Task Tag 조회

```http
GET /api/v1/projects/{projectId}/tasks/{taskId}/tags
```

## 11.2 Task에 Tag 연결

```http
POST /api/v1/projects/{projectId}/tasks/{taskId}/tags
```

```json
{
  "tagIds": [
    "tag-uuid-1",
    "tag-uuid-2"
  ]
}
```

## 11.3 Task Tag 연결 해제

```http
DELETE /api/v1/projects/{projectId}/tasks/{taskId}/tags/{tagId}
```

---

# 12. TimeRecord API

## 12.1 시간 기록 목록 조회

프로젝트 기준:

```http
GET /api/v1/projects/{projectId}/time-records
```

Task 기준:

```http
GET /api/v1/projects/{projectId}/tasks/{taskId}/time-records
```

쿼리 파라미터:

```text
startDate
endDate
page
size
```

응답:

```json
{
  "data": [
    {
      "id": "time-record-uuid",
      "projectId": "project-uuid",
      "taskId": "task-uuid",
      "startedAt": "2026-07-22T09:00:00+09:00",
      "endedAt": "2026-07-22T10:30:00+09:00",
      "durationMinutes": 90,
      "description": "연결 리스트 구현"
    }
  ],
  "pagination": {
    "page": 1,
    "size": 20,
    "totalElements": 1,
    "totalPages": 1
  }
}
```

## 12.2 시간 기록 생성

```http
POST /api/v1/projects/{projectId}/time-records
```

```json
{
  "taskId": "task-uuid",
  "startedAt": "2026-07-22T09:00:00+09:00",
  "endedAt": "2026-07-22T10:30:00+09:00",
  "description": "연결 리스트 구현"
}
```

검증:

```text
1. taskId가 있으면 Task가 같은 Project에 속해야 한다.
2. endedAt은 startedAt보다 늦어야 한다.
3. durationMinutes는 서버에서 계산한다.
```

## 12.3 타이머 시작

```http
POST /api/v1/projects/{projectId}/time-records/start
```

```json
{
  "taskId": "task-uuid",
  "description": "연결 리스트 학습"
}
```

서버는 `startedAt`을 현재 시각으로 기록하고 `endedAt=null`인 활성 기록을 생성한다.

## 12.4 타이머 종료

```http
POST /api/v1/time-records/{timeRecordId}/stop
```

```json
{
  "endedAt": "2026-07-22T10:30:00+09:00"
}
```

동시에 여러 활성 타이머를 허용하지 않는 정책을 적용할 수 있다.

## 12.5 시간 기록 수정

```http
PATCH /api/v1/time-records/{timeRecordId}
```

```json
{
  "startedAt": "2026-07-22T09:10:00+09:00",
  "endedAt": "2026-07-22T10:20:00+09:00",
  "description": "시간 수정"
}
```

## 12.6 시간 기록 삭제

```http
DELETE /api/v1/time-records/{timeRecordId}
```

---

# 13. Note API

Note는 Project 또는 Task 중 하나에 연결된다.

## 13.1 프로젝트 Note 목록 조회

```http
GET /api/v1/projects/{projectId}/notes
```

## 13.2 Task Note 목록 조회

```http
GET /api/v1/projects/{projectId}/tasks/{taskId}/notes
```

## 13.3 Note 생성

프로젝트 Note:

```http
POST /api/v1/projects/{projectId}/notes
```

```json
{
  "title": "프로젝트 회고",
  "content": "이번 주에는 연결 리스트 구현을 완료했다.",
  "isPinned": true
}
```

Task Note:

```http
POST /api/v1/projects/{projectId}/tasks/{taskId}/notes
```

```json
{
  "title": "구현 메모",
  "content": "head와 tail 포인터를 별도로 관리한다.",
  "isPinned": false
}
```

## 13.4 Note 상세 조회

```http
GET /api/v1/notes/{noteId}
```

## 13.5 Note 수정

```http
PATCH /api/v1/notes/{noteId}
```

```json
{
  "title": "수정된 메모",
  "content": "수정된 내용",
  "isPinned": false
}
```

## 13.6 Note 삭제

```http
DELETE /api/v1/notes/{noteId}
```

---

# 14. ProjectFile API

## 14.1 프로젝트 파일 목록 조회

```http
GET /api/v1/projects/{projectId}/files
```

응답:

```json
{
  "data": [
    {
      "id": "file-uuid",
      "projectId": "project-uuid",
      "originalName": "자료구조정리.pdf",
      "mimeType": "application/pdf",
      "sizeBytes": 1048576,
      "downloadUrl": "/api/v1/project-files/file-uuid/download",
      "createdAt": "2026-07-22T10:00:00+09:00"
    }
  ]
}
```

## 14.2 파일 업로드

```http
POST /api/v1/projects/{projectId}/files
Content-Type: multipart/form-data
```

Form Data:

```text
file: binary
description: 참고 자료
```

응답:

```http
201 Created
```

## 14.3 파일 정보 수정

```http
PATCH /api/v1/project-files/{fileId}
```

```json
{
  "description": "자료구조 강의 참고 파일"
}
```

## 14.4 파일 다운로드

```http
GET /api/v1/project-files/{fileId}/download
```

## 14.5 파일 삭제

```http
DELETE /api/v1/project-files/{fileId}
```

DB 레코드 삭제와 실제 스토리지 파일 삭제가 함께 처리되어야 한다.

---

# 15. NotificationSetting API

## 15.1 알림 설정 조회

```http
GET /api/v1/notification-settings
```

응답:

```json
{
  "data": {
    "emailEnabled": true,
    "pushEnabled": false,
    "dueDateReminderEnabled": true,
    "dueDateReminderMinutes": 1440,
    "dailySummaryEnabled": false,
    "dailySummaryTime": "20:00",
    "timezone": "Asia/Seoul"
  }
}
```

## 15.2 알림 설정 수정

```http
PATCH /api/v1/notification-settings
```

```json
{
  "emailEnabled": true,
  "pushEnabled": true,
  "dueDateReminderEnabled": true,
  "dueDateReminderMinutes": 1440,
  "dailySummaryEnabled": true,
  "dailySummaryTime": "20:00",
  "timezone": "Asia/Seoul"
}
```

사용자당 NotificationSetting은 하나만 존재하도록 한다.

---

# 16. Dashboard API

## 16.1 대시보드 요약 조회

```http
GET /api/v1/dashboard
```

쿼리:

```text
startDate
endDate
categoryId
```

응답:

```json
{
  "data": {
    "projectSummary": {
      "total": 12,
      "planned": 2,
      "inProgress": 7,
      "paused": 1,
      "completed": 2
    },
    "taskSummary": {
      "total": 86,
      "todo": 24,
      "inProgress": 15,
      "blocked": 3,
      "completed": 44
    },
    "averageProjectProgress": 58,
    "timeSummary": {
      "totalMinutes": 870,
      "thisWeekMinutes": 240
    },
    "categoryProgress": [
      {
        "categoryId": "category-study-uuid",
        "categoryName": "공부",
        "projectCount": 4,
        "averageProgress": 72
      }
    ],
    "weeklyCompletedTasks": [
      {
        "date": "2026-07-13",
        "count": 5
      }
    ],
    "upcomingDeadlines": [
      {
        "type": "TASK",
        "id": "task-uuid",
        "projectId": "project-uuid",
        "title": "연결 리스트 구현",
        "dueDate": "2026-07-25",
        "daysRemaining": 7
      }
    ],
    "recentActivities": []
  }
}
```

## 16.2 미완료 Task 조회

```http
GET /api/v1/dashboard/incomplete-tasks
```

쿼리:

```text
categoryId
projectId
milestoneId
priority
dueBefore
sort
page
size
```

## 16.3 시간 통계 조회

```http
GET /api/v1/dashboard/time-statistics
```

쿼리:

```text
startDate
endDate
groupBy
```

`groupBy`:

```text
day
week
month
category
project
```

---

# 17. 진행률 계산 규칙

## 17.1 Task 완료 판정

```text
status = COMPLETED
```

인 Task만 완료 Task로 계산한다.

`CANCELLED` Task는 기본적으로 진행률 계산에서 제외한다.

## 17.2 프로젝트 진행률

```text
완료된 계산 대상 Task 수
────────────────────────── × 100
전체 계산 대상 Task 수
```

계산 대상:

```text
프로젝트에 속한 모든 활성 Task
단, status = CANCELLED인 Task는 제외
```

Task가 하나도 없으면:

```text
0%
```

예시:

```text
전체 활성 Task: 10개
COMPLETED: 4개
CANCELLED: 2개

계산 대상 Task = 8개
프로젝트 진행률 = 4 / 8 × 100 = 50%
```

## 17.3 Milestone 진행률

```text
Milestone에 연결된 완료 Task 수
───────────────────────────── × 100
Milestone에 연결된 전체 활성 Task 수
```

## 17.4 하위 Task 처리

상위 Task와 하위 Task를 모두 독립된 Task로 집계하면 진행률이 이중으로 왜곡될 수 있다.

초기 설계에서는 다음 규칙을 사용한다.

```text
하위 Task가 없는 Task → 해당 Task 자체를 계산
하위 Task가 있는 Task → 하위 Task만 계산하고 상위 Task는 진행률 분모에서 제외
```

예시:

```text
상위 Task A
 ├─ 하위 Task A-1 완료
 ├─ 하위 Task A-2 완료
 └─ 하위 Task A-3 미완료

A 그룹 진행률 = 2 / 3 × 100
```

---

# 18. 권한 검증 규칙

모든 하위 리소스 API는 다음 순서로 검증한다.

```text
1. Access Token에서 현재 userId를 확인한다.
2. Project가 현재 사용자의 소유인지 확인한다.
3. Milestone이 URL의 Project에 속하는지 확인한다.
4. Task가 URL의 Project에 속하는지 확인한다.
5. parentTaskId가 있으면 부모 Task가 같은 Project에 속하는지 확인한다.
6. milestoneId가 있으면 Milestone이 같은 Project에 속하는지 확인한다.
7. TimeRecord, Note, File이 해당 사용자 소유 Project에 속하는지 확인한다.
8. Tag가 현재 사용자의 Tag인지 확인한다.
```

다른 사용자의 리소스는 존재하더라도 다음을 반환한다.

```http
404 Not Found
```

이는 리소스 존재 여부 노출을 방지하기 위한 정책이다.

---

# 19. 주요 오류 코드

| 코드                                  | 설명                           |
| ----------------------------------- | ---------------------------- |
| `VALIDATION_ERROR`                  | 입력값 검증 실패                    |
| `AUTHENTICATION_REQUIRED`           | 인증 필요                        |
| `INVALID_ACCESS_TOKEN`              | Access Token 오류              |
| `USER_NOT_FOUND`                    | 사용자 없음                       |
| `CATEGORY_NOT_FOUND`                | Category 없음                  |
| `CATEGORY_IN_USE`                   | 사용 중인 Category 삭제            |
| `PROJECT_NOT_FOUND`                 | Project 없음                   |
| `PROJECT_ARCHIVED`                  | 보관된 Project 수정               |
| `INVALID_PROJECT_STATUS_TRANSITION` | 잘못된 Project 상태 전이            |
| `MILESTONE_NOT_FOUND`               | Milestone 없음                 |
| `MILESTONE_PROJECT_MISMATCH`        | Milestone과 Project 불일치       |
| `MILESTONE_HAS_TASKS`               | Task가 있는 Milestone 삭제        |
| `TASK_NOT_FOUND`                    | Task 없음                      |
| `TASK_PROJECT_MISMATCH`             | Task와 Project 불일치            |
| `TASK_MILESTONE_MISMATCH`           | Task와 Milestone의 Project 불일치 |
| `TASK_PARENT_PROJECT_MISMATCH`      | 부모 Task와 Project 불일치         |
| `TASK_DEPTH_LIMIT_EXCEEDED`         | Task 깊이 제한 초과                |
| `TASK_HAS_SUBTASKS`                 | 하위 Task가 있는 Task 삭제          |
| `INVALID_TASK_ORDER`                | Task 순서 변경 오류                |
| `TAG_NOT_FOUND`                     | Tag 없음                       |
| `DUPLICATE_TAG_NAME`                | Tag 이름 중복                    |
| `TIME_RECORD_NOT_FOUND`             | 시간 기록 없음                     |
| `INVALID_TIME_RANGE`                | 시작·종료 시간 오류                  |
| `ACTIVE_TIMER_ALREADY_EXISTS`       | 활성 타이머 중복                    |
| `NOTE_NOT_FOUND`                    | Note 없음                      |
| `PROJECT_FILE_NOT_FOUND`            | File 없음                      |
| `FILE_TOO_LARGE`                    | 파일 크기 제한 초과                  |
| `UNSUPPORTED_FILE_TYPE`             | 허용하지 않는 파일 유형                |

---

# 20. 엔드포인트 요약

## Auth

```text
POST   /api/v1/auth/signup
POST   /api/v1/auth/login
POST   /api/v1/auth/refresh
POST   /api/v1/auth/logout
```

## User

```text
GET    /api/v1/users/me
PATCH  /api/v1/users/me
PATCH  /api/v1/users/me/password
DELETE /api/v1/users/me
```

## Category

```text
GET    /api/v1/categories
POST   /api/v1/categories
GET    /api/v1/categories/{categoryId}
PATCH  /api/v1/categories/{categoryId}
PATCH  /api/v1/categories/order
DELETE /api/v1/categories/{categoryId}
```

## Project

```text
POST   /api/v1/projects
GET    /api/v1/projects
GET    /api/v1/projects/{projectId}
PATCH  /api/v1/projects/{projectId}
PATCH  /api/v1/projects/{projectId}/status
POST   /api/v1/projects/{projectId}/archive
DELETE /api/v1/projects/{projectId}/archive
DELETE /api/v1/projects/{projectId}
```

## Milestone

```text
GET    /api/v1/projects/{projectId}/milestones
POST   /api/v1/projects/{projectId}/milestones
GET    /api/v1/projects/{projectId}/milestones/{milestoneId}
PATCH  /api/v1/projects/{projectId}/milestones/{milestoneId}
PATCH  /api/v1/projects/{projectId}/milestones/{milestoneId}/status
PATCH  /api/v1/projects/{projectId}/milestones/order
DELETE /api/v1/projects/{projectId}/milestones/{milestoneId}
```

## Task

```text
GET    /api/v1/projects/{projectId}/tasks
POST   /api/v1/projects/{projectId}/tasks
GET    /api/v1/projects/{projectId}/tasks/{taskId}
PATCH  /api/v1/projects/{projectId}/tasks/{taskId}
PATCH  /api/v1/projects/{projectId}/tasks/{taskId}/status
PATCH  /api/v1/projects/{projectId}/tasks/{taskId}/completion
PATCH  /api/v1/projects/{projectId}/tasks/order
PATCH  /api/v1/projects/{projectId}/tasks/{taskId}/move
DELETE /api/v1/projects/{projectId}/tasks/{taskId}
```

## Tag

```text
GET    /api/v1/tags
POST   /api/v1/tags
PATCH  /api/v1/tags/{tagId}
DELETE /api/v1/tags/{tagId}

GET    /api/v1/projects/{projectId}/tags
POST   /api/v1/projects/{projectId}/tags
DELETE /api/v1/projects/{projectId}/tags/{tagId}

GET    /api/v1/projects/{projectId}/tasks/{taskId}/tags
POST   /api/v1/projects/{projectId}/tasks/{taskId}/tags
DELETE /api/v1/projects/{projectId}/tasks/{taskId}/tags/{tagId}
```

## TimeRecord

```text
GET    /api/v1/projects/{projectId}/time-records
POST   /api/v1/projects/{projectId}/time-records
GET    /api/v1/projects/{projectId}/tasks/{taskId}/time-records
POST   /api/v1/projects/{projectId}/time-records/start
POST   /api/v1/time-records/{timeRecordId}/stop
PATCH  /api/v1/time-records/{timeRecordId}
DELETE /api/v1/time-records/{timeRecordId}
```

## Note

```text
GET    /api/v1/projects/{projectId}/notes
POST   /api/v1/projects/{projectId}/notes
GET    /api/v1/projects/{projectId}/tasks/{taskId}/notes
POST   /api/v1/projects/{projectId}/tasks/{taskId}/notes
GET    /api/v1/notes/{noteId}
PATCH  /api/v1/notes/{noteId}
DELETE /api/v1/notes/{noteId}
```

## ProjectFile

```text
GET    /api/v1/projects/{projectId}/files
POST   /api/v1/projects/{projectId}/files
PATCH  /api/v1/project-files/{fileId}
GET    /api/v1/project-files/{fileId}/download
DELETE /api/v1/project-files/{fileId}
```

## NotificationSetting

```text
GET    /api/v1/notification-settings
PATCH  /api/v1/notification-settings
```

## Dashboard

```text
GET    /api/v1/dashboard
GET    /api/v1/dashboard/incomplete-tasks
GET    /api/v1/dashboard/time-statistics
```

---

# 21. 이번 단계에서 확정할 사항

1. API 리소스명은 실제 DB 테이블과 동일하게 Category, Project, Milestone, Task 중심으로 구성한다.
2. Step, ChecklistItem, ProjectMode API는 사용하지 않는다.
3. 프로젝트 분류는 Category로 관리한다.
4. 실제 작업 단위는 Task로 관리한다.
5. Task는 자기참조 방식으로 최대 1단계 하위 Task를 지원한다.
6. Milestone은 Project의 중간 목표를 나타낸다.
7. Task와 Milestone은 반드시 같은 Project에 속해야 한다.
8. 프로젝트 진행률은 Task 완료 상태에서 계산한다.
9. Tag 관계는 `project_tags`, `task_tags`를 통해 관리한다.
10. 시간 기록은 `time_records`에서 관리한다.
11. 프로젝트와 Task 메모는 `notes`에서 관리한다.
12. 첨부 파일은 `project_files`에서 관리한다.
13. 사용자별 알림 설정은 `notification_settings`에서 관리한다.
14. 프로젝트 삭제보다 `isArchived=true` 보관을 우선한다.
15. 모든 하위 리소스 요청에서 Project 소유권과 FK 소속 관계를 검증한다.
