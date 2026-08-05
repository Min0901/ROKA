import type { Server } from "node:http";
import { app } from "./app.js";
import { env } from "./config/env.js";
import { logger } from "./lib/logger.js";
import { prisma } from "./lib/prisma.js";

let server: Server;
let isShuttingDown = false;

async function shutdown(signal: string, exitCode = 0): Promise<void> {
  if (isShuttingDown) {
    return;
  }

  isShuttingDown = true;
  logger.info("Server shutdown started.", { signal });

  const forceExitTimer = setTimeout(() => {
    logger.error("Server shutdown timed out.");
    process.exit(1);
  }, 10_000);

  forceExitTimer.unref();

  const closeServer = new Promise<void>((resolve, reject) => {
    server.close((error) => {
      if (error) {
        reject(error);
        return;
      }

      resolve();
    });
  });

  try {
    await closeServer;
    await prisma.$disconnect();
    clearTimeout(forceExitTimer);
    logger.info("Server shutdown completed.");
    process.exit(exitCode);
  } catch (error) {
    logger.error("Server shutdown failed.", {
      error:
        error instanceof Error
          ? {
              name: error.name,
              message: error.message,
              stack: error.stack,
            }
          : error,
    });
    process.exit(1);
  }
}

server = app.listen(env.PORT, () => {
  logger.info("Task Manager API started.", {
    port: env.PORT,
    environment: env.NODE_ENV,
    apiBaseUrl: `http://localhost:${env.PORT}/api/v1`,
  });
});

process.on("SIGINT", () => {
  void shutdown("SIGINT");
});

process.on("SIGTERM", () => {
  void shutdown("SIGTERM");
});

process.on("unhandledRejection", (reason) => {
  logger.error("Unhandled promise rejection.", { reason });
  void shutdown("unhandledRejection", 1);
});

process.on("uncaughtException", (error) => {
  logger.error("Uncaught exception.", {
    error: {
      name: error.name,
      message: error.message,
      stack: error.stack,
    },
  });
  void shutdown("uncaughtException", 1);
});
