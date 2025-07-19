import React from 'react';
import { useNetworkManager } from '../hooks/useNetworkManager';
import type { NetworkStatsDisplayProps } from '../types';

/**
 * Component for displaying network statistics and connection information
 */
export const NetworkStatsDisplay: React.FC<NetworkStatsDisplayProps> = ({
  showConnectionStats = true,
  showBandwidthUsage = true,
  showConnectionStatus = true,
  refreshInterval = 2000,
  className,
  style,
  onError
}) => {
  const {
    connectionStats,
    bandwidthUsage,
    isConnected,
    error,
    refreshStats
  } = useNetworkManager({
    onError
  });

  React.useEffect(() => {
    const interval = setInterval(() => {
      refreshStats();
    }, refreshInterval);

    return () => clearInterval(interval);
  }, [refreshInterval, refreshStats]);

  const formatBytes = (bytes: number): string => {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const formatBytesPerSecond = (bytesPerSecond: number): string => {
    return `${formatBytes(bytesPerSecond)}/s`;
  };

  const formatDuration = (milliseconds: number): string => {
    const seconds = Math.floor(milliseconds / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    
    if (hours > 0) {
      return `${hours}h ${minutes % 60}m ${seconds % 60}s`;
    } else if (minutes > 0) {
      return `${minutes}m ${seconds % 60}s`;
    } else {
      return `${seconds}s`;
    }
  };

  if (error) {
    return (
      <div className={`network-stats-error ${className || ''}`} style={style}>
        <h3>Network Stats Error</h3>
        <p>{error}</p>
        <button onClick={refreshStats}>Retry</button>
      </div>
    );
  }

  return (
    <div className={`network-stats-display ${className || ''}`} style={style}>
      <div className="network-stats-header">
        <h2>Network Statistics</h2>
        <div className="stats-controls">
          <button onClick={refreshStats}>
            Refresh
          </button>
        </div>
      </div>

      {showConnectionStatus && (
        <div className="connection-status">
          <div className={`status-indicator ${isConnected ? 'connected' : 'disconnected'}`}>
            <span className="status-dot"></span>
            <span className="status-text">
              {isConnected ? 'Connected' : 'Disconnected'}
            </span>
          </div>
        </div>
      )}

      <div className="network-metrics">
        {showConnectionStats && connectionStats && (
          <div className="metric-card connection-stats">
            <h3>Connection Statistics</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>Active Connections:</label>
                <span>{connectionStats.activeConnections}</span>
              </div>
              <div className="metric-item">
                <label>Total Connections:</label>
                <span>{connectionStats.totalConnections}</span>
              </div>
              <div className="metric-item">
                <label>Failed Connections:</label>
                <span>{connectionStats.failedConnections}</span>
              </div>
              <div className="metric-item">
                <label>Success Rate:</label>
                <span>
                  {connectionStats.totalConnections > 0
                    ? ((connectionStats.totalConnections - connectionStats.failedConnections) / connectionStats.totalConnections * 100).toFixed(1)
                    : 0}%
                </span>
              </div>
              <div className="metric-item">
                <label>Average Response Time:</label>
                <span>{connectionStats.averageResponseTime}ms</span>
              </div>
              <div className="metric-item">
                <label>Timeout Count:</label>
                <span>{connectionStats.timeoutCount}</span>
              </div>
              <div className="metric-item">
                <label>Retry Count:</label>
                <span>{connectionStats.retryCount}</span>
              </div>
              <div className="metric-item">
                <label>Last Error:</label>
                <span>{connectionStats.lastError || 'None'}</span>
              </div>
            </div>
          </div>
        )}

        {showBandwidthUsage && bandwidthUsage && (
          <div className="metric-card bandwidth-usage">
            <h3>Bandwidth Usage</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>Upload Speed:</label>
                <span>{formatBytesPerSecond(bandwidthUsage.uploadSpeed)}</span>
              </div>
              <div className="metric-item">
                <label>Download Speed:</label>
                <span>{formatBytesPerSecond(bandwidthUsage.downloadSpeed)}</span>
              </div>
              <div className="metric-item">
                <label>Total Uploaded:</label>
                <span>{formatBytes(bandwidthUsage.totalUploaded)}</span>
              </div>
              <div className="metric-item">
                <label>Total Downloaded:</label>
                <span>{formatBytes(bandwidthUsage.totalDownloaded)}</span>
              </div>
              <div className="metric-item">
                <label>Peak Upload:</label>
                <span>{formatBytesPerSecond(bandwidthUsage.peakUploadSpeed)}</span>
              </div>
              <div className="metric-item">
                <label>Peak Download:</label>
                <span>{formatBytesPerSecond(bandwidthUsage.peakDownloadSpeed)}</span>
              </div>
              <div className="metric-item">
                <label>Session Duration:</label>
                <span>{formatDuration(bandwidthUsage.sessionDuration)}</span>
              </div>
              <div className="metric-item">
                <label>Data Efficiency:</label>
                <span>
                  {bandwidthUsage.totalDownloaded > 0
                    ? ((bandwidthUsage.totalDownloaded - bandwidthUsage.totalUploaded) / bandwidthUsage.totalDownloaded * 100).toFixed(1)
                    : 0}%
                </span>
              </div>
            </div>
          </div>
        )}
      </div>

      <div className="network-charts">
        {/* Placeholder for future chart implementations */}
        <div className="chart-placeholder">
          <p>Real-time charts will be implemented here</p>
          <small>Consider integrating with Chart.js or similar library</small>
        </div>
      </div>
    </div>
  );
};

export default NetworkStatsDisplay;