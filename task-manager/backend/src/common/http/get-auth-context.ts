import type { Request } from "express";
import { AppError } from "../errors/app-error.js";
import { ErrorCode } from "../errors/error-code.js";

export function getAuthContext(request: Request): Express.AuthContext {
  if (!request.auth) {
    throw new AppError({
      statusCode: 401,
      code: ErrorCode.UNAUTHORIZED,
      message: "Authentication context is missing.",
    });
  }

  return request.auth;
}
