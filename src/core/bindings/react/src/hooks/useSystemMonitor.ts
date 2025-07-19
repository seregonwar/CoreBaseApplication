import { useState, useEffect, useCallback, useRef } from 'react';
import { useCoreBase } from '../contexts/CoreBaseContext';
import {
  UseSystemMonitorReturn,
  UseSystemMonitorOptions,
  CpuInfo,
  MemoryInfo,
  DiskInfo,
  NetworkInfo,
  ProcessInfo,
  LoadInfo,
  TemperatureInfo,
  PerformanceMetrics,
  AlertInfo
} from '../types';

/**
 * Hook for managing system monitoring functionality
 * @param options Configuration options for the system monitor
 * @returns System monitor functions and state
 */
export const useSystemMonitor = (options: UseSystemMonitorOptions = {}): UseSystemMonitorReturn => {
  const { systemMonitor, isInitialized } = useCoreBase();
  const [isMonitorInitialized, setIsMonitorInitialized] = useState(false);
  const [isMonitoring, setIsMonitoring] = useState(false);
  const [cpuInfo, setCpuInfo] = useState<CpuInfo | null>(null);
  const [memoryInfo, setMemoryInfo] = useState<MemoryInfo | null>(null);
  const [diskInfo, setDiskInfo] = useState<DiskInfo[] | null>(null);
  const [networkInfo, setNetworkInfo] = useState<NetworkInfo[] | null>(null);
  const [processInfo, setProcessInfo] = useState<ProcessInfo[] | null>(null);
  const [loadInfo, setLoadInfo] = useState<LoadInfo | null>(null);
  const [temperatureInfo, setTemperatureInfo] = useState<TemperatureInfo | null>(null);
  const [performanceMetrics, setPerformanceMetrics] = useState<PerformanceMetrics | null>(null);
  const [alerts, setAlerts] = useState<AlertInfo[] | null>(null);
  
  const refreshIntervalRef = useRef<NodeJS.Timeout | null>(null);
  const alertCheckIntervalRef = useRef<NodeJS.Timeout | null>(null);

  // Initialize system monitor
  useEffect(() => {
    const initializeMonitor = async () => {
      if (!systemMonitor || !isInitialized) {
        setIsMonitorInitialized(false);
        return;
      }

      try {
        // Set refresh interval if provided
        if (options.refreshInterval) {
          await systemMonitor.setMonitoringInterval(options.refreshInterval);
        }

        // Auto-start monitoring if enabled
        if (options.autoStart) {
          await systemMonitor.startMonitoring();
          setIsMonitoring(true);
        }

        setIsMonitorInitialized(true);
      } catch (error) {
        console.error('Failed to initialize system monitor:', error);
        if (options.onError) {
          options.onError(error instanceof Error ? error : new Error(String(error)));
        }
        setIsMonitorInitialized(false);
      }
    };

    initializeMonitor();
  }, [systemMonitor, isInitialized, options.autoStart, options.refreshInterval, options.onError]);

  // Setup data refresh interval
  useEffect(() => {
    if (!isMonitorInitialized || !isMonitoring) {
      return;
    }

    const refreshData = async () => {
      try {
        await updateAllData();
      } catch (error) {
        console.error('Failed to refresh system data:', error);
        if (options.onError) {
          options.onError(error instanceof Error ? error : new Error(String(error)));
        }
      }
    };

    // Initial data load
    refreshData();

    // Setup refresh interval
    const interval = options.refreshInterval || 1000;
    refreshIntervalRef.current = setInterval(refreshData, interval);

    return () => {
      if (refreshIntervalRef.current) {
        clearInterval(refreshIntervalRef.current);
        refreshIntervalRef.current = null;
      }
    };
  }, [isMonitorInitialized, isMonitoring, options.refreshInterval, options.onError]);

  // Setup alert checking
  useEffect(() => {
    if (!isMonitorInitialized || !isMonitoring || !options.enableAlerts) {
      return;
    }

    const checkAlerts = async () => {
      try {
        if (systemMonitor) {
          const currentAlerts = await systemMonitor.getAlerts();
          setAlerts(currentAlerts);
          
          // Notify about new alerts
          if (options.onAlert && currentAlerts.length > 0) {
            currentAlerts.forEach(alert => {
              options.onAlert!(alert);
            });
          }
        }
      } catch (error) {
        console.error('Failed to check alerts:', error);
      }
    };

    // Check alerts every 5 seconds
    alertCheckIntervalRef.current = setInterval(checkAlerts, 5000);

    return () => {
      if (alertCheckIntervalRef.current) {
        clearInterval(alertCheckIntervalRef.current);
        alertCheckIntervalRef.current = null;
      }
    };
  }, [isMonitorInitialized, isMonitoring, options.enableAlerts, options.onAlert, systemMonitor]);

  // Update all system data
  const updateAllData = useCallback(async (): Promise<void> => {
    if (!systemMonitor || !isMonitorInitialized) {
      return;
    }

    try {
      const [cpu, memory, disk, network, process, load, temperature, performance] = await Promise.allSettled([
        systemMonitor.getCpuInfo(),
        systemMonitor.getMemoryInfo(),
        systemMonitor.getDiskInfo(),
        systemMonitor.getNetworkInfo(),
        systemMonitor.getProcessInfo(),
        systemMonitor.getLoadInfo(),
        systemMonitor.getTemperatureInfo(),
        systemMonitor.getPerformanceMetrics()
      ]);

      if (cpu.status === 'fulfilled') setCpuInfo(cpu.value);
      if (memory.status === 'fulfilled') setMemoryInfo(memory.value);
      if (disk.status === 'fulfilled') setDiskInfo(disk.value);
      if (network.status === 'fulfilled') setNetworkInfo(network.value);
      if (process.status === 'fulfilled') setProcessInfo(process.value);
      if (load.status === 'fulfilled') setLoadInfo(load.value);
      if (temperature.status === 'fulfilled') setTemperatureInfo(temperature.value);
      if (performance.status === 'fulfilled') setPerformanceMetrics(performance.value);
    } catch (error) {
      console.error('Failed to update system data:', error);
      if (options.onError) {
        options.onError(error instanceof Error ? error : new Error(String(error)));
      }
    }
  }, [systemMonitor, isMonitorInitialized, options.onError]);

  // Start monitoring
  const startMonitoring = useCallback(async (): Promise<void> => {
    if (!systemMonitor || !isMonitorInitialized) {
      throw new Error('System monitor not initialized');
    }

    try {
      await systemMonitor.startMonitoring();
      setIsMonitoring(true);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [systemMonitor, isMonitorInitialized, options.onError]);

  // Stop monitoring
  const stopMonitoring = useCallback(async (): Promise<void> => {
    if (!systemMonitor || !isMonitorInitialized) {
      throw new Error('System monitor not initialized');
    }

    try {
      await systemMonitor.stopMonitoring();
      setIsMonitoring(false);
      
      // Clear intervals
      if (refreshIntervalRef.current) {
        clearInterval(refreshIntervalRef.current);
        refreshIntervalRef.current = null;
      }
      if (alertCheckIntervalRef.current) {
        clearInterval(alertCheckIntervalRef.current);
        alertCheckIntervalRef.current = null;
      }
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [systemMonitor, isMonitorInitialized, options.onError]);

  // Refresh data manually
  const refreshData = useCallback(async (): Promise<void> => {
    await updateAllData();
  }, [updateAllData]);

  // Set monitoring interval
  const setMonitoringInterval = useCallback(async (interval: number): Promise<void> => {
    if (!systemMonitor || !isMonitorInitialized) {
      throw new Error('System monitor not initialized');
    }

    try {
      await systemMonitor.setMonitoringInterval(interval);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [systemMonitor, isMonitorInitialized, options.onError]);

  // Export data
  const exportData = useCallback(async (format: string, filePath: string): Promise<void> => {
    if (!systemMonitor || !isMonitorInitialized) {
      throw new Error('System monitor not initialized');
    }

    try {
      await systemMonitor.exportData(format, filePath);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [systemMonitor, isMonitorInitialized, options.onError]);

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      if (refreshIntervalRef.current) {
        clearInterval(refreshIntervalRef.current);
      }
      if (alertCheckIntervalRef.current) {
        clearInterval(alertCheckIntervalRef.current);
      }
    };
  }, []);

  return {
    systemMonitor,
    isMonitoring,
    cpuInfo,
    memoryInfo,
    diskInfo,
    networkInfo,
    processInfo,
    loadInfo,
    temperatureInfo,
    performanceMetrics,
    alerts,
    startMonitoring,
    stopMonitoring,
    refreshData,
    setMonitoringInterval,
    exportData,
    isInitialized: isMonitorInitialized
  };
};

export default useSystemMonitor;