import '@testing-library/jest-dom';
import { configure } from '@testing-library/react';

// Configure testing library
configure({
  testIdAttribute: 'data-testid'
});

// Global test timeout
jest.setTimeout(30000);

afterEach(() => {
  // Clear all mocks
  jest.clearAllMocks();
  
  // Clear all timers
  jest.clearAllTimers();
});

// Global error handler for unhandled promise rejections
process.on('unhandledRejection', (reason) => {
  console.error('Unhandled Promise Rejection:', reason);
});

// Global error handler for uncaught exceptions
process.on('uncaughtException', (error) => {
  console.error('Uncaught Exception:', error);
});

// Mock IntersectionObserver
global.IntersectionObserver = class IntersectionObserver {
  constructor() {}
  observe() {
    return null;
  }
  disconnect() {
    return null;
  }
  unobserve() {
    return null;
  }
};

// Mock ResizeObserver
global.ResizeObserver = class ResizeObserver {
  constructor() {}
  observe() {
    return null;
  }
  disconnect() {
    return null;
  }
  unobserve() {
    return null;
  }
};

// Mock matchMedia
Object.defineProperty(window, 'matchMedia', {
  writable: true,
  value: jest.fn().mockImplementation(query => ({
    matches: false,
    media: query,
    onchange: null,
    addListener: jest.fn(), // deprecated
    removeListener: jest.fn(), // deprecated
    addEventListener: jest.fn(),
    removeEventListener: jest.fn(),
    dispatchEvent: jest.fn(),
  })),
});

// Mock window.getComputedStyle
Object.defineProperty(window, 'getComputedStyle', {
  value: () => ({
    getPropertyValue: () => '',
  }),
});

// Mock scrollTo
Object.defineProperty(window, 'scrollTo', {
  value: jest.fn(),
  writable: true
});

// Global test utilities
declare global {
  namespace jest {
    interface Matchers<R> {
      toBeInTheDocument(): R;
      toHaveClass(className: string): R;
      toHaveStyle(style: Record<string, any>): R;
      toHaveTextContent(text: string): R;
    }
  }
  
  var wait: (ms?: number) => Promise<void>;
  var createMockFunction: <T extends (...args: any[]) => any>(implementation?: T) => jest.MockedFunction<T>;
  var createMockAsyncFunction: <T>(resolveValue?: T, rejectValue?: any) => jest.MockedFunction<() => Promise<T>>;
}

// Global test helper functions
global.wait = (ms: number = 0): Promise<void> => {
  return new Promise(resolve => setTimeout(resolve, ms));
};

global.createMockFunction = <T extends (...args: any[]) => any>(
  implementation?: T
): jest.MockedFunction<T> => {
  return jest.fn(implementation) as jest.MockedFunction<T>;
};

global.createMockAsyncFunction = <T>(
  resolveValue?: T,
  rejectValue?: any
): jest.MockedFunction<() => Promise<T>> => {
  if (rejectValue !== undefined) {
    return jest.fn().mockRejectedValue(rejectValue);
  }
  return jest.fn().mockResolvedValue(resolveValue);
};

// Custom matchers
expect.extend({
  toBeWithinRange(received: number, floor: number, ceiling: number) {
    const pass = received >= floor && received <= ceiling;
    if (pass) {
      return {
        message: () => `expected ${received} not to be within range ${floor} - ${ceiling}`,
        pass: true,
      };
    } else {
      return {
        message: () => `expected ${received} to be within range ${floor} - ${ceiling}`,
        pass: false,
      };
    }
  },
});

declare global {
  namespace jest {
    interface Matchers<R> {
      toBeWithinRange(floor: number, ceiling: number): R;
    }
  }
}