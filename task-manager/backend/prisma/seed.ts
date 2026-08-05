import "dotenv/config";
import { PrismaPg } from "@prisma/adapter-pg";
import { PrismaClient } from "../src/generated/prisma/client.js";

const databaseUrl = process.env.DATABASE_URL;
const developmentUserId =
  process.env.DEV_USER_ID ?? "00000000-0000-4000-8000-000000000001";

if (!databaseUrl) {
  throw new Error("DATABASE_URL is required to run the seed.");
}

const adapter = new PrismaPg({
  connectionString: databaseUrl,
});

const prisma = new PrismaClient({
  adapter,
});

const categories = [
  {
    name: "공부",
    slug: "study",
    description: "학습 계획과 진도를 관리합니다.",
    icon: "book-open",
    color: "#7C3AED",
    position: 0,
  },
  {
    name: "운동",
    slug: "workout",
    description: "운동 루틴과 수행 기록을 관리합니다.",
    icon: "dumbbell",
    color: "#16A34A",
    position: 1,
  },
  {
    name: "과제",
    slug: "assignment",
    description: "학교 과제와 마감 일정을 관리합니다.",
    icon: "file-text",
    color: "#EA580C",
    position: 2,
  },
  {
    name: "포트폴리오",
    slug: "portfolio",
    description: "포트폴리오 프로젝트와 산출물을 관리합니다.",
    icon: "folder-kanban",
    color: "#2563EB",
    position: 3,
  },
] as const;

async function main(): Promise<void> {
  await prisma.user.upsert({
    where: {
      id: developmentUserId,
    },
    update: {
      email: "developer@task-manager.local",
      displayName: "개발 사용자",
    },
    create: {
      id: developmentUserId,
      email: "developer@task-manager.local",
      passwordHash: "development-only-authentication-not-configured",
      displayName: "개발 사용자",
    },
  });

  for (const category of categories) {
    await prisma.category.upsert({
      where: {
        userId_slug: {
          userId: developmentUserId,
          slug: category.slug,
        },
      },
      update: {
        ...category,
        isActive: true,
      },
      create: {
        userId: developmentUserId,
        ...category,
      },
    });
  }

  await prisma.notificationSetting.upsert({
    where: {
      userId: developmentUserId,
    },
    update: {},
    create: {
      userId: developmentUserId,
      timezone: "Asia/Seoul",
    },
  });
}

main()
  .then(async () => {
    console.log("Database seed completed.");
    await prisma.$disconnect();
  })
  .catch(async (error) => {
    console.error(error);
    await prisma.$disconnect();
    process.exit(1);
  });
