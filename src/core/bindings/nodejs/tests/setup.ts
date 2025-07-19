/**
 * @file setup.ts
 * @brief Jest setup file for CoreBase Node.js bindings tests
 */

// Extend Jest matchers if needed
import 'jest-extended';

// Global test configuration
jest.setTimeout(30000); // 30 seconds timeout for all tests

// Mock console methods to reduce noise during testing
const originalConsoleError = console.error;
const originalConsoleWarn = console.warn;
const originalConsoleLog = console.log;

// Store original methods for restoration
(global as any).originalConsole = {
  error: originalConsoleError,
  warn: originalConsoleWarn,
  log: originalConsoleLog,
};

// Mock console methods during tests
console.error = jest.fn();
console.warn = jest.fn();
console.log = jest.fn();

// Global test utilities
(global as any).testUtils = {
  /**
   * Wait for a specified amount of time
   * @param ms Milliseconds to wait
   */
  wait: (ms: number): Promise<void> => {
    return new Promise(resolve => setTimeout(resolve, ms));
  },

  /**
   * Create a mock function with specified return value
   * @param returnValue Value to return
   */
  createMockFunction: <T>(returnValue: T): jest.MockedFunction<() => T> => {
    return jest.fn().mockReturnValue(returnValue);
  },

  /**
   * Create a mock async function with specified return value
   * @param returnValue Value to return
   */
  createMockAsyncFunction: <T>(returnValue: T): jest.MockedFunction<() => Promise<T>> => {
    return jest.fn().mockResolvedValue(returnValue);
  },

  /**
   * Restore console methods
   */
  restoreConsole: (): void => {
    console.error = originalConsoleError;
    console.warn = originalConsoleWarn;
    console.log = originalConsoleLog;
  },

  /**
   * Mock console methods
   */
  mockConsole: (): void => {
    console.error = jest.fn();
    console.warn = jest.fn();
    console.log = jest.fn();
  },
};

// Global beforeEach for all tests
beforeEach(() => {
  // Clear all mocks before each test
  jest.clearAllMocks();
  
  // Reset console mocks
  (console.error as jest.MockedFunction<typeof console.error>).mockClear();
  (console.warn as jest.MockedFunction<typeof console.warn>).mockClear();
  (console.log as jest.MockedFunction<typeof console.log>).mockClear();
});

// Global afterEach for all tests
afterEach(() => {
  // Clean up any remaining timers
  jest.clearAllTimers();
  
  // Restore all mocks
  jest.restoreAllMocks();
});

// Handle unhandled promise rejections in tests
process.on('unhandledRejection', (reason, promise) => {
  console.error('Unhandled Rejection at:', promise, 'reason:', reason);
  // Don't exit the process in tests, just log the error
});

// Handle uncaught exceptions in tests
process.on('uncaughtException', (error) => {
  console.error('Uncaught Exception:', error);
  // Don't exit the process in tests, just log the error
});

// Export test utilities for use in test files
export const testUtils = (global as any).testUtils;