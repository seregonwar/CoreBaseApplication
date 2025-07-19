import React from 'react';
import { renderHook, act } from '@testing-library/react';
import { jest } from '@jest/globals';

import { useErrorHandler } from '../hooks/useErrorHandler';
import { CoreBaseProvider } from '../contexts/CoreBaseContext';

// Mock @corebase/nodejs
const mockErrorHandler = {
  setLogLevel: jest.fn(),
  getLogLevel: jest.fn().mockReturnValue('info'),
  logError: jest.fn(),
  logWarning: jest.fn(),
  logInfo: jest.fn(),
  logDebug: jest.fn(),
  handleError: jest.fn(),
  handleException: jest.fn(),
};

jest.mock('@corebase/nodejs', () => ({
  CoreBaseManager: jest.fn().mockImplementation(() => ({
    initialize: jest.fn().mockResolvedValue(undefined),
    shutdown: jest.fn().mockResolvedValue(undefined),
    getErrorHandler: jest.fn().mockReturnValue(mockErrorHandler),
  })),
  ErrorLevel: {
    DEBUG: 0,
    INFO: 1,
    WARNING: 2,
    ERROR: 3,
  },
}));

const TestWrapper: React.FC<{ children: React.ReactNode }> = ({ children }) => (
  <CoreBaseProvider config={{ autoInitialize: true }}>
    {children}
  </CoreBaseProvider>
);

describe('useErrorHandler', () => {
  beforeEach(() => {
    jest.clearAllMocks();
  });

  it('should return error handler functions', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    expect(result.current.setLogLevel).toBeDefined();
    expect(result.current.getLogLevel).toBeDefined();
    expect(result.current.logError).toBeDefined();
    expect(result.current.logWarning).toBeDefined();
    expect(result.current.logInfo).toBeDefined();
    expect(result.current.logDebug).toBeDefined();
    expect(result.current.handleError).toBeDefined();
    expect(result.current.handleException).toBeDefined();
  });

  it('should get current log level', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    const logLevel = result.current.getLogLevel();
    expect(logLevel).toBe('info');
    expect(mockErrorHandler.getLogLevel).toHaveBeenCalled();
  });

  it('should set log level', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.setLogLevel('debug');
    });

    expect(mockErrorHandler.setLogLevel).toHaveBeenCalledWith('debug');
  });

  it('should log error messages', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.logError('Test error message');
    });

    expect(mockErrorHandler.logError).toHaveBeenCalledWith('Test error message');
  });

  it('should log error messages with context', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    const context = { userId: '123', action: 'login' };

    act(() => {
      result.current.logError('Login failed', context);
    });

    expect(mockErrorHandler.logError).toHaveBeenCalledWith('Login failed', context);
  });

  it('should log warning messages', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.logWarning('Test warning message');
    });

    expect(mockErrorHandler.logWarning).toHaveBeenCalledWith('Test warning message');
  });

  it('should log info messages', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.logInfo('Test info message');
    });

    expect(mockErrorHandler.logInfo).toHaveBeenCalledWith('Test info message');
  });

  it('should log debug messages', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.logDebug('Test debug message');
    });

    expect(mockErrorHandler.logDebug).toHaveBeenCalledWith('Test debug message');
  });

  it('should handle error objects', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    const error = new Error('Test error');
    const context = { component: 'TestComponent' };

    act(() => {
      result.current.handleError(error, context);
    });

    expect(mockErrorHandler.handleError).toHaveBeenCalledWith(error, context);
  });

  it('should handle exceptions', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    const exception = new Error('Test exception');
    const context = { method: 'testMethod' };

    act(() => {
      result.current.handleException(exception, context);
    });

    expect(mockErrorHandler.handleException).toHaveBeenCalledWith(exception, context);
  });

  it('should handle string errors', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.handleError('String error message');
    });

    expect(mockErrorHandler.handleError).toHaveBeenCalledWith('String error message', undefined);
  });

  it('should work with different log levels', () => {
    mockErrorHandler.getLogLevel.mockReturnValue('debug');

    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    expect(result.current.getLogLevel()).toBe('debug');

    act(() => {
      result.current.setLogLevel('error');
    });

    expect(mockErrorHandler.setLogLevel).toHaveBeenCalledWith('error');
  });

  it('should handle multiple log calls', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.logInfo('First message');
      result.current.logWarning('Second message');
      result.current.logError('Third message');
    });

    expect(mockErrorHandler.logInfo).toHaveBeenCalledWith('First message');
    expect(mockErrorHandler.logWarning).toHaveBeenCalledWith('Second message');
    expect(mockErrorHandler.logError).toHaveBeenCalledWith('Third message');
  });

  it('should handle complex context objects', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    const complexContext = {
      user: { id: '123', name: 'John Doe' },
      request: { url: '/api/test', method: 'POST' },
      timestamp: new Date().toISOString(),
      metadata: { version: '1.0.0', environment: 'test' },
    };

    act(() => {
      result.current.logError('Complex error', complexContext);
    });

    expect(mockErrorHandler.logError).toHaveBeenCalledWith('Complex error', complexContext);
  });

  it('should handle errors without context', () => {
    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.logError('Simple error');
      result.current.logWarning('Simple warning');
      result.current.logInfo('Simple info');
      result.current.logDebug('Simple debug');
    });

    expect(mockErrorHandler.logError).toHaveBeenCalledWith('Simple error');
    expect(mockErrorHandler.logWarning).toHaveBeenCalledWith('Simple warning');
    expect(mockErrorHandler.logInfo).toHaveBeenCalledWith('Simple info');
    expect(mockErrorHandler.logDebug).toHaveBeenCalledWith('Simple debug');
  });

  it('should throw error when used outside provider', () => {
    const consoleSpy = jest.spyOn(console, 'error').mockImplementation(() => {});

    expect(() => {
      renderHook(() => useErrorHandler());
    }).toThrow('useErrorHandler must be used within a CoreBaseProvider');

    consoleSpy.mockRestore();
  });

  it('should handle error handler not ready', () => {
    const TestWrapperNotReady: React.FC<{ children: React.ReactNode }> = ({ children }) => (
      <CoreBaseProvider config={{ autoInitialize: false }}>
        {children}
      </CoreBaseProvider>
    );

    const { result } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapperNotReady,
    });

    // Should not throw, but functions should handle the case gracefully
    act(() => {
      result.current.logError('Test error');
    });

    // The actual behavior depends on implementation
    // This test ensures no crash occurs when error handler is not ready
  });

  it('should maintain function references across re-renders', () => {
    const { result, rerender } = renderHook(() => useErrorHandler(), {
      wrapper: TestWrapper,
    });

    const firstRenderFunctions = {
      setLogLevel: result.current.setLogLevel,
      getLogLevel: result.current.getLogLevel,
      logError: result.current.logError,
      logWarning: result.current.logWarning,
      logInfo: result.current.logInfo,
      logDebug: result.current.logDebug,
      handleError: result.current.handleError,
      handleException: result.current.handleException,
    };

    rerender();

    // Functions should be memoized and maintain references
    expect(result.current.setLogLevel).toBe(firstRenderFunctions.setLogLevel);
    expect(result.current.getLogLevel).toBe(firstRenderFunctions.getLogLevel);
    expect(result.current.logError).toBe(firstRenderFunctions.logError);
    expect(result.current.logWarning).toBe(firstRenderFunctions.logWarning);
    expect(result.current.logInfo).toBe(firstRenderFunctions.logInfo);
    expect(result.current.logDebug).toBe(firstRenderFunctions.logDebug);
    expect(result.current.handleError).toBe(firstRenderFunctions.handleError);
    expect(result.current.handleException).toBe(firstRenderFunctions.handleException);
  });
});