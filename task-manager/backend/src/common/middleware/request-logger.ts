import { performance } from "node:perf_hooks";
import type { RequestHandler } from "express";
import { logger } from "../../lib/logger.js";

export const requestLogger: RequestHandler = (request, response, next) => {
  const startedAt = performance.now();

  response.on("finish", () => {
    logger.info("HTTP request completed.", {
      requestId: request.requestId,
      method: request.method,
      path: request.originalUrl,
      statusCode: response.statusCode,
      durationMs: Number((performance.now() - startedAt).toFixed(2)),
    });
  });

  next();
};
