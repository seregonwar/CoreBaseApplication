import React from 'react';
import { render, screen, fireEvent, waitFor, act } from '@testing-library/react';
import { jest } from '@jest/globals';

import { NetworkStatsDisplay } from '../components/NetworkStatsDisplay';
import { CoreBaseProvider } from '../contexts/CoreBaseContext';
import * as useNetworkManagerModule from '../hooks/useNetworkManager';

// Mock the useNetworkManager hook
const mockUseNetworkManager = jest.fn();
jest.mock('../hooks/useNetworkManager', () => ({
  useNetworkManager: mockUseNetworkManager,
}));

// Mock @corebase/nodejs
jest.mock('@corebase/nodejs', () => ({
  CoreBaseManager: jest.fn().mockImplementation(() => ({
    initialize: jest.fn().mockResolvedValue(undefined),
    shutdown: jest.fn().mockResolvedValue(undefined),
    getNetworkManager: jest.fn().mockReturnValue({
      getConnectionStats: jest.fn().mockResolvedValue({
        activeConnections: 12,
        totalConnections: 156,
        bytesSent: 1024000,
        bytesReceived: 2048000,
        packetsDropped: 0,
        errors: 0,
      }),
      getBandwidthUsage: jest.fn().mockResolvedValue({
        upload: 1024,
        download: 2048,
        total: 3072,
      }),
      isConnected: jest.fn().mockReturnValue(true),
      refreshStats: jest.fn().mockResolvedValue(undefined),
    }),
  })),
}));

const TestWrapper: React.FC<{ children: React.ReactNode }> = ({ children }) => (
  <CoreBaseProvider config={{ autoInitialize: true }}>
    {children}
  </CoreBaseProvider>
);

const mockConnectionStats = {
  activeConnections: 12,
  totalConnections: 156,
  bytesSent: 1024000,
  bytesReceived: 2048000,
  packetsDropped: 0,
  errors: 0,
};

const mockBandwidthUsage = {
  upload: 1024,
  download: 2048,
  total: 3072,
};

const defaultMockReturn = {
  connectionStats: mockConnectionStats,
  bandwidthUsage: mockBandwidthUsage,
  isConnected: true,
  isLoading: false,
  error: null,
  refreshStats: jest.fn(),
};

describe('NetworkStatsDisplay', () => {
  beforeEach(() => {
    jest.clearAllMocks();
    mockUseNetworkManager.mockReturnValue(defaultMockReturn);
  });

  afterEach(() => {
    jest.clearAllTimers();
  });

  it('renders with all network stats', () => {
    render(
      <TestWrapper>
        <NetworkStatsDisplay />
      </TestWrapper>
    );

    expect(screen.getByText('Network Statistics')).toBeInTheDocument();
    expect(screen.getByText('Connection Statistics')).toBeInTheDocument();
    expect(screen.getByText('Bandwidth Usage')).toBeInTheDocument();
    expect(screen.getByText('Connection Status')).toBeInTheDocument();
  });

  it('displays connection statistics correctly', () => {
    render(
      <TestWrapper>
        <NetworkStatsDisplay showConnectionStats={true} />
      </TestWrapper>
    );

    expect(screen.getByText('Active Connections:')).toBeInTheDocument();
    expect(screen.getByText('12')).toBeInTheDocument();
    expect(screen.getByText('Total Connections:')).toBeInTheDocument();
    expect(screen.getByText('156')).toBeInTheDocument();
    expect(screen.getByText('Bytes Sent:')).toBeInTheDocument();
    expect(screen.getByText('1.00 MB')).toBeInTheDocument();
    expect(screen.getByText('Bytes Received:')).toBeInTheDocument();
    expect(screen.getByText('2.00 MB')).toBeInTheDocument();
  });

  it('displays bandwidth usage correctly', () => {
    render(
      <TestWrapper>
        <NetworkStatsDisplay showBandwidthUsage={true} />
      </TestWrapper>
    );

    expect(screen.getByText('Upload:')).toBeInTheDocument();
    expect(screen.getByText('1.00 KB/s')).toBeInTheDocument();
    expect(screen.getByText('Download:')).toBeInTheDocument();
    expect(screen.getByText('2.00 KB/s')).toBeInTheDocument();
    expect(screen.getByText('Total:')).toBeInTheDocument();
    expect(screen.getByText('3.00 KB/s')).toBeInTheDocument();
  });

  it('displays connection status correctly', () => {
    render(
      <TestWrapper>
        <NetworkStatsDisplay showConnectionStatus={true} />
      </TestWrapper>
    );

    expect(screen.getByText('Status:')).toBeInTheDocument();
    expect(screen.getByText('Connected')).toBeInTheDocument();
    expect(screen.getByText('Connected')).toHaveStyle('color: #28a745');
  });

  it('shows disconnected status when not connected', () => {
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      isConnected: false,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay showConnectionStatus={true} />
      </TestWrapper>
    );

    expect(screen.getByText('Disconnected')).toBeInTheDocument();
    expect(screen.getByText('Disconnected')).toHaveStyle('color: #dc3545');
  });

  it('handles refresh button click', async () => {
    const mockRefreshStats = jest.fn();
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      refreshStats: mockRefreshStats,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay />
      </TestWrapper>
    );

    const refreshButton = screen.getByText('Refresh');
    fireEvent.click(refreshButton);

    await waitFor(() => {
      expect(mockRefreshStats).toHaveBeenCalledTimes(1);
    });
  });

  it('displays loading state', () => {
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      isLoading: true,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay />
      </TestWrapper>
    );

    expect(screen.getByText('Loading network stats...')).toBeInTheDocument();
  });

  it('displays error state', () => {
    const mockError = new Error('Network error');
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      error: mockError,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay />
      </TestWrapper>
    );

    expect(screen.getByText('Error loading network stats:')).toBeInTheDocument();
    expect(screen.getByText('Network error')).toBeInTheDocument();
  });

  it('handles retry button click', async () => {
    const mockRefreshStats = jest.fn();
    const mockError = new Error('Network error');
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      error: mockError,
      refreshStats: mockRefreshStats,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay />
      </TestWrapper>
    );

    const retryButton = screen.getByText('Retry');
    fireEvent.click(retryButton);

    await waitFor(() => {
      expect(mockRefreshStats).toHaveBeenCalledTimes(1);
    });
  });

  it('hides sections based on props', () => {
    render(
      <TestWrapper>
        <NetworkStatsDisplay
          showConnectionStats={false}
          showBandwidthUsage={false}
          showConnectionStatus={true}
        />
      </TestWrapper>
    );

    expect(screen.queryByText('Connection Statistics')).not.toBeInTheDocument();
    expect(screen.queryByText('Bandwidth Usage')).not.toBeInTheDocument();
    expect(screen.getByText('Connection Status')).toBeInTheDocument();
  });

  it('applies custom className and style', () => {
    const customStyle = { backgroundColor: 'red' };
    
    render(
      <TestWrapper>
        <NetworkStatsDisplay
          className="custom-network-stats"
          style={customStyle}
        />
      </TestWrapper>
    );

    const container = screen.getByText('Network Statistics').closest('div');
    expect(container).toHaveClass('custom-network-stats');
    expect(container).toHaveStyle('background-color: red');
  });

  it('calls onError callback when error occurs', () => {
    const mockOnError = jest.fn();
    const mockError = new Error('Network error');
    
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      error: mockError,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay onError={mockOnError} />
      </TestWrapper>
    );

    expect(mockOnError).toHaveBeenCalledWith(mockError);
  });

  it('handles auto refresh', async () => {
    jest.useFakeTimers();
    const mockRefreshStats = jest.fn();
    
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      refreshStats: mockRefreshStats,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay autoRefresh={true} refreshInterval={5000} />
      </TestWrapper>
    );

    // Fast-forward time
    act(() => {
      jest.advanceTimersByTime(5000);
    });

    await waitFor(() => {
      expect(mockRefreshStats).toHaveBeenCalled();
    });

    jest.useRealTimers();
  });

  it('does not auto refresh when autoRefresh is false', async () => {
    jest.useFakeTimers();
    const mockRefreshStats = jest.fn();
    
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      refreshStats: mockRefreshStats,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay autoRefresh={false} refreshInterval={5000} />
      </TestWrapper>
    );

    // Fast-forward time
    act(() => {
      jest.advanceTimersByTime(10000);
    });

    expect(mockRefreshStats).not.toHaveBeenCalled();

    jest.useRealTimers();
  });

  it('formats bytes correctly', () => {
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      connectionStats: {
        ...mockConnectionStats,
        bytesSent: 1536, // 1.5 KB
        bytesReceived: 1048576, // 1 MB
      },
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay showConnectionStats={true} />
      </TestWrapper>
    );

    expect(screen.getByText('1.50 KB')).toBeInTheDocument();
    expect(screen.getByText('1.00 MB')).toBeInTheDocument();
  });

  it('handles missing data gracefully', () => {
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      connectionStats: null,
      bandwidthUsage: null,
    });

    render(
      <TestWrapper>
        <NetworkStatsDisplay />
      </TestWrapper>
    );

    expect(screen.getByText('Network Statistics')).toBeInTheDocument();
    expect(screen.getByText('No connection data available')).toBeInTheDocument();
    expect(screen.getByText('No bandwidth data available')).toBeInTheDocument();
  });

  it('cleans up auto refresh on unmount', () => {
    jest.useFakeTimers();
    const mockRefreshStats = jest.fn();
    
    mockUseNetworkManager.mockReturnValue({
      ...defaultMockReturn,
      refreshStats: mockRefreshStats,
    });

    const { unmount } = render(
      <TestWrapper>
        <NetworkStatsDisplay autoRefresh={true} refreshInterval={5000} />
      </TestWrapper>
    );

    unmount();

    // Fast-forward time after unmount
    act(() => {
      jest.advanceTimersByTime(10000);
    });

    // Should not call refresh after unmount
    expect(mockRefreshStats).not.toHaveBeenCalled();

    jest.useRealTimers();
  });
});