import React from 'react';
import { renderHook, act } from '@testing-library/react';
import { jest } from '@jest/globals';

import { useNetworkManager } from '../hooks/useNetworkManager';
import { CoreBaseProvider } from '../contexts/CoreBaseContext';

// Mock @corebase/nodejs
const mockNetworkManager = {
  getConnectionStats: jest.fn(),
  getBandwidthUsage: jest.fn(),
  getConnectionStatus: jest.fn(),
  startMonitoring: jest.fn(),
  stopMonitoring: jest.fn(),
  isMonitoring: jest.fn(),
  refreshData: jest.fn(),
  exportData: jest.fn(),
  setRefreshInterval: jest.fn(),
  getRefreshInterval: jest.fn(),
  clearData: jest.fn(),
  getNetworkInterfaces: jest.fn(),
  getActiveConnections: jest.fn(),
  getNetworkConfiguration: jest.fn(),
  setNetworkConfiguration: jest.fn(),
};

jest.mock('@corebase/nodejs', () => ({
  CoreBaseManager: jest.fn().mockImplementation(() => ({
    initialize: jest.fn().mockResolvedValue(undefined),
    shutdown: jest.fn().mockResolvedValue(undefined),
    getNetworkManager: jest.fn().mockReturnValue(mockNetworkManager),
  })),
}));

const TestWrapper: React.FC<{ children: React.ReactNode }> = ({ children }) => (
  <CoreBaseProvider config={{ autoInitialize: true }}>
    {children}
  </CoreBaseProvider>
);

describe('useNetworkManager', () => {
  beforeEach(() => {
    jest.clearAllMocks();
    jest.useFakeTimers();
  });

  afterEach(() => {
    jest.useRealTimers();
  });

  it('should return initial state', () => {
    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    expect(result.current.connectionStats).toBeNull();
    expect(result.current.bandwidthUsage).toBeNull();
    expect(result.current.connectionStatus).toBeNull();
    expect(result.current.isLoading).toBe(false);
    expect(result.current.error).toBeNull();
    expect(result.current.isMonitoring).toBe(false);
    expect(result.current.lastUpdated).toBeNull();
  });

  it('should return network manager functions', () => {
    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    expect(result.current.startMonitoring).toBeDefined();
    expect(result.current.stopMonitoring).toBeDefined();
    expect(result.current.refreshData).toBeDefined();
    expect(result.current.exportData).toBeDefined();
    expect(result.current.setRefreshInterval).toBeDefined();
    expect(result.current.clearData).toBeDefined();
    expect(result.current.getNetworkInterfaces).toBeDefined();
    expect(result.current.getActiveConnections).toBeDefined();
    expect(result.current.getNetworkConfiguration).toBeDefined();
    expect(result.current.setNetworkConfiguration).toBeDefined();
  });

  it('should start monitoring automatically when autoStart is true', () => {
    mockNetworkManager.isMonitoring.mockReturnValue(false);
    mockNetworkManager.startMonitoring.mockResolvedValue(undefined);

    renderHook(() => useNetworkManager({ autoStart: true }), {
      wrapper: TestWrapper,
    });

    expect(mockNetworkManager.startMonitoring).toHaveBeenCalled();
  });

  it('should not start monitoring automatically when autoStart is false', () => {
    mockNetworkManager.isMonitoring.mockReturnValue(false);

    renderHook(() => useNetworkManager({ autoStart: false }), {
      wrapper: TestWrapper,
    });

    expect(mockNetworkManager.startMonitoring).not.toHaveBeenCalled();
  });

  it('should start monitoring manually', async () => {
    mockNetworkManager.startMonitoring.mockResolvedValue(undefined);
    mockNetworkManager.isMonitoring.mockReturnValue(true);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.startMonitoring();
    });

    expect(mockNetworkManager.startMonitoring).toHaveBeenCalled();
  });

  it('should stop monitoring', async () => {
    mockNetworkManager.stopMonitoring.mockResolvedValue(undefined);
    mockNetworkManager.isMonitoring.mockReturnValue(false);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.stopMonitoring();
    });

    expect(mockNetworkManager.stopMonitoring).toHaveBeenCalled();
  });

  it('should refresh data manually', async () => {
    const mockConnectionStats = {
      totalConnections: 10,
      activeConnections: 5,
      failedConnections: 1,
      averageResponseTime: 150,
    };
    const mockBandwidthUsage = {
      downloadSpeed: 1000000,
      uploadSpeed: 500000,
      totalDownloaded: 1000000000,
      totalUploaded: 500000000,
    };
    const mockConnectionStatus = {
      isConnected: true,
      connectionType: 'ethernet',
      signalStrength: 100,
      ipAddress: '192.168.1.100',
    };

    mockNetworkManager.getConnectionStats.mockResolvedValue(mockConnectionStats);
    mockNetworkManager.getBandwidthUsage.mockResolvedValue(mockBandwidthUsage);
    mockNetworkManager.getConnectionStatus.mockResolvedValue(mockConnectionStatus);
    mockNetworkManager.refreshData.mockResolvedValue(undefined);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.refreshData();
    });

    expect(mockNetworkManager.refreshData).toHaveBeenCalled();
    expect(result.current.connectionStats).toEqual(mockConnectionStats);
    expect(result.current.bandwidthUsage).toEqual(mockBandwidthUsage);
    expect(result.current.connectionStatus).toEqual(mockConnectionStatus);
    expect(result.current.lastUpdated).not.toBeNull();
  });

  it('should handle refresh data errors', async () => {
    const error = new Error('Network refresh failed');
    mockNetworkManager.refreshData.mockRejectedValue(error);

    const onError = jest.fn();
    const { result } = renderHook(() => useNetworkManager({ onError }), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.refreshData();
    });

    expect(result.current.error).toBe(error);
    expect(onError).toHaveBeenCalledWith(error);
  });

  it('should export data', async () => {
    const mockExportData = {
      connectionStats: { totalConnections: 10 },
      bandwidthUsage: { downloadSpeed: 1000000 },
      connectionStatus: { isConnected: true },
      timestamp: new Date().toISOString(),
    };

    mockNetworkManager.exportData.mockResolvedValue(mockExportData);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const data = await result.current.exportData();
      expect(data).toEqual(mockExportData);
    });

    expect(mockNetworkManager.exportData).toHaveBeenCalled();
  });

  it('should export data with format', async () => {
    const mockExportData = 'csv,data,here';
    mockNetworkManager.exportData.mockResolvedValue(mockExportData);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const data = await result.current.exportData('csv');
      expect(data).toBe(mockExportData);
    });

    expect(mockNetworkManager.exportData).toHaveBeenCalledWith('csv');
  });

  it('should set refresh interval', async () => {
    mockNetworkManager.setRefreshInterval.mockResolvedValue(undefined);
    mockNetworkManager.getRefreshInterval.mockReturnValue(5000);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.setRefreshInterval(5000);
    });

    expect(mockNetworkManager.setRefreshInterval).toHaveBeenCalledWith(5000);
  });

  it('should clear data', async () => {
    mockNetworkManager.clearData.mockResolvedValue(undefined);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.clearData();
    });

    expect(mockNetworkManager.clearData).toHaveBeenCalled();
    expect(result.current.connectionStats).toBeNull();
    expect(result.current.bandwidthUsage).toBeNull();
    expect(result.current.connectionStatus).toBeNull();
  });

  it('should get network interfaces', async () => {
    const mockInterfaces = [
      {
        name: 'eth0',
        type: 'ethernet',
        status: 'up',
        ipAddress: '192.168.1.100',
        macAddress: '00:11:22:33:44:55',
      },
      {
        name: 'wlan0',
        type: 'wireless',
        status: 'down',
        ipAddress: null,
        macAddress: '66:77:88:99:AA:BB',
      },
    ];

    mockNetworkManager.getNetworkInterfaces.mockResolvedValue(mockInterfaces);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const interfaces = await result.current.getNetworkInterfaces();
      expect(interfaces).toEqual(mockInterfaces);
    });

    expect(mockNetworkManager.getNetworkInterfaces).toHaveBeenCalled();
  });

  it('should get active connections', async () => {
    const mockConnections = [
      {
        id: 'conn1',
        protocol: 'TCP',
        localAddress: '192.168.1.100:8080',
        remoteAddress: '93.184.216.34:443',
        status: 'ESTABLISHED',
        processName: 'chrome.exe',
      },
      {
        id: 'conn2',
        protocol: 'UDP',
        localAddress: '192.168.1.100:53',
        remoteAddress: '8.8.8.8:53',
        status: 'LISTENING',
        processName: 'dns.exe',
      },
    ];

    mockNetworkManager.getActiveConnections.mockResolvedValue(mockConnections);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const connections = await result.current.getActiveConnections();
      expect(connections).toEqual(mockConnections);
    });

    expect(mockNetworkManager.getActiveConnections).toHaveBeenCalled();
  });

  it('should get network configuration', async () => {
    const mockConfig = {
      dhcpEnabled: true,
      dnsServers: ['8.8.8.8', '8.8.4.4'],
      gateway: '192.168.1.1',
      subnet: '255.255.255.0',
      mtu: 1500,
    };

    mockNetworkManager.getNetworkConfiguration.mockResolvedValue(mockConfig);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const config = await result.current.getNetworkConfiguration();
      expect(config).toEqual(mockConfig);
    });

    expect(mockNetworkManager.getNetworkConfiguration).toHaveBeenCalled();
  });

  it('should set network configuration', async () => {
    const newConfig = {
      dhcpEnabled: false,
      staticIp: '192.168.1.150',
      dnsServers: ['1.1.1.1', '1.0.0.1'],
    };

    mockNetworkManager.setNetworkConfiguration.mockResolvedValue(undefined);

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.setNetworkConfiguration(newConfig);
    });

    expect(mockNetworkManager.setNetworkConfiguration).toHaveBeenCalledWith(newConfig);
  });

  it('should handle auto-refresh when enabled', async () => {
    const mockConnectionStats = { totalConnections: 10 };
    mockNetworkManager.getConnectionStats.mockResolvedValue(mockConnectionStats);
    mockNetworkManager.getBandwidthUsage.mockResolvedValue({});
    mockNetworkManager.getConnectionStatus.mockResolvedValue({});
    mockNetworkManager.refreshData.mockResolvedValue(undefined);
    mockNetworkManager.isMonitoring.mockReturnValue(true);

    const { result } = renderHook(() => useNetworkManager({ 
      autoRefresh: true, 
      refreshInterval: 1000 
    }), {
      wrapper: TestWrapper,
    });

    // Fast-forward time to trigger auto-refresh
    act(() => {
      jest.advanceTimersByTime(1000);
    });

    await act(async () => {
      await Promise.resolve(); // Wait for async operations
    });

    expect(mockNetworkManager.refreshData).toHaveBeenCalled();
  });

  it('should cleanup auto-refresh on unmount', () => {
    const { unmount } = renderHook(() => useNetworkManager({ 
      autoRefresh: true, 
      refreshInterval: 1000 
    }), {
      wrapper: TestWrapper,
    });

    const clearIntervalSpy = jest.spyOn(global, 'clearInterval');

    unmount();

    expect(clearIntervalSpy).toHaveBeenCalled();
    clearIntervalSpy.mockRestore();
  });

  it('should handle monitoring state changes', async () => {
    mockNetworkManager.isMonitoring.mockReturnValue(false);
    mockNetworkManager.startMonitoring.mockResolvedValue(undefined);

    const { result, rerender } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    expect(result.current.isMonitoring).toBe(false);

    // Simulate monitoring start
    mockNetworkManager.isMonitoring.mockReturnValue(true);

    await act(async () => {
      await result.current.startMonitoring();
    });

    rerender();

    expect(result.current.isMonitoring).toBe(true);
  });

  it('should handle loading states correctly', async () => {
    mockNetworkManager.refreshData.mockImplementation(() => 
      new Promise(resolve => setTimeout(resolve, 100))
    );

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    expect(result.current.isLoading).toBe(false);

    act(() => {
      result.current.refreshData();
    });

    expect(result.current.isLoading).toBe(true);

    await act(async () => {
      jest.advanceTimersByTime(100);
      await Promise.resolve();
    });

    expect(result.current.isLoading).toBe(false);
  });

  it('should throw error when used outside provider', () => {
    const consoleSpy = jest.spyOn(console, 'error').mockImplementation(() => {});

    expect(() => {
      renderHook(() => useNetworkManager());
    }).toThrow('useNetworkManager must be used within a CoreBaseProvider');

    consoleSpy.mockRestore();
  });

  it('should handle network manager not ready', async () => {
    const TestWrapperNotReady: React.FC<{ children: React.ReactNode }> = ({ children }) => (
      <CoreBaseProvider config={{ autoInitialize: false }}>
        {children}
      </CoreBaseProvider>
    );

    const { result } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapperNotReady,
    });

    // Should not throw, but functions should handle the case gracefully
    await act(async () => {
      try {
        await result.current.refreshData();
      } catch (e) {
        // Expected to fail when network manager is not ready
      }
    });
  });

  it('should maintain function references across re-renders', () => {
    const { result, rerender } = renderHook(() => useNetworkManager(), {
      wrapper: TestWrapper,
    });

    const firstRenderFunctions = {
      startMonitoring: result.current.startMonitoring,
      stopMonitoring: result.current.stopMonitoring,
      refreshData: result.current.refreshData,
      exportData: result.current.exportData,
      setRefreshInterval: result.current.setRefreshInterval,
      clearData: result.current.clearData,
      getNetworkInterfaces: result.current.getNetworkInterfaces,
      getActiveConnections: result.current.getActiveConnections,
      getNetworkConfiguration: result.current.getNetworkConfiguration,
      setNetworkConfiguration: result.current.setNetworkConfiguration,
    };

    rerender();

    // Functions should be memoized and maintain references
    expect(result.current.startMonitoring).toBe(firstRenderFunctions.startMonitoring);
    expect(result.current.stopMonitoring).toBe(firstRenderFunctions.stopMonitoring);
    expect(result.current.refreshData).toBe(firstRenderFunctions.refreshData);
    expect(result.current.exportData).toBe(firstRenderFunctions.exportData);
    expect(result.current.setRefreshInterval).toBe(firstRenderFunctions.setRefreshInterval);
    expect(result.current.clearData).toBe(firstRenderFunctions.clearData);
    expect(result.current.getNetworkInterfaces).toBe(firstRenderFunctions.getNetworkInterfaces);
    expect(result.current.getActiveConnections).toBe(firstRenderFunctions.getActiveConnections);
    expect(result.current.getNetworkConfiguration).toBe(firstRenderFunctions.getNetworkConfiguration);
    expect(result.current.setNetworkConfiguration).toBe(firstRenderFunctions.setNetworkConfiguration);
  });
});