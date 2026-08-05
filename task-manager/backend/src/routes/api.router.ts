import { Router } from "express";
import { sendSuccess } from "../common/http/api-response.js";
import { getAuthContext } from "../common/http/get-auth-context.js";

export const apiRouter = Router();

apiRouter.get("/", (_request, response) => {
  sendSuccess(response, {
    name: "Task Manager API",
    version: "v1",
  });
});

apiRouter.get("/context", (request, response) => {
  const auth = getAuthContext(request);

  sendSuccess(response, {
    userId: auth.userId,
    authentication: "development-only",
  });
});
