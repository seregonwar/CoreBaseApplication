import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import { SystemMonitorDisplay } from '../components/SystemMonitorDisplay';
import { CoreBaseProvider } from '../contexts/CoreBaseContext';
import type { CoreBaseConfig } from '../types';

// Mock the useSystemMonitor hook
const mockUseSystemMonitor = {
  cpuInfo: {
    usage: 45.5,
    cores: 8,
    threads: 16,
    frequency: 3200,
    model: 'Intel Core i7-9700K'
  },
  memoryInfo: {
    usage: 65.2,
    total: 16777216000,
    used: 10737418240,
    available: 6039797760,
    free: 6039797760
  },
  diskInfo: [
    {
      device: 'C:',
      mountPoint: 'C:\\',
      usage: 75.3,
      total: 1000000000000,
      used: 753000000000,
      free: 247000000000,
      fileSystem: 'NTFS'
    }
  ],
  networkInfo: {
    bytesSent: 1048576,
    bytesReceived: 2097152,
    packetsSent: 1000,
    packetsReceived: 1500,
    errorsIn: 0,
    errorsOut: 0
  },
  processInfo: {
    pid: 1234,
    cpuUsage: 12.5,
    memoryUsage: 134217728,
    threads: 4,
    handles: 256,
    startTime: Date.now() - 3600000
  },
  loadInfo: {
    oneMinute: 1.2,
    fiveMinutes: 1.5,
    fifteenMinutes: 1.8
  },
  temperatureInfo: {
    cpu: 65,
    gpu: 70,
    motherboard: 45
  },
  performanceMetrics: {
    bootTime: 45,
    uptime: 86400,
    contextSwitches: 1000000,
    interrupts: 500000,
    systemCalls: 2000000
  },
  alerts: [
    {
      type: 'cpu',
      severity: 'warning' as const,
      message: 'High CPU usage detected',
      timestamp: Date.now(),
      value: 95.5,
      threshold: 90
    }
  ],
  isMonitoring: true,
  error: null,
  startMonitoring: jest.fn(),
  stopMonitoring: jest.fn(),
  refreshData: jest.fn(),
  setMonitoringInterval: jest.fn(),
  exportData: jest.fn()
};

jest.mock('../hooks/useSystemMonitor', () => ({
  useSystemMonitor: jest.fn(() => mockUseSystemMonitor)
}));

// Mock the nodejs bindings
jest.mock('@corebase/nodejs', () => ({
  CoreBaseManager: jest.fn().mockImplementation(() => ({
    initialize: jest.fn().mockResolvedValue(undefined),
    shutdown: jest.fn().mockResolvedValue(undefined),
    getPlatformInfo: jest.fn().mockReturnValue({}),
    getVersion: jest.fn().mockReturnValue('1.0.0'),
    getErrorHandler: jest.fn().mockReturnValue({}),
    getConfigManager: jest.fn().mockReturnValue({}),
    getSystemMonitor: jest.fn().mockReturnValue({}),
    getNetworkManager: jest.fn().mockReturnValue({})
  }))
}));

describe('SystemMonitorDisplay', () => {
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

  it('should render system monitor display with all metrics', () => {
    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    expect(screen.getByText('System Monitor')).toBeInTheDocument();
    expect(screen.getByText('CPU Information')).toBeInTheDocument();
    expect(screen.getByText('Memory Information')).toBeInTheDocument();
    expect(screen.getByText('Disk Information')).toBeInTheDocument();
    expect(screen.getByText('Network Information')).toBeInTheDocument();
    expect(screen.getByText('Process Information')).toBeInTheDocument();
    expect(screen.getByText('System Load')).toBeInTheDocument();
    expect(screen.getByText('Temperature Information')).toBeInTheDocument();
    expect(screen.getByText('Performance Metrics')).toBeInTheDocument();
  });

  it('should display CPU information correctly', () => {
    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    expect(screen.getByText('45.5%')).toBeInTheDocument(); // CPU usage
    expect(screen.getByText('8')).toBeInTheDocument(); // CPU cores
    expect(screen.getByText('16')).toBeInTheDocument(); // CPU threads
    expect(screen.getByText('3200 MHz')).toBeInTheDocument(); // CPU frequency
    expect(screen.getByText('Intel Core i7-9700K')).toBeInTheDocument(); // CPU model
  });

  it('should display memory information with formatted bytes', () => {
    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    expect(screen.getByText('65.2%')).toBeInTheDocument(); // Memory usage
    expect(screen.getByText('15.62 GB')).toBeInTheDocument(); // Total memory
    expect(screen.getByText('10 GB')).toBeInTheDocument(); // Used memory
    expect(screen.getByText('5.62 GB')).toBeInTheDocument(); // Available memory
  });

  it('should display disk information for multiple disks', () => {
    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    expect(screen.getByText('C: (C:\\)')).toBeInTheDocument();
    expect(screen.getByText('75.3%')).toBeInTheDocument(); // Disk usage
    expect(screen.getByText('931.32 GB')).toBeInTheDocument(); // Total disk space
    expect(screen.getByText('NTFS')).toBeInTheDocument(); // File system
  });

  it('should display alerts when present', () => {
    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    expect(screen.getByText('Alerts')).toBeInTheDocument();
    expect(screen.getByText('cpu:')).toBeInTheDocument();
    expect(screen.getByText('High CPU usage detected')).toBeInTheDocument();
  });

  it('should handle start/stop monitoring buttons', () => {
    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    const stopButton = screen.getByText('Stop Monitoring');
    expect(stopButton).toBeInTheDocument();

    fireEvent.click(stopButton);
    expect(mockUseSystemMonitor.stopMonitoring).toHaveBeenCalled();
  });

  it('should handle refresh button', () => {
    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    const refreshButton = screen.getByText('Refresh');
    fireEvent.click(refreshButton);
    expect(mockUseSystemMonitor.refreshData).toHaveBeenCalled();
  });

  it('should show start button when not monitoring', () => {
    const mockNotMonitoring = {
      ...mockUseSystemMonitor,
      isMonitoring: false
    };

    const { useSystemMonitor } = require('../hooks/useSystemMonitor');
    useSystemMonitor.mockReturnValue(mockNotMonitoring);

    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    const startButton = screen.getByText('Start Monitoring');
    expect(startButton).toBeInTheDocument();

    fireEvent.click(startButton);
    expect(mockUseSystemMonitor.startMonitoring).toHaveBeenCalled();
  });

  it('should display error state when error occurs', () => {
    const mockWithError = {
      ...mockUseSystemMonitor,
      error: 'System monitor initialization failed'
    };

    const { useSystemMonitor } = require('../hooks/useSystemMonitor');
    useSystemMonitor.mockReturnValue(mockWithError);

    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    expect(screen.getByText('System Monitor Error')).toBeInTheDocument();
    expect(screen.getByText('System monitor initialization failed')).toBeInTheDocument();
    expect(screen.getByText('Retry')).toBeInTheDocument();
  });

  it('should handle retry button in error state', () => {
    const mockWithError = {
      ...mockUseSystemMonitor,
      error: 'Test error'
    };

    const { useSystemMonitor } = require('../hooks/useSystemMonitor');
    useSystemMonitor.mockReturnValue(mockWithError);

    render(
      <SystemMonitorDisplay />,
      { wrapper }
    );

    const retryButton = screen.getByText('Retry');
    fireEvent.click(retryButton);
    expect(mockUseSystemMonitor.refreshData).toHaveBeenCalled();
  });

  it('should hide sections based on props', () => {
    render(
      <SystemMonitorDisplay
        showCpu={false}
        showMemory={false}
        showAlerts={false}
      />,
      { wrapper }
    );

    expect(screen.queryByText('CPU Information')).not.toBeInTheDocument();
    expect(screen.queryByText('Memory Information')).not.toBeInTheDocument();
    expect(screen.queryByText('Alerts')).not.toBeInTheDocument();
    
    // Other sections should still be visible
    expect(screen.getByText('Disk Information')).toBeInTheDocument();
    expect(screen.getByText('Network Information')).toBeInTheDocument();
  });

  it('should apply custom className and style', () => {
    const customStyle = { backgroundColor: 'red' };
    const customClassName = 'custom-monitor';

    render(
      <SystemMonitorDisplay
        className={customClassName}
        style={customStyle}
      />,
      { wrapper }
    );

    const container = screen.getByText('System Monitor').closest('.system-monitor-display');
    expect(container).toHaveClass('system-monitor-display', customClassName);
    expect(container).toHaveStyle(customStyle);
  });

  it('should update monitoring interval when refreshInterval prop changes', async () => {
    const { rerender } = render(
      <SystemMonitorDisplay refreshInterval={1000} />,
      { wrapper }
    );

    rerender(
      <SystemMonitorDisplay refreshInterval={2000} />
    );

    await waitFor(() => {
      expect(mockUseSystemMonitor.setMonitoringInterval).toHaveBeenCalledWith(2000);
    });
  });

  it('should call onAlert callback when provided', () => {
    const onAlert = jest.fn();

    render(
      <SystemMonitorDisplay onAlert={onAlert} />,
      { wrapper }
    );

    // The onAlert should be passed to useSystemMonitor
    const { useSystemMonitor } = require('../hooks/useSystemMonitor');
    const lastCall = useSystemMonitor.mock.calls[useSystemMonitor.mock.calls.length - 1];
    expect(lastCall[0]).toEqual(expect.objectContaining({ onAlert }));
  });

  it('should call onError callback when provided', () => {
    const onError = jest.fn();

    render(
      <SystemMonitorDisplay onError={onError} />,
      { wrapper }
    );

    // The onError should be passed to useSystemMonitor
    const { useSystemMonitor } = require('../hooks/useSystemMonitor');
    const lastCall = useSystemMonitor.mock.calls[useSystemMonitor.mock.calls.length - 1];
    expect(lastCall[0]).toEqual(expect.objectContaining({ onError }));
  });
});