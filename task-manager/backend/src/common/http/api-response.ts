import type { Response } from "express";

interface SuccessEnvelope<T> {
  success: true;
  data: T;
  meta?: Record<string, unknown>;
}

export function sendSuccess<T>(
  response: Response,
  data: T,
  options: {
    statusCode?: number;
    meta?: Record<string, unknown>;
  } = {},
): Response<SuccessEnvelope<T>> {
  const payload: SuccessEnvelope<T> = {
    success: true,
    data,
  };

  if (options.meta !== undefined) {
    payload.meta = options.meta;
  }

  return response.status(options.statusCode ?? 200).json(payload);
}
