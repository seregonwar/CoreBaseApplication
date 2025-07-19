import { useState, useEffect, useCallback } from 'react';
import { useCoreBase } from '../contexts/CoreBaseContext';
import {
  UseErrorHandlerReturn,
  UseErrorHandlerOptions,
  LogLevel
} from '../types';

/**
 * Hook for managing error handling functionality
 * @param options Configuration options for the error handler
 * @returns Error handler functions and state
 */
export const useErrorHandler = (options: UseErrorHandlerOptions = {}): UseErrorHandlerReturn => {
  const { errorHandler, isInitialized } = useCoreBase();
  const [logLevel, setLogLevelState] = useState<LogLevel>(options.defaultLogLevel || 'info');
  const [isHandlerInitialized, setIsHandlerInitialized] = useState(false);

  // Initialize error handler
  useEffect(() => {
    const initializeHandler = async () => {
      if (!errorHandler || !isInitialized) {
        setIsHandlerInitialized(false);
        return;
      }

      try {
        // Set default log level if provided
        if (options.defaultLogLevel) {
          await errorHandler.setLogLevel(options.defaultLogLevel);
          setLogLevelState(options.defaultLogLevel);
        } else {
          // Get current log level
          const currentLevel = await errorHandler.getLogLevel();
          setLogLevelState(currentLevel);
        }

        setIsHandlerInitialized(true);
      } catch (error) {
        console.error('Failed to initialize error handler:', error);
        if (options.onError) {
          options.onError(error instanceof Error ? error : new Error(String(error)));
        }
        setIsHandlerInitialized(false);
      }
    };

    initializeHandler();
  }, [errorHandler, isInitialized, options.defaultLogLevel, options.onError]);

  // Set log level
  const setLogLevel = useCallback(async (level: LogLevel): Promise<void> => {
    if (!errorHandler || !isHandlerInitialized) {
      throw new Error('Error handler not initialized');
    }

    try {
      await errorHandler.setLogLevel(level);
      setLogLevelState(level);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [errorHandler, isHandlerInitialized, options.onError]);

  // Log error message
  const logError = useCallback(async (message: string, details?: string): Promise<void> => {
    if (!errorHandler || !isHandlerInitialized) {
      if (options.enableConsoleOutput !== false) {
        console.error(`[ERROR] ${message}`, details || '');
      }
      return;
    }

    try {
      await errorHandler.logError(message, details);
      if (options.enableConsoleOutput !== false) {
        console.error(`[ERROR] ${message}`, details || '');
      }
    } catch (error) {
      console.error('Failed to log error:', error);
      if (options.onError) {
        options.onError(error instanceof Error ? error : new Error(String(error)));
      }
    }
  }, [errorHandler, isHandlerInitialized, options.enableConsoleOutput, options.onError]);

  // Log warning message
  const logWarning = useCallback(async (message: string, details?: string): Promise<void> => {
    if (!errorHandler || !isHandlerInitialized) {
      if (options.enableConsoleOutput !== false) {
        console.warn(`[WARNING] ${message}`, details || '');
      }
      return;
    }

    try {
      await errorHandler.logWarning(message, details);
      if (options.enableConsoleOutput !== false) {
        console.warn(`[WARNING] ${message}`, details || '');
      }
    } catch (error) {
      console.error('Failed to log warning:', error);
      if (options.onError) {
        options.onError(error instanceof Error ? error : new Error(String(error)));
      }
    }
  }, [errorHandler, isHandlerInitialized, options.enableConsoleOutput, options.onError]);

  // Log info message
  const logInfo = useCallback(async (message: string, details?: string): Promise<void> => {
    if (!errorHandler || !isHandlerInitialized) {
      if (options.enableConsoleOutput !== false) {
        console.info(`[INFO] ${message}`, details || '');
      }
      return;
    }

    try {
      await errorHandler.logInfo(message, details);
      if (options.enableConsoleOutput !== false) {
        console.info(`[INFO] ${message}`, details || '');
      }
    } catch (error) {
      console.error('Failed to log info:', error);
      if (options.onError) {
        options.onError(error instanceof Error ? error : new Error(String(error)));
      }
    }
  }, [errorHandler, isHandlerInitialized, options.enableConsoleOutput, options.onError]);

  // Log debug message
  const logDebug = useCallback(async (message: string, details?: string): Promise<void> => {
    if (!errorHandler || !isHandlerInitialized) {
      if (options.enableConsoleOutput !== false) {
        console.debug(`[DEBUG] ${message}`, details || '');
      }
      return;
    }

    try {
      await errorHandler.logDebug(message, details);
      if (options.enableConsoleOutput !== false) {
        console.debug(`[DEBUG] ${message}`, details || '');
      }
    } catch (error) {
      console.error('Failed to log debug:', error);
      if (options.onError) {
        options.onError(error instanceof Error ? error : new Error(String(error)));
      }
    }
  }, [errorHandler, isHandlerInitialized, options.enableConsoleOutput, options.onError]);

  // Handle error object
  const handleError = useCallback(async (error: Error): Promise<void> => {
    if (!errorHandler || !isHandlerInitialized) {
      if (options.enableConsoleOutput !== false) {
        console.error('[ERROR]', error);
      }
      if (options.onError) {
        options.onError(error);
      }
      return;
    }

    try {
      await errorHandler.handleError(error.message, error.stack || '');
      if (options.enableConsoleOutput !== false) {
        console.error('[ERROR]', error);
      }
    } catch (handlerError) {
      console.error('Failed to handle error:', handlerError);
      if (options.onError) {
        options.onError(handlerError instanceof Error ? handlerError : new Error(String(handlerError)));
      }
    }
  }, [errorHandler, isHandlerInitialized, options.enableConsoleOutput, options.onError]);

  // Handle exception string
  const handleException = useCallback(async (exception: string): Promise<void> => {
    if (!errorHandler || !isHandlerInitialized) {
      if (options.enableConsoleOutput !== false) {
        console.error(`[EXCEPTION] ${exception}`);
      }
      return;
    }

    try {
      await errorHandler.handleException(exception);
      if (options.enableConsoleOutput !== false) {
        console.error(`[EXCEPTION] ${exception}`);
      }
    } catch (error) {
      console.error('Failed to handle exception:', error);
      if (options.onError) {
        options.onError(error instanceof Error ? error : new Error(String(error)));
      }
    }
  }, [errorHandler, isHandlerInitialized, options.enableConsoleOutput, options.onError]);

  return {
    errorHandler,
    logLevel,
    setLogLevel,
    logError,
    logWarning,
    logInfo,
    logDebug,
    handleError,
    handleException,
    isInitialized: isHandlerInitialized
  };
};

export default useErrorHandler;