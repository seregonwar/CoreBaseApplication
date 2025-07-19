/**
 * @file index.ts
 * @brief TypeScript type definitions for CoreBase Node.js bindings
 */

// Error Handler Types
export interface LogLevel {
  ERROR: number;
  WARNING: number;
  INFO: number;
  DEBUG: number;
}

export interface ErrorHandler {
  initialize(): boolean;
  shutdown(): void;
  setLogLevel(level: number): void;
  getLogLevel(): number;
  logError(message: string): void;
  logWarning(message: string): void;
  logInfo(message: string): void;
  logDebug(message: string): void;
  handleError(error: string): void;
  handleException(exception: string): void;
  getLogLevels(): LogLevel;
}

// Config Manager Types
export interface ConfigManager {
  initialize(): boolean;
  shutdown(): void;
  loadConfig(filePath: string): boolean;
  saveConfig(filePath: string): boolean;
  getValue(key: string): any;
  setValue(key: string, value: any): boolean;
  hasKey(key: string): boolean;
  removeKey(key: string): boolean;
  getAllKeys(): string[];
  validateConfig(): boolean;
  getConfigAsJson(): string;
  setConfigFromJson(json: string): boolean;
}

// System Monitor Types
export interface CpuInfo {
  usage: number;
  cores: number;
  frequency: number;
  temperature?: number;
}

export interface MemoryInfo {
  totalPhysical: number;
  availablePhysical: number;
  usedPhysical: number;
  totalVirtual: number;
  availableVirtual: number;
  usedVirtual: number;
  usagePercentage: number;
}

export interface DiskInfo {
  totalSpace: number;
  freeSpace: number;
  usedSpace: number;
  usagePercentage: number;
  diskName: string;
}

export interface NetworkInfo {
  interfaceName: string;
  bytesReceived: number;
  bytesSent: number;
  packetsReceived: number;
  packetsSent: number;
  speed: number;
  isConnected: boolean;
}

export interface ProcessInfo {
  pid: number;
  name: string;
  cpuUsage: number;
  memoryUsage: number;
  threadCount: number;
}

export interface SystemLoad {
  load1min: number;
  load5min: number;
  load15min: number;
}

export interface TemperatureInfo {
  cpu: number;
  gpu?: number;
  motherboard?: number;
}

export interface PerformanceMetrics {
  timestamp: number;
  cpuUsage: number;
  memoryUsage: number;
  diskUsage: number;
  networkUsage: number;
}

export interface SystemAlert {
  type: string;
  message: string;
  timestamp: number;
  severity: string;
}

export interface SystemMonitor {
  initialize(): boolean;
  shutdown(): void;
  startMonitoring(): boolean;
  stopMonitoring(): void;
  isMonitoring(): boolean;
  getCpuInfo(): CpuInfo;
  getMemoryInfo(): MemoryInfo;
  getDiskInfo(drive?: string): DiskInfo[];
  getNetworkInfo(): NetworkInfo[];
  getProcessInfo(pid?: number): ProcessInfo[];
  getSystemLoad(): SystemLoad;
  getTemperatureInfo(): TemperatureInfo;
  getPerformanceMetrics(startTime?: number, endTime?: number): PerformanceMetrics[];
  getHistoricalData(hours: number): PerformanceMetrics[];
  setMonitoringInterval(intervalMs: number): void;
  getMonitoringInterval(): number;
  setThresholds(thresholds: Record<string, number>): void;
  getThresholds(): Record<string, number>;
  getAlerts(): SystemAlert[];
  clearAlerts(): void;
  exportData(format: string, filePath: string): boolean;
}

// Network Manager Types
export interface HttpResponse {
  statusCode: number;
  body: string;
  headers: Record<string, string>;
  success: boolean;
  errorMessage: string;
}

export interface HttpRequest {
  method: string;
  url: string;
  body?: string;
  headers?: Record<string, string>;
  timeout?: number;
}

export interface ConnectionStats {
  totalConnections: number;
  activeConnections: number;
  bytesReceived: number;
  bytesSent: number;
}

export interface BandwidthUsage {
  downloadSpeed: number;
  uploadSpeed: number;
  totalDownloaded: number;
  totalUploaded: number;
}

export interface NetworkManager {
  initialize(): boolean;
  shutdown(): void;
  isInitialized(): boolean;
  
  // HTTP Methods
  httpGet(url: string, headers?: Record<string, string>): HttpResponse;
  httpPost(url: string, data: string, headers?: Record<string, string>): HttpResponse;
  httpPut(url: string, data: string, headers?: Record<string, string>): HttpResponse;
  httpDelete(url: string, headers?: Record<string, string>): HttpResponse;
  httpRequest(request: HttpRequest): HttpResponse;
  
  // WebSocket Methods
  createWebSocket(url: string): number;
  connectWebSocket(socketId: number): boolean;
  sendWebSocketMessage(socketId: number, message: string): boolean;
  closeWebSocket(socketId: number): void;
  isWebSocketConnected(socketId: number): boolean;
  
  // TCP/UDP Methods
  createTcpServer(port: number, host?: string): number;
  createTcpClient(host: string, port: number): number;
  createUdpSocket(port: number, host?: string): number;
  sendTcpData(connectionId: number, data: string): boolean;
  sendUdpData(socketId: number, data: string, host: string, port: number): boolean;
  closeTcpConnection(connectionId: number): void;
  closeUdpSocket(socketId: number): void;
  
  // Configuration Methods
  setTimeout(timeoutMs: number): void;
  getTimeout(): number;
  setRetryCount(count: number): void;
  getRetryCount(): number;
  setUserAgent(userAgent: string): void;
  getUserAgent(): string;
  setHeaders(headers: Record<string, string>): void;
  getHeaders(): Record<string, string>;
  
  // SSL/TLS Methods
  enableSSL(): void;
  disableSSL(): void;
  setCertificate(certPath: string): boolean;
  setPrivateKey(keyPath: string): boolean;
  setCACertificate(caPath: string): boolean;
  verifySSL(verify: boolean): void;
  
  // Monitoring Methods
  getConnectionStats(): ConnectionStats;
  getBandwidthUsage(): BandwidthUsage;
  getActiveConnections(): number[];
  clearStats(): void;
  
  // Utility Methods
  isConnected(): boolean;
  getLocalIP(): string;
  getPublicIP(): string;
  resolveHostname(hostname: string): string;
  pingHost(host: string, timeout?: number): number;
  traceRoute(host: string): string[];
}

// Main CoreBase Interface
export interface CoreBase {
  initialize(): boolean;
  shutdown(): void;
  getPlatformInfo(): {
    platform: string;
    architecture: string;
    version: string;
  };
  getVersion(): string;
  
  // Component instances
  errorHandler: ErrorHandler;
  configManager: ConfigManager;
  systemMonitor: SystemMonitor;
  networkManager: NetworkManager;
}

// Platform-specific types
export type Platform = 'windows' | 'macos' | 'linux' | 'ios' | 'android';
export type Architecture = 'x64' | 'x86' | 'arm64' | 'arm';

export interface PlatformInfo {
  platform: Platform;
  architecture: Architecture;
  version: string;
  buildTarget: string;
}

// Export classes for instantiation
export declare class ErrorHandlerClass implements ErrorHandler {
  constructor();
  initialize(): boolean;
  shutdown(): void;
  setLogLevel(level: number): void;
  getLogLevel(): number;
  logError(message: string): void;
  logWarning(message: string): void;
  logInfo(message: string): void;
  logDebug(message: string): void;
  handleError(error: string): void;
  handleException(exception: string): void;
  getLogLevels(): LogLevel;
}

export declare class ConfigManagerClass implements ConfigManager {
  constructor();
  initialize(): boolean;
  shutdown(): void;
  loadConfig(filePath: string): boolean;
  saveConfig(filePath: string): boolean;
  getValue(key: string): any;
  setValue(key: string, value: any): boolean;
  hasKey(key: string): boolean;
  removeKey(key: string): boolean;
  getAllKeys(): string[];
  validateConfig(): boolean;
  getConfigAsJson(): string;
  setConfigFromJson(json: string): boolean;
}

export declare class SystemMonitorClass implements SystemMonitor {
  constructor();
  initialize(): boolean;
  shutdown(): void;
  startMonitoring(): boolean;
  stopMonitoring(): void;
  isMonitoring(): boolean;
  getCpuInfo(): CpuInfo;
  getMemoryInfo(): MemoryInfo;
  getDiskInfo(drive?: string): DiskInfo[];
  getNetworkInfo(): NetworkInfo[];
  getProcessInfo(pid?: number): ProcessInfo[];
  getSystemLoad(): SystemLoad;
  getTemperatureInfo(): TemperatureInfo;
  getPerformanceMetrics(startTime?: number, endTime?: number): PerformanceMetrics[];
  getHistoricalData(hours: number): PerformanceMetrics[];
  setMonitoringInterval(intervalMs: number): void;
  getMonitoringInterval(): number;
  setThresholds(thresholds: Record<string, number>): void;
  getThresholds(): Record<string, number>;
  getAlerts(): SystemAlert[];
  clearAlerts(): void;
  exportData(format: string, filePath: string): boolean;
}

export declare class NetworkManagerClass implements NetworkManager {
  constructor();
  initialize(): boolean;
  shutdown(): void;
  isInitialized(): boolean;
  httpGet(url: string, headers?: Record<string, string>): HttpResponse;
  httpPost(url: string, data: string, headers?: Record<string, string>): HttpResponse;
  httpPut(url: string, data: string, headers?: Record<string, string>): HttpResponse;
  httpDelete(url: string, headers?: Record<string, string>): HttpResponse;
  httpRequest(request: HttpRequest): HttpResponse;
  createWebSocket(url: string): number;
  connectWebSocket(socketId: number): boolean;
  sendWebSocketMessage(socketId: number, message: string): boolean;
  closeWebSocket(socketId: number): void;
  isWebSocketConnected(socketId: number): boolean;
  createTcpServer(port: number, host?: string): number;
  createTcpClient(host: string, port: number): number;
  createUdpSocket(port: number, host?: string): number;
  sendTcpData(connectionId: number, data: string): boolean;
  sendUdpData(socketId: number, data: string, host: string, port: number): boolean;
  closeTcpConnection(connectionId: number): void;
  closeUdpSocket(socketId: number): void;
  setTimeout(timeoutMs: number): void;
  getTimeout(): number;
  setRetryCount(count: number): void;
  getRetryCount(): number;
  setUserAgent(userAgent: string): void;
  getUserAgent(): string;
  setHeaders(headers: Record<string, string>): void;
  getHeaders(): Record<string, string>;
  enableSSL(): void;
  disableSSL(): void;
  setCertificate(certPath: string): boolean;
  setPrivateKey(keyPath: string): boolean;
  setCACertificate(caPath: string): boolean;
  verifySSL(verify: boolean): void;
  getConnectionStats(): ConnectionStats;
  getBandwidthUsage(): BandwidthUsage;
  getActiveConnections(): number[];
  clearStats(): void;
  isConnected(): boolean;
  getLocalIP(): string;
  getPublicIP(): string;
  resolveHostname(hostname: string): string;
  pingHost(host: string, timeout?: number): number;
  traceRoute(host: string): string[];
}