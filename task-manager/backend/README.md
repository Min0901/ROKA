# Task Manager Backend Starter

최종 확정된 12개 도메인 테이블을 기준으로 만든 백엔드 초기 구성입니다.

- Node.js 24 LTS
- TypeScript + ESM
- Express 5
- Zod 4
- Prisma ORM 7
- PostgreSQL
- 개발용 단일 사용자 컨텍스트

## 포함된 공통 기반

- 환경변수 시작 시 검증
- Prisma 7 PostgreSQL driver adapter 연결
- 공통 성공·오류 응답 형식
- `AppError`와 Prisma 오류 매핑
- Zod 요청 검증 미들웨어
- 요청 ID와 JSON 요청 로그
- 개발용 사용자 주입 미들웨어
- `/health/live`, `/health/ready`
- Docker Compose PostgreSQL
- 개발 사용자와 기본 카테고리 4개 시드
- 추가 CHECK 제약조건과 교차 테이블 검증 트리거

## 디렉터리 구조

```text
src/
├─ app.ts
├─ server.ts
├─ config/
├─ lib/
├─ common/
│  ├─ errors/
│  ├─ http/
│  ├─ middleware/
│  ├─ schemas/
│  ├─ types/
│  └─ utils/
├─ modules/
│  └─ health/
└─ routes/
prisma/
├─ schema.prisma
├─ seed.ts
└─ sql/domain_constraints.sql
```

## 1. 설치

```bash
npm install
cp .env.example .env
```

Node.js는 24 LTS 사용을 권장합니다.

## 2. PostgreSQL 실행

```bash
npm run db:up
```

## 3. Prisma 초기 마이그레이션

Prisma Schema만으로 표현되지 않는 CHECK와 트리거를 최초 migration에 합칩니다.

```bash
npm run prisma:format
npm run prisma:validate
npm run prisma:migrate:create
npm run prisma:migrate:append-constraints
npm run prisma:migrate:dev
npm run prisma:generate
npm run prisma:seed
```

Prisma ORM 7에서는 migration 후 Client 생성과 seed가 자동 실행되지 않으므로 명시적으로 실행합니다.

## 4. 개발 서버 실행

```bash
npm run dev
```

확인 주소:

```text
GET http://localhost:4000/health/live
GET http://localhost:4000/health/ready
GET http://localhost:4000/api/v1
GET http://localhost:4000/api/v1/context
```

예상 개발 사용자 ID:

```text
00000000-0000-4000-8000-000000000001
```

다른 개발 사용자 ID를 시험하려면 요청 헤더를 사용합니다.

```text
x-dev-user-id: <UUID>
```

해당 UUID의 사용자가 DB에 실제로 존재하는지는 이후 인증 또는 사용자 조회 미들웨어에서 검증해야 합니다.

## 응답 형식

성공:

```json
{
  "success": true,
  "data": {}
}
```

실패:

```json
{
  "success": false,
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "Request validation failed.",
    "details": []
  },
  "requestId": "..."
}
```

## 중요한 경계

현재 인증은 로컬 개발 전용입니다. `NODE_ENV=production`으로 실행하면 환경변수 검증 단계에서 서버 시작을 차단합니다. 공개 배포 전에 비밀번호 해시, Access Token, Refresh Token 저장·회전·폐기 정책을 구현해야 합니다.

DB가 강제하는 규칙 외에 Service 계층에서 구현해야 할 규칙은 다음과 같습니다.

- Project, Milestone, Task 상태 전이
- 보관된 Project 수정 금지
- Milestone에 Task가 있을 때 명시적 삭제 거부
- 하위 Task가 있는 부모 Task의 명시적 삭제 거부
- 순서 변경 요청의 전체 배열 검증
- Project와 Milestone 진행률 계산
- 파일 MIME signature 검증과 저장소 보상 처리

## 다음 구현 단위

첫 기능 모듈은 `categories` 조회와 `projects` 생성·목록·상세입니다. 각 모듈은 다음 구조를 따릅니다.

```text
modules/projects/
├─ project.schema.ts
├─ project.repository.ts
├─ project.service.ts
├─ project.controller.ts
└─ project.router.ts
```

Controller는 HTTP 변환만, Service는 비즈니스 규칙, Repository는 Prisma 접근만 담당합니다.
