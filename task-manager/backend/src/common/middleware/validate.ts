import type { RequestHandler } from "express";
import { z } from "zod";
import { AppError } from "../errors/app-error.js";
import { ErrorCode } from "../errors/error-code.js";

interface ValidationSchemas {
  body?: z.ZodType;
  query?: z.ZodType;
  params?: z.ZodType;
}

type ValidationTarget = keyof ValidationSchemas;

const targets: ValidationTarget[] = ["body", "query", "params"];

export function validate(schemas: ValidationSchemas): RequestHandler {
  return async (request, _response, next) => {
    const validated: Express.ValidatedRequestData = {
      body: request.body,
      query: request.query,
      params: request.params,
    };

    const issues: Array<{
      target: ValidationTarget;
      path: PropertyKey[];
      message: string;
      code: string;
    }> = [];

    for (const target of targets) {
      const schema = schemas[target];

      if (!schema) {
        continue;
      }

      const result = await schema.safeParseAsync(request[target]);

      if (!result.success) {
        issues.push(
          ...result.error.issues.map((issue) => ({
            target,
            path: issue.path,
            message: issue.message,
            code: issue.code,
          })),
        );
        continue;
      }

      validated[target] = result.data;
    }

    if (issues.length > 0) {
      throw new AppError({
        statusCode: 422,
        code: ErrorCode.VALIDATION_ERROR,
        message: "Request validation failed.",
        details: issues,
      });
    }

    request.validated = validated;
    next();
  };
}

export function getValidatedInput<
  TBody = unknown,
  TQuery = unknown,
  TParams = unknown,
>(request: Express.Request): {
  body: TBody;
  query: TQuery;
  params: TParams;
} {
  if (!request.validated) {
    throw new AppError({
      statusCode: 500,
      code: ErrorCode.INTERNAL_ERROR,
      message: "Validated request data is missing.",
    });
  }

  return request.validated as {
    body: TBody;
    query: TQuery;
    params: TParams;
  };
}
