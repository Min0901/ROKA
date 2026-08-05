import type { RequestHandler } from "express";
import { z } from "zod";
import { env } from "../../config/env.js";
import { AppError } from "../errors/app-error.js";
import { ErrorCode } from "../errors/error-code.js";

const userIdSchema = z.uuid();

export const developmentAuth: RequestHandler = (request, _response, next) => {
  const candidateUserId = request.header("x-dev-user-id") ?? env.DEV_USER_ID;
  const parsed = userIdSchema.safeParse(candidateUserId);

  if (!parsed.success) {
    throw new AppError({
      statusCode: 400,
      code: ErrorCode.INVALID_DEV_USER,
      message: "x-dev-user-id must be a valid UUID.",
      details: parsed.error.issues,
    });
  }

  request.auth = {
    userId: parsed.data,
  };

  next();
};
