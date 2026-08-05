import type { ErrorRequestHandler } from "express";
import { z } from "zod";
import { env } from "../../config/env.js";
import { Prisma } from "../../generated/prisma/client.js";
import { logger } from "../../lib/logger.js";
import { AppError } from "../errors/app-error.js";
import { ErrorCode } from "../errors/error-code.js";

interface NormalizedError {
  statusCode: number;
  code: string;
  message: string;
  details?: unknown;
  operational: boolean;
}

function normalizeError(error: unknown): NormalizedError {
  if (error instanceof AppError) {
    return {
      statusCode: error.statusCode,
      code: error.code,
      message: error.message,
      details: error.details,
      operational: true,
    };
  }

  if (error instanceof z.ZodError) {
    return {
      statusCode: 422,
      code: ErrorCode.VALIDATION_ERROR,
      message: "Validation failed.",
      details: error.issues,
      operational: true,
    };
  }

  if (error instanceof Prisma.PrismaClientKnownRequestError) {
    if (error.code === "P2002") {
      return {
        statusCode: 409,
        code: ErrorCode.CONFLICT,
        message: "A record with the same unique value already exists.",
        details: error.meta,
        operational: true,
      };
    }

    if (error.code === "P2003") {
      return {
        statusCode: 409,
        code: ErrorCode.FOREIGN_KEY_CONFLICT,
        message: "The requested operation conflicts with a related record.",
        details: error.meta,
        operational: true,
      };
    }

    if (error.code === "P2025") {
      return {
        statusCode: 404,
        code: ErrorCode.NOT_FOUND,
        message: "The requested record was not found.",
        details: error.meta,
        operational: true,
      };
    }

    return {
      statusCode: 400,
      code: ErrorCode.DATABASE_ERROR,
      message: "The database rejected the request.",
      details: {
        prismaCode: error.code,
        meta: error.meta,
      },
      operational: true,
    };
  }

  if (error instanceof Prisma.PrismaClientInitializationError) {
    return {
      statusCode: 503,
      code: ErrorCode.SERVICE_UNAVAILABLE,
      message: "The database is unavailable.",
      operational: false,
    };
  }

  if (
    error instanceof SyntaxError &&
    typeof error === "object" &&
    error !== null &&
    "type" in error &&
    error.type === "entity.parse.failed"
  ) {
    return {
      statusCode: 400,
      code: ErrorCode.INVALID_JSON,
      message: "The request body contains invalid JSON.",
      operational: true,
    };
  }

  return {
    statusCode: 500,
    code: ErrorCode.INTERNAL_ERROR,
    message: "An unexpected server error occurred.",
    operational: false,
  };
}

export const errorHandler: ErrorRequestHandler = (error, request, response, next) => {
  if (response.headersSent) {
    next(error);
    return;
  }

  const normalized = normalizeError(error);

  logger.error("HTTP request failed.", {
    requestId: request.requestId,
    method: request.method,
    path: request.originalUrl,
    statusCode: normalized.statusCode,
    code: normalized.code,
    operational: normalized.operational,
    error:
      error instanceof Error
        ? {
            name: error.name,
            message: error.message,
            stack: error.stack,
          }
        : error,
  });

  const payload: Record<string, unknown> = {
    success: false,
    error: {
      code: normalized.code,
      message: normalized.message,
      ...(normalized.details === undefined ? {} : { details: normalized.details }),
    },
    requestId: request.requestId,
  };

  if (env.NODE_ENV !== "production" && !normalized.operational && error instanceof Error) {
    payload.debug = {
      name: error.name,
      message: error.message,
      stack: error.stack,
    };
  }

  response.status(normalized.statusCode).json(payload);
};
