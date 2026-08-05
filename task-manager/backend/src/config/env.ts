import "dotenv/config";
import { z } from "zod";

const postgresUrlSchema = z
  .string()
  .min(1, "DATABASE_URL is required.")
  .refine(
    (value) => value.startsWith("postgresql://") || value.startsWith("postgres://"),
    "DATABASE_URL must be a PostgreSQL connection URL.",
  );

const envSchema = z
  .object({
    NODE_ENV: z.enum(["development", "test", "production"]).default("development"),
    PORT: z.coerce.number().int().min(1).max(65_535).default(4000),
    DATABASE_URL: postgresUrlSchema,
    CORS_ORIGIN: z.string().min(1).default("http://localhost:5173"),
    DEV_USER_ID: z.uuid().default("00000000-0000-4000-8000-000000000001"),
    JSON_BODY_LIMIT: z.string().min(1).default("1mb"),
    TRUST_PROXY: z.coerce.number().int().min(0).default(0),
  })
  .superRefine((value, context) => {
    if (value.NODE_ENV === "production") {
      context.addIssue({
        code: "custom",
        path: ["NODE_ENV"],
        message:
          "This starter uses development-only authentication. Implement production authentication before setting NODE_ENV=production.",
      });
    }
  });

const parsed = envSchema.safeParse(process.env);

if (!parsed.success) {
  const issues = parsed.error.issues
    .map((issue) => `${issue.path.join(".") || "environment"}: ${issue.message}`)
    .join("\n");

  throw new Error(`Invalid environment variables:\n${issues}`);
}

export const env = parsed.data;
