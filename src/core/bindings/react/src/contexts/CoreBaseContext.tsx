import React, { createContext, useContext, useEffect, useState, useCallback, ReactNode } from 'react';
import {
  getCoreBase,
  initializeCoreBase,
  shutdownCoreBase,
  getPlatformInfo,
  getVersion
} from '@corebase/nodejs';
import {
  CoreBaseContextValue,
  CoreBaseConfig,
  CoreBaseProviderProps,
  LogLevel,
  PlatformInfo
} from '../types';

// Create the context
const CoreBaseContext = createContext<CoreBaseContextValue | undefined>(undefined);

// Default configuration
const defaultConfig: CoreBaseConfig = {
  logLevel: 'info' as LogLevel,
  monitoringInterval: 1000,
  enableSystemMonitoring: true,
  enableNetworkMonitoring: true,
  autoStart: true,
  errorReporting: true
};

/**
 * CoreBase Provider Component
 * Provides CoreBase functionality to the React component tree
 */
export const CoreBaseProvider: React.FC<CoreBaseProviderProps> = ({
  children,
  config = defaultConfig,
  onError,
  onInitialized
}) => {
  const [isInitialized, setIsInitialized] = useState(false);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState<Error | null>(null);
  const [platformInfo, setPlatformInfo] = useState<PlatformInfo | null>(null);
  const [version, setVersion] = useState<string | null>(null);
  const [coreBase, setCoreBase] = useState<any>(null);

  // Handle errors
  const handleError = useCallback((err: Error) => {
    console.error('CoreBase Error:', err);
    setError(err);
    if (onError) {
      onError(err);
    }
  }, [onError]);

  // Initialize CoreBase
  const initialize = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);

      // Initialize CoreBase with configuration
      await initializeCoreBase({
        logLevel: config.logLevel || 'info',
        enableSystemMonitoring: config.enableSystemMonitoring !== false,
        enableNetworkMonitoring: config.enableNetworkMonitoring !== false
      });

      // Get CoreBase instance
      const instance = getCoreBase();
      setCoreBase(instance);

      // Get platform information
      try {
        const platform = await getPlatformInfo();
        setPlatformInfo(platform);
      } catch (err) {
        console.warn('Failed to get platform info:', err);
      }

      // Get version
      try {
        const ver = await getVersion();
        setVersion(ver);
      } catch (err) {
        console.warn('Failed to get version:', err);
      }

      // Configure error handler if available
      if (instance?.errorHandler && config.logLevel) {
        try {
          await instance.errorHandler.setLogLevel(config.logLevel);
        } catch (err) {
          console.warn('Failed to set log level:', err);
        }
      }

      // Configure system monitor if available
      if (instance?.systemMonitor && config.monitoringInterval) {
        try {
          await instance.systemMonitor.setMonitoringInterval(config.monitoringInterval);
          
          if (config.autoStart) {
            await instance.systemMonitor.startMonitoring();
          }
        } catch (err) {
          console.warn('Failed to configure system monitor:', err);
        }
      }

      setIsInitialized(true);
      if (onInitialized) {
        onInitialized();
      }
    } catch (err) {
      const error = err instanceof Error ? err : new Error(String(err));
      handleError(error);
    } finally {
      setIsLoading(false);
    }
  }, [config, onInitialized, handleError]);

  // Cleanup function
  const cleanup = useCallback(async () => {
    try {
      if (coreBase?.systemMonitor) {
        await coreBase.systemMonitor.stopMonitoring();
      }
      await shutdownCoreBase();
    } catch (err) {
      console.warn('Error during cleanup:', err);
    }
  }, [coreBase]);

  // Initialize on mount
  useEffect(() => {
    initialize();

    // Cleanup on unmount
    return () => {
      cleanup();
    };
  }, [initialize, cleanup]);

  // Context value
  const contextValue: CoreBaseContextValue = {
    errorHandler: coreBase?.errorHandler || null,
    configManager: coreBase?.configManager || null,
    systemMonitor: coreBase?.systemMonitor || null,
    networkManager: coreBase?.networkManager || null,
    isInitialized,
    isLoading,
    error,
    platformInfo,
    version
  };

  return (
    <CoreBaseContext.Provider value={contextValue}>
      {children}
    </CoreBaseContext.Provider>
  );
};

/**
 * Hook to use CoreBase context
 * @returns CoreBase context value
 * @throws Error if used outside of CoreBaseProvider
 */
export const useCoreBase = (): CoreBaseContextValue => {
  const context = useContext(CoreBaseContext);
  
  if (context === undefined) {
    throw new Error('useCoreBase must be used within a CoreBaseProvider');
  }
  
  return context;
};

/**
 * Hook to check if CoreBase is ready
 * @returns boolean indicating if CoreBase is initialized and ready
 */
export const useCoreBaseReady = (): boolean => {
  const { isInitialized, isLoading, error } = useCoreBase();
  return isInitialized && !isLoading && !error;
};

/**
 * Hook to get CoreBase error state
 * @returns current error or null
 */
export const useCoreBaseError = (): Error | null => {
  const { error } = useCoreBase();
  return error;
};

/**
 * Hook to get CoreBase loading state
 * @returns boolean indicating if CoreBase is loading
 */
export const useCoreBaseLoading = (): boolean => {
  const { isLoading } = useCoreBase();
  return isLoading;
};

/**
 * Hook to get platform information
 * @returns platform information or null
 */
export const usePlatformInfo = (): PlatformInfo | null => {
  const { platformInfo } = useCoreBase();
  return platformInfo;
};

/**
 * Hook to get CoreBase version
 * @returns version string or null
 */
export const useCoreBaseVersion = (): string | null => {
  const { version } = useCoreBase();
  return version;
};

export default CoreBaseContext;