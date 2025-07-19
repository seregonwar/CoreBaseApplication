import { useState, useEffect, useCallback } from 'react';
import { useCoreBase } from '../contexts/CoreBaseContext';
import {
  UseConfigManagerReturn,
  UseConfigManagerOptions
} from '../types';

/**
 * Hook for managing configuration functionality
 * @param options Configuration options for the config manager
 * @returns Config manager functions and state
 */
export const useConfigManager = (options: UseConfigManagerOptions = {}): UseConfigManagerReturn => {
  const { configManager, isInitialized } = useCoreBase();
  const [isManagerInitialized, setIsManagerInitialized] = useState(false);

  // Initialize config manager
  useEffect(() => {
    const initializeManager = async () => {
      if (!configManager || !isInitialized) {
        setIsManagerInitialized(false);
        return;
      }

      try {
        // Auto-load config file if specified
        if (options.autoLoad && options.configFile) {
          await configManager.loadConfig(options.configFile);
        }

        setIsManagerInitialized(true);
      } catch (error) {
        console.error('Failed to initialize config manager:', error);
        if (options.onError) {
          options.onError(error instanceof Error ? error : new Error(String(error)));
        }
        setIsManagerInitialized(false);
      }
    };

    initializeManager();
  }, [configManager, isInitialized, options.autoLoad, options.configFile, options.onError]);

  // Get configuration value
  const getValue = useCallback(async <T = any>(key: string, defaultValue?: T): Promise<T> => {
    if (!configManager || !isManagerInitialized) {
      if (defaultValue !== undefined) {
        return defaultValue;
      }
      throw new Error('Config manager not initialized');
    }

    try {
      const value = await configManager.getValue(key, defaultValue);
      return value;
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      if (defaultValue !== undefined) {
        return defaultValue;
      }
      throw err;
    }
  }, [configManager, isManagerInitialized, options.onError]);

  // Set configuration value
  const setValue = useCallback(async (key: string, value: any): Promise<void> => {
    if (!configManager || !isManagerInitialized) {
      throw new Error('Config manager not initialized');
    }

    try {
      await configManager.setValue(key, value);
      
      // Notify about config change
      if (options.onConfigChange) {
        options.onConfigChange(key, value);
      }
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [configManager, isManagerInitialized, options.onConfigChange, options.onError]);

  // Check if key exists
  const hasKey = useCallback(async (key: string): Promise<boolean> => {
    if (!configManager || !isManagerInitialized) {
      return false;
    }

    try {
      return await configManager.hasKey(key);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      return false;
    }
  }, [configManager, isManagerInitialized, options.onError]);

  // Remove configuration key
  const removeKey = useCallback(async (key: string): Promise<void> => {
    if (!configManager || !isManagerInitialized) {
      throw new Error('Config manager not initialized');
    }

    try {
      await configManager.removeKey(key);
      
      // Notify about config change
      if (options.onConfigChange) {
        options.onConfigChange(key, undefined);
      }
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [configManager, isManagerInitialized, options.onConfigChange, options.onError]);

  // Validate configuration
  const validateConfig = useCallback(async (config: Record<string, any>): Promise<boolean> => {
    if (!configManager || !isManagerInitialized) {
      throw new Error('Config manager not initialized');
    }

    try {
      return await configManager.validateConfig(config);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      return false;
    }
  }, [configManager, isManagerInitialized, options.onError]);

  // Load configuration from file
  const loadConfig = useCallback(async (filePath: string): Promise<void> => {
    if (!configManager || !isManagerInitialized) {
      throw new Error('Config manager not initialized');
    }

    try {
      await configManager.loadConfig(filePath);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [configManager, isManagerInitialized, options.onError]);

  // Save configuration to file
  const saveConfig = useCallback(async (filePath: string): Promise<void> => {
    if (!configManager || !isManagerInitialized) {
      throw new Error('Config manager not initialized');
    }

    try {
      await configManager.saveConfig(filePath);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [configManager, isManagerInitialized, options.onError]);

  return {
    configManager,
    getValue,
    setValue,
    hasKey,
    removeKey,
    validateConfig,
    loadConfig,
    saveConfig,
    isInitialized: isManagerInitialized
  };
};

export default useConfigManager;