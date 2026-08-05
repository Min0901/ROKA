import cors from "cors";
import express from "express";
import helmet from "helmet";
import { AppError } from "./common/errors/app-error.js";
import { ErrorCode } from "./common/errors/error-code.js";
import { developmentAuth } from "./common/middleware/development-auth.js";
import { errorHandler } from "./common/middleware/error-handler.js";
import { notFoundHandler } from "./common/middleware/not-found.js";
import { requestId } from "./common/middleware/request-id.js";
import { requestLogger } from "./common/middleware/request-logger.js";
import { env } from "./config/env.js";
import { healthRouter } from "./modules/health/health.router.js";
import { apiRouter } from "./routes/api.router.js";

const allowedOrigins = env.CORS_ORIGIN.split(",")
  .map((origin) => origin.trim())
  .filter(Boolean);

export const app = express();

app.disable("x-powered-by");

if (env.TRUST_PROXY > 0) {
  app.set("trust proxy", env.TRUST_PROXY);
}

app.use(requestId);
app.use(requestLogger);
app.use(helmet());
app.use(
  cors({
    origin(origin, callback) {
      if (!origin || allowedOrigins.includes("*") || allowedOrigins.includes(origin)) {
        callback(null, true);
        return;
      }

      callback(
        new AppError({
          statusCode: 403,
          code: ErrorCode.FORBIDDEN,
          message: "The request origin is not allowed.",
        }),
      );
    },
    credentials: true,
  }),
);
app.use(express.json({ limit: env.JSON_BODY_LIMIT }));
app.use(express.urlencoded({ extended: false, limit: env.JSON_BODY_LIMIT }));

app.use("/health", healthRouter);
app.use("/api/v1", developmentAuth, apiRouter);

app.use(notFoundHandler);
app.use(errorHandler);
