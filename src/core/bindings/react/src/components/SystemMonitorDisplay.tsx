import React from 'react';
import { useSystemMonitor } from '../hooks/useSystemMonitor';
import type { SystemMonitorDisplayProps } from '../types';

/**
 * Component for displaying system monitoring information
 */
export const SystemMonitorDisplay: React.FC<SystemMonitorDisplayProps> = ({
  showCpu = true,
  showMemory = true,
  showDisk = true,
  showNetwork = true,
  showProcess = true,
  showLoad = true,
  showTemperature = true,
  showPerformance = true,
  showAlerts = true,
  refreshInterval = 1000,
  className,
  style,
  onAlert,
  onError
}) => {
  const {
    cpuInfo,
    memoryInfo,
    diskInfo,
    networkInfo,
    processInfo,
    loadInfo,
    temperatureInfo,
    performanceMetrics,
    alerts,
    isMonitoring,
    error,
    startMonitoring,
    stopMonitoring,
    refreshData,
    setMonitoringInterval
  } = useSystemMonitor({
    autoStart: true,
    refreshInterval,
    onAlert,
    onError
  });

  React.useEffect(() => {
    setMonitoringInterval(refreshInterval);
  }, [refreshInterval, setMonitoringInterval]);

  const formatBytes = (bytes: number): string => {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const formatPercentage = (value: number): string => {
    return `${value.toFixed(1)}%`;
  };

  if (error) {
    return (
      <div className={`system-monitor-error ${className || ''}`} style={style}>
        <h3>System Monitor Error</h3>
        <p>{error}</p>
        <button onClick={refreshData}>Retry</button>
      </div>
    );
  }

  return (
    <div className={`system-monitor-display ${className || ''}`} style={style}>
      <div className="system-monitor-header">
        <h2>System Monitor</h2>
        <div className="monitor-controls">
          <button
            onClick={isMonitoring ? stopMonitoring : startMonitoring}
            className={isMonitoring ? 'stop-btn' : 'start-btn'}
          >
            {isMonitoring ? 'Stop' : 'Start'} Monitoring
          </button>
          <button onClick={refreshData} disabled={!isMonitoring}>
            Refresh
          </button>
        </div>
      </div>

      {showAlerts && alerts.length > 0 && (
        <div className="system-alerts">
          <h3>Alerts</h3>
          {alerts.map((alert, index) => (
            <div key={index} className={`alert alert-${alert.severity}`}>
              <strong>{alert.type}:</strong> {alert.message}
              <small>{new Date(alert.timestamp).toLocaleString()}</small>
            </div>
          ))}
        </div>
      )}

      <div className="system-metrics">
        {showCpu && cpuInfo && (
          <div className="metric-card cpu-info">
            <h3>CPU Information</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>Usage:</label>
                <span>{formatPercentage(cpuInfo.usage)}</span>
              </div>
              <div className="metric-item">
                <label>Cores:</label>
                <span>{cpuInfo.cores}</span>
              </div>
              <div className="metric-item">
                <label>Threads:</label>
                <span>{cpuInfo.threads}</span>
              </div>
              <div className="metric-item">
                <label>Frequency:</label>
                <span>{cpuInfo.frequency} MHz</span>
              </div>
              <div className="metric-item">
                <label>Model:</label>
                <span>{cpuInfo.model}</span>
              </div>
            </div>
          </div>
        )}

        {showMemory && memoryInfo && (
          <div className="metric-card memory-info">
            <h3>Memory Information</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>Usage:</label>
                <span>{formatPercentage(memoryInfo.usage)}</span>
              </div>
              <div className="metric-item">
                <label>Total:</label>
                <span>{formatBytes(memoryInfo.total)}</span>
              </div>
              <div className="metric-item">
                <label>Used:</label>
                <span>{formatBytes(memoryInfo.used)}</span>
              </div>
              <div className="metric-item">
                <label>Available:</label>
                <span>{formatBytes(memoryInfo.available)}</span>
              </div>
              <div className="metric-item">
                <label>Free:</label>
                <span>{formatBytes(memoryInfo.free)}</span>
              </div>
            </div>
          </div>
        )}

        {showDisk && diskInfo && diskInfo.length > 0 && (
          <div className="metric-card disk-info">
            <h3>Disk Information</h3>
            {diskInfo.map((disk, index) => (
              <div key={index} className="disk-item">
                <h4>{disk.device} ({disk.mountPoint})</h4>
                <div className="metric-grid">
                  <div className="metric-item">
                    <label>Usage:</label>
                    <span>{formatPercentage(disk.usage)}</span>
                  </div>
                  <div className="metric-item">
                    <label>Total:</label>
                    <span>{formatBytes(disk.total)}</span>
                  </div>
                  <div className="metric-item">
                    <label>Used:</label>
                    <span>{formatBytes(disk.used)}</span>
                  </div>
                  <div className="metric-item">
                    <label>Free:</label>
                    <span>{formatBytes(disk.free)}</span>
                  </div>
                  <div className="metric-item">
                    <label>Type:</label>
                    <span>{disk.fileSystem}</span>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}

        {showNetwork && networkInfo && (
          <div className="metric-card network-info">
            <h3>Network Information</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>Bytes Sent:</label>
                <span>{formatBytes(networkInfo.bytesSent)}</span>
              </div>
              <div className="metric-item">
                <label>Bytes Received:</label>
                <span>{formatBytes(networkInfo.bytesReceived)}</span>
              </div>
              <div className="metric-item">
                <label>Packets Sent:</label>
                <span>{networkInfo.packetsSent.toLocaleString()}</span>
              </div>
              <div className="metric-item">
                <label>Packets Received:</label>
                <span>{networkInfo.packetsReceived.toLocaleString()}</span>
              </div>
              <div className="metric-item">
                <label>Errors In:</label>
                <span>{networkInfo.errorsIn}</span>
              </div>
              <div className="metric-item">
                <label>Errors Out:</label>
                <span>{networkInfo.errorsOut}</span>
              </div>
            </div>
          </div>
        )}

        {showProcess && processInfo && (
          <div className="metric-card process-info">
            <h3>Process Information</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>PID:</label>
                <span>{processInfo.pid}</span>
              </div>
              <div className="metric-item">
                <label>CPU Usage:</label>
                <span>{formatPercentage(processInfo.cpuUsage)}</span>
              </div>
              <div className="metric-item">
                <label>Memory Usage:</label>
                <span>{formatBytes(processInfo.memoryUsage)}</span>
              </div>
              <div className="metric-item">
                <label>Threads:</label>
                <span>{processInfo.threads}</span>
              </div>
              <div className="metric-item">
                <label>Handles:</label>
                <span>{processInfo.handles}</span>
              </div>
              <div className="metric-item">
                <label>Start Time:</label>
                <span>{new Date(processInfo.startTime).toLocaleString()}</span>
              </div>
            </div>
          </div>
        )}

        {showLoad && loadInfo && (
          <div className="metric-card load-info">
            <h3>System Load</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>1 minute:</label>
                <span>{loadInfo.oneMinute.toFixed(2)}</span>
              </div>
              <div className="metric-item">
                <label>5 minutes:</label>
                <span>{loadInfo.fiveMinutes.toFixed(2)}</span>
              </div>
              <div className="metric-item">
                <label>15 minutes:</label>
                <span>{loadInfo.fifteenMinutes.toFixed(2)}</span>
              </div>
            </div>
          </div>
        )}

        {showTemperature && temperatureInfo && (
          <div className="metric-card temperature-info">
            <h3>Temperature Information</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>CPU:</label>
                <span>{temperatureInfo.cpu}°C</span>
              </div>
              <div className="metric-item">
                <label>GPU:</label>
                <span>{temperatureInfo.gpu}°C</span>
              </div>
              <div className="metric-item">
                <label>Motherboard:</label>
                <span>{temperatureInfo.motherboard}°C</span>
              </div>
            </div>
          </div>
        )}

        {showPerformance && performanceMetrics && (
          <div className="metric-card performance-info">
            <h3>Performance Metrics</h3>
            <div className="metric-grid">
              <div className="metric-item">
                <label>Boot Time:</label>
                <span>{performanceMetrics.bootTime} seconds</span>
              </div>
              <div className="metric-item">
                <label>Uptime:</label>
                <span>{Math.floor(performanceMetrics.uptime / 3600)} hours</span>
              </div>
              <div className="metric-item">
                <label>Context Switches:</label>
                <span>{performanceMetrics.contextSwitches.toLocaleString()}</span>
              </div>
              <div className="metric-item">
                <label>Interrupts:</label>
                <span>{performanceMetrics.interrupts.toLocaleString()}</span>
              </div>
              <div className="metric-item">
                <label>System Calls:</label>
                <span>{performanceMetrics.systemCalls.toLocaleString()}</span>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default SystemMonitorDisplay;