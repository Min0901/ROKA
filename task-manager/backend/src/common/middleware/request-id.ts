import { randomUUID } from "node:crypto";
import type { RequestHandler } from "express";

const MAX_REQUEST_ID_LENGTH = 100;

export const requestId: RequestHandler = (request, response, next) => {
  const incomingRequestId = request.header("x-request-id");
  const resolvedRequestId =
    incomingRequestId && incomingRequestId.length <= MAX_REQUEST_ID_LENGTH
      ? incomingRequestId
      : randomUUID();

  request.requestId = resolvedRequestId;
  response.setHeader("x-request-id", resolvedRequestId);
  next();
};
