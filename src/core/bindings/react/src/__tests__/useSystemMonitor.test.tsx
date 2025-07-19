import React from 'react';
import { renderHook, act, waitFor } from '@testing-library/react';
import { useSystemMonitor } from '../hooks/useSystemMonitor';
import { CoreBaseProvider } from '../contexts/CoreBaseContext';
import type { CoreBaseConfig, UseSystemMonitorOptions } from '../types';

// Mock the nodejs bindings
const mockSystemMonitor = {
  startMonitoring: jest.fn().mockResolvedValue(undefined),
  stopMonitoring: jest.fn().mockResolvedValue(undefined),
  isMonitoring: jest.fn().mockReturnValue(false),
  getCpuInfo: jest.fn().mockReturnValue({
    usage: 45.5,
    cores: 8,
    threads: 16,
    frequency: 3200,
    model: 'Intel Core i7-9700K'
  }),
  getMemoryInfo: jest.fn().mockReturnValue({
    usage: 65.2,
    total: 16777216000,
    used: 10737418240,
    available: 6039797760,
    free: 6039797760
  }),
  getDiskInfo: jest.fn().mockReturnValue([
    {
      device: 'C:',
      mountPoint: 'C:\\',
      usage: 75.3,
      total: 1000000000000,
      used: 753000000000,
      free: 247000000000,
      fileSystem: 'NTFS'
    }
  ]),
  getNetworkInfo: jest.fn().mockReturnValue({
    bytesSent: 1048576,
    bytesReceived: 2097152,
    packetsSent: 1000,
    packetsReceived: 1500,
    errorsIn: 0,
    errorsOut: 0
  }),
  getProcessInfo: jest.fn().mockReturnValue({
    pid: 1234,
    cpuUsage: 12.5,
    memoryUsage: 134217728,
    threads: 4,
    handles: 256,
    startTime: Date.now() - 3600000
  }),
  getLoadInfo: jest.fn().mockReturnValue({
    oneMinute: 1.2,
    fiveMinutes: 1.5,
    fifteenMinutes: 1.8
  }),
  getTemperatureInfo: jest.fn().mockReturnValue({
    cpu: 65,
    gpu: 70,
    motherboard: 45
  }),
  getPerformanceMetrics: jest.fn().mockReturnValue({
    bootTime: 45,
    uptime: 86400,
    contextSwitches: 1000000,
    interrupts: 500000,
    systemCalls: 2000000
  }),
  getAlerts: jest.fn().mockReturnValue([]),
  setMonitoringInterval: jest.fn().mockResolvedValue(undefined),
  refreshData: jest.fn().mockResolvedValue(undefined),
  exportData: jest.fn().mockResolvedValue('exported-data')
};

jest.mock('@corebase/nodejs', () => ({
  CoreBaseManager: jest.fn().mockImplementation(() => ({
    initialize: jest.fn().mockResolvedValue(undefined),
    shutdown: jest.fn().mockResolvedValue(undefined),
    getPlatformInfo: jest.fn().mockReturnValue({}),
    getVersion: jest.fn().mockReturnValue('1.0.0'),
    getErrorHandler: jest.fn().mockReturnValue({}),
    getConfigManager: jest.fn().mockReturnValue({}),
    getSystemMonitor: jest.fn().mockReturnValue(mockSystemMonitor),
    getNetworkManager: jest.fn().mockReturnValue({})
  }))
}));

describe('useSystemMonitor', () => {
  const wrapper: React.FC<{ children: React.ReactNode }> = ({ children }) => {
    const config: CoreBaseConfig = {
      logLevel: 'info',
      autoInitialize: true
    };

    return (
      <CoreBaseProvider config={config}>
        {children}
      </CoreBaseProvider>
    );
  };

  beforeEach(() => {
    jest.clearAllMocks();
  });

  it('should initialize with default state', async () => {
    const { result } = renderHook(() => useSystemMonitor(), { wrapper });

    expect(result.current.cpuInfo).toBeNull();
    expect(result.current.memoryInfo).toBeNull();
    expect(result.current.diskInfo).toBeNull();
    expect(result.current.networkInfo).toBeNull();
    expect(result.current.processInfo).toBeNull();
    expect(result.current.loadInfo).toBeNull();
    expect(result.current.temperatureInfo).toBeNull();
    expect(result.current.performanceMetrics).toBeNull();
    expect(result.current.alerts).toEqual([]);
    expect(result.current.isMonitoring).toBe(false);
    expect(result.current.error).toBeNull();
  });

  it('should start monitoring when autoStart is true', async () => {
    const options: UseSystemMonitorOptions = {
      autoStart: true,
      refreshInterval: 1000
    };

    renderHook(() => useSystemMonitor(options), { wrapper });

    await waitFor(() => {
      expect(mockSystemMonitor.startMonitoring).toHaveBeenCalled();
    });
  });

  it('should start and stop monitoring', async () => {
    const { result } = renderHook(() => useSystemMonitor(), { wrapper });

    await waitFor(() => {
      expect(result.current.startMonitoring).toBeDefined();
    });

    await act(async () => {
      await result.current.startMonitoring();
    });

    expect(mockSystemMonitor.startMonitoring).toHaveBeenCalled();

    await act(async () => {
      await result.current.stopMonitoring();
    });

    expect(mockSystemMonitor.stopMonitoring).toHaveBeenCalled();
  });

  it('should refresh data and update state', async () => {
    const { result } = renderHook(() => useSystemMonitor(), { wrapper });

    await waitFor(() => {
      expect(result.current.refreshData).toBeDefined();
    });

    await act(async () => {
      await result.current.refreshData();
    });

    expect(mockSystemMonitor.getCpuInfo).toHaveBeenCalled();
    expect(mockSystemMonitor.getMemoryInfo).toHaveBeenCalled();
    expect(mockSystemMonitor.getDiskInfo).toHaveBeenCalled();
    expect(mockSystemMonitor.getNetworkInfo).toHaveBeenCalled();
    expect(mockSystemMonitor.getProcessInfo).toHaveBeenCalled();
    expect(mockSystemMonitor.getLoadInfo).toHaveBeenCalled();
    expect(mockSystemMonitor.getTemperatureInfo).toHaveBeenCalled();
    expect(mockSystemMonitor.getPerformanceMetrics).toHaveBeenCalled();
    expect(mockSystemMonitor.getAlerts).toHaveBeenCalled();

    expect(result.current.cpuInfo).toEqual({
      usage: 45.5,
      cores: 8,
      threads: 16,
      frequency: 3200,
      model: 'Intel Core i7-9700K'
    });

    expect(result.current.memoryInfo).toEqual({
      usage: 65.2,
      total: 16777216000,
      used: 10737418240,
      available: 6039797760,
      free: 6039797760
    });
  });

  it('should set monitoring interval', async () => {
    const { result } = renderHook(() => useSystemMonitor(), { wrapper });

    await waitFor(() => {
      expect(result.current.setMonitoringInterval).toBeDefined();
    });

    await act(async () => {
      await result.current.setMonitoringInterval(2000);
    });

    expect(mockSystemMonitor.setMonitoringInterval).toHaveBeenCalledWith(2000);
  });

  it('should export data', async () => {
    const { result } = renderHook(() => useSystemMonitor(), { wrapper });

    await waitFor(() => {
      expect(result.current.exportData).toBeDefined();
    });

    let exportedData: string | undefined;
    await act(async () => {
      exportedData = await result.current.exportData();
    });

    expect(mockSystemMonitor.exportData).toHaveBeenCalled();
    expect(exportedData).toBe('exported-data');
  });

  it('should handle errors gracefully', async () => {
    const errorMessage = 'System monitor error';
    mockSystemMonitor.startMonitoring.mockRejectedValueOnce(new Error(errorMessage));

    const { result } = renderHook(() => useSystemMonitor(), { wrapper });

    await waitFor(() => {
      expect(result.current.startMonitoring).toBeDefined();
    });

    await act(async () => {
      await result.current.startMonitoring();
    });

    expect(result.current.error).toBe(errorMessage);
  });

  it('should call onAlert callback when alerts are detected', async () => {
    const mockAlert = {
      type: 'cpu',
      severity: 'warning' as const,
      message: 'High CPU usage detected',
      timestamp: Date.now(),
      value: 95.5,
      threshold: 90
    };

    mockSystemMonitor.getAlerts.mockReturnValue([mockAlert]);

    const onAlert = jest.fn();
    const options: UseSystemMonitorOptions = {
      onAlert
    };

    const { result } = renderHook(() => useSystemMonitor(options), { wrapper });

    await waitFor(() => {
      expect(result.current.refreshData).toBeDefined();
    });

    await act(async () => {
      await result.current.refreshData();
    });

    expect(result.current.alerts).toEqual([mockAlert]);
    expect(onAlert).toHaveBeenCalledWith(mockAlert);
  });

  it('should call onError callback when errors occur', async () => {
    const errorMessage = 'Test error';
    const onError = jest.fn();
    const options: UseSystemMonitorOptions = {
      onError
    };

    mockSystemMonitor.refreshData.mockRejectedValueOnce(new Error(errorMessage));

    const { result } = renderHook(() => useSystemMonitor(options), { wrapper });

    await waitFor(() => {
      expect(result.current.refreshData).toBeDefined();
    });

    await act(async () => {
      await result.current.refreshData();
    });

    expect(onError).toHaveBeenCalledWith(errorMessage);
  });

  it('should auto-refresh data when autoRefresh is enabled', async () => {
    jest.useFakeTimers();

    const options: UseSystemMonitorOptions = {
      autoRefresh: true,
      refreshInterval: 1000
    };

    renderHook(() => useSystemMonitor(options), { wrapper });

    await waitFor(() => {
      expect(mockSystemMonitor.getCpuInfo).toHaveBeenCalled();
    });

    // Clear previous calls
    jest.clearAllMocks();

    // Fast-forward time
    act(() => {
      jest.advanceTimersByTime(1000);
    });

    await waitFor(() => {
      expect(mockSystemMonitor.getCpuInfo).toHaveBeenCalled();
    });

    jest.useRealTimers();
  });
});