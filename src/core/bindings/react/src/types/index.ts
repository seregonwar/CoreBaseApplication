import { ReactNode } from 'react';
import {
  ErrorHandler,
  ConfigManager,
  SystemMonitor,
  NetworkManager,
  LogLevel,
  CpuInfo,
  MemoryInfo,
  DiskInfo,
  NetworkInfo,
  ProcessInfo,
  LoadInfo,
  TemperatureInfo,
  PerformanceMetrics,
  HistoricalData,
  AlertInfo,
  ConnectionStats,
  BandwidthUsage,
  HttpResponse,
  HttpRequest,
  WebSocketMessage,
  TcpServerInfo,
  UdpServerInfo,
  PlatformInfo
} from '@corebase/nodejs';

// Re-export types from Node.js bindings
export {
  ErrorHandler,
  ConfigManager,
  SystemMonitor,
  NetworkManager,
  LogLevel,
  CpuInfo,
  MemoryInfo,
  DiskInfo,
  NetworkInfo,
  ProcessInfo,
  LoadInfo,
  TemperatureInfo,
  PerformanceMetrics,
  HistoricalData,
  AlertInfo,
  ConnectionStats,
  BandwidthUsage,
  HttpResponse,
  HttpRequest,
  WebSocketMessage,
  TcpServerInfo,
  UdpServerInfo,
  PlatformInfo
};

// React-specific types
export interface CoreBaseProviderProps {
  children: ReactNode;
  config?: CoreBaseConfig;
  onError?: (error: Error) => void;
  onInitialized?: () => void;
}

export interface CoreBaseConfig {
  logLevel?: LogLevel;
  monitoringInterval?: number;
  enableSystemMonitoring?: boolean;
  enableNetworkMonitoring?: boolean;
  autoStart?: boolean;
  errorReporting?: boolean;
}

export interface CoreBaseContextValue {
  errorHandler: ErrorHandler | null;
  configManager: ConfigManager | null;
  systemMonitor: SystemMonitor | null;
  networkManager: NetworkManager | null;
  isInitialized: boolean;
  isLoading: boolean;
  error: Error | null;
  platformInfo: PlatformInfo | null;
  version: string | null;
}

// Hook return types
export interface UseErrorHandlerReturn {
  errorHandler: ErrorHandler | null;
  logLevel: LogLevel;
  setLogLevel: (level: LogLevel) => Promise<void>;
  logError: (message: string, details?: string) => Promise<void>;
  logWarning: (message: string, details?: string) => Promise<void>;
  logInfo: (message: string, details?: string) => Promise<void>;
  logDebug: (message: string, details?: string) => Promise<void>;
  handleError: (error: Error) => Promise<void>;
  handleException: (exception: string) => Promise<void>;
  isInitialized: boolean;
}

export interface UseConfigManagerReturn {
  configManager: ConfigManager | null;
  getValue: <T = any>(key: string, defaultValue?: T) => Promise<T>;
  setValue: (key: string, value: any) => Promise<void>;
  hasKey: (key: string) => Promise<boolean>;
  removeKey: (key: string) => Promise<void>;
  validateConfig: (config: Record<string, any>) => Promise<boolean>;
  loadConfig: (filePath: string) => Promise<void>;
  saveConfig: (filePath: string) => Promise<void>;
  isInitialized: boolean;
}

export interface UseSystemMonitorReturn {
  systemMonitor: SystemMonitor | null;
  isMonitoring: boolean;
  cpuInfo: CpuInfo | null;
  memoryInfo: MemoryInfo | null;
  diskInfo: DiskInfo[] | null;
  networkInfo: NetworkInfo[] | null;
  processInfo: ProcessInfo[] | null;
  loadInfo: LoadInfo | null;
  temperatureInfo: TemperatureInfo | null;
  performanceMetrics: PerformanceMetrics | null;
  alerts: AlertInfo[] | null;
  startMonitoring: () => Promise<void>;
  stopMonitoring: () => Promise<void>;
  refreshData: () => Promise<void>;
  setMonitoringInterval: (interval: number) => Promise<void>;
  exportData: (format: string, filePath: string) => Promise<void>;
  isInitialized: boolean;
}

export interface UseNetworkManagerReturn {
  networkManager: NetworkManager | null;
  connectionStats: ConnectionStats | null;
  bandwidthUsage: BandwidthUsage | null;
  isConnected: boolean;
  httpGet: (url: string, headers?: Record<string, string>) => Promise<HttpResponse>;
  httpPost: (url: string, data: any, headers?: Record<string, string>) => Promise<HttpResponse>;
  httpPut: (url: string, data: any, headers?: Record<string, string>) => Promise<HttpResponse>;
  httpDelete: (url: string, headers?: Record<string, string>) => Promise<HttpResponse>;
  createWebSocket: (url: string) => Promise<string>;
  connectWebSocket: (id: string) => Promise<void>;
  sendWebSocketMessage: (id: string, message: string) => Promise<void>;
  closeWebSocket: (id: string) => Promise<void>;
  isWebSocketConnected: (id: string) => Promise<boolean>;
  createTcpServer: (port: number, host?: string) => Promise<TcpServerInfo>;
  createUdpServer: (port: number, host?: string) => Promise<UdpServerInfo>;
  setTimeout: (timeout: number) => Promise<void>;
  setRetryCount: (count: number) => Promise<void>;
  setUserAgent: (userAgent: string) => Promise<void>;
  ping: (host: string, timeout?: number) => Promise<number>;
  traceroute: (host: string) => Promise<string[]>;
  isInitialized: boolean;
}

// Component props
export interface SystemMonitorDisplayProps {
  className?: string;
  refreshInterval?: number;
  showCpu?: boolean;
  showMemory?: boolean;
  showDisk?: boolean;
  showNetwork?: boolean;
  showProcess?: boolean;
  showLoad?: boolean;
  showTemperature?: boolean;
  onError?: (error: Error) => void;
}

export interface NetworkStatusProps {
  className?: string;
  showConnectionStats?: boolean;
  showBandwidthUsage?: boolean;
  refreshInterval?: number;
  onError?: (error: Error) => void;
}

export interface ConfigEditorProps {
  className?: string;
  configKey?: string;
  defaultValue?: any;
  onSave?: (key: string, value: any) => void;
  onError?: (error: Error) => void;
  readOnly?: boolean;
}

export interface ErrorLoggerProps {
  className?: string;
  maxEntries?: number;
  showTimestamp?: boolean;
  showLogLevel?: boolean;
  filterLevel?: LogLevel;
  onError?: (error: Error) => void;
}

// Event types
export interface SystemMonitorEvent {
  type: 'cpu' | 'memory' | 'disk' | 'network' | 'process' | 'load' | 'temperature' | 'alert';
  data: any;
  timestamp: Date;
}

export interface NetworkEvent {
  type: 'connection' | 'bandwidth' | 'websocket' | 'http' | 'tcp' | 'udp';
  data: any;
  timestamp: Date;
}

export interface ConfigEvent {
  type: 'load' | 'save' | 'set' | 'remove' | 'validate';
  key?: string;
  value?: any;
  timestamp: Date;
}

export interface ErrorEvent {
  type: 'error' | 'warning' | 'info' | 'debug';
  message: string;
  details?: string;
  timestamp: Date;
}

// Utility types
export type CoreBaseEventType = 'system' | 'network' | 'config' | 'error';

export interface CoreBaseEvent {
  type: CoreBaseEventType;
  event: SystemMonitorEvent | NetworkEvent | ConfigEvent | ErrorEvent;
}

export type EventCallback<T = any> = (event: T) => void;

export interface EventSubscription {
  unsubscribe: () => void;
}

// Hook options
export interface UseSystemMonitorOptions {
  autoStart?: boolean;
  refreshInterval?: number;
  enableAlerts?: boolean;
  onAlert?: (alert: AlertInfo) => void;
  onError?: (error: Error) => void;
}

export interface UseNetworkManagerOptions {
  autoConnect?: boolean;
  timeout?: number;
  retryCount?: number;
  userAgent?: string;
  onError?: (error: Error) => void;
}

export interface UseConfigManagerOptions {
  autoLoad?: boolean;
  configFile?: string;
  onConfigChange?: (key: string, value: any) => void;
  onError?: (error: Error) => void;
}

export interface UseErrorHandlerOptions {
  defaultLogLevel?: LogLevel;
  enableConsoleOutput?: boolean;
  onError?: (error: Error) => void;
}