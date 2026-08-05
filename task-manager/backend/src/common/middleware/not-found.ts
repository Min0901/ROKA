import type { RequestHandler } from "express";
import { AppError } from "../errors/app-error.js";
import { ErrorCode } from "../errors/error-code.js";

export const notFoundHandler: RequestHandler = (request, _response, next) => {
  next(
    new AppError({
      statusCode: 404,
      code: ErrorCode.ROUTE_NOT_FOUND,
      message: `Route not found: ${request.method} ${request.originalUrl}`,
    }),
  );
};
