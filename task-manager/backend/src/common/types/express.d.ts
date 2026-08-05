export {};

declare global {
  namespace Express {
    interface AuthContext {
      userId: string;
    }

    interface ValidatedRequestData {
      body: unknown;
      query: unknown;
      params: unknown;
    }

    interface Request {
      requestId: string;
      auth?: AuthContext;
      validated?: ValidatedRequestData;
    }
  }
}
