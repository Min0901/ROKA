import { Router } from "express";
import { sendSuccess } from "../../common/http/api-response.js";
import { prisma } from "../../lib/prisma.js";

export const healthRouter = Router();

healthRouter.get("/live", (_request, response) => {
  sendSuccess(response, {
    status: "ok",
    timestamp: new Date().toISOString(),
    uptimeSeconds: Math.floor(process.uptime()),
  });
});

healthRouter.get("/ready", async (_request, response) => {
  await prisma.$queryRaw`SELECT 1`;

  sendSuccess(response, {
    status: "ready",
    database: "reachable",
    timestamp: new Date().toISOString(),
  });
});
