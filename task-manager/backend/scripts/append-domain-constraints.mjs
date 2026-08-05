import { appendFile, readFile, readdir } from "node:fs/promises";
import path from "node:path";

const migrationsDir = path.resolve("prisma/migrations");
const constraintsPath = path.resolve("prisma/sql/domain_constraints.sql");
const marker = "-- Task Manager supplemental domain constraints";

const entries = await readdir(migrationsDir, { withFileTypes: true });
const migrationDirectories = entries
  .filter((entry) => entry.isDirectory())
  .map((entry) => entry.name)
  .sort();

const latestDirectory = migrationDirectories.at(-1);

if (!latestDirectory) {
  throw new Error("No Prisma migration directory was found. Run prisma:migrate:create first.");
}

const migrationPath = path.join(migrationsDir, latestDirectory, "migration.sql");
const migrationSql = await readFile(migrationPath, "utf8");

if (migrationSql.includes(marker)) {
  console.log(`Constraints are already appended: ${migrationPath}`);
  process.exit(0);
}

const constraintsSql = await readFile(constraintsPath, "utf8");
await appendFile(
  migrationPath,
  `\n\n${marker}\n${constraintsSql.trim()}\n`,
  "utf8",
);

console.log(`Appended domain constraints to ${migrationPath}`);
