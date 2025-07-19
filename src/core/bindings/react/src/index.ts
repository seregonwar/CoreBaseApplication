/**
 * CoreBase React Bindings
 * 
 * This package provides React hooks and components for integrating CoreBase
 * functionality into React applications.
 * 
 * @example
 * ```tsx
 * import { CoreBaseProvider, useSystemMonitor, SystemMonitorDisplay } from '@corebase/react';
 * 
 * function App() {
 *   return (
 *     <CoreBaseProvider config={{ logLevel: 'info' }}>
 *       <MyComponent />
 *     </CoreBaseProvider>
 *   );
 * }
 * 
 * function MyComponent() {
 *   const { cpuInfo, memoryInfo } = useSystemMonitor();
 *   return <SystemMonitorDisplay />;
 * }
 * ```
 */

// Context and Provider
export { CoreBaseProvider } from './contexts/CoreBaseContext';
export {
  useCoreBase,
  useCoreBaseReady,
  useCoreBaseError,
  useCoreBaseLoading,
  usePlatformInfo,
  useCoreBaseVersion
} from './contexts/CoreBaseContext';

// Hooks
export {
  useErrorHandler,
  useConfigManager,
  useSystemMonitor,
  useNetworkManager
} from './hooks';

// Components
export {
  SystemMonitorDisplay,
  NetworkStatsDisplay
} from './components';

// Types
export type {
  // Context types
  CoreBaseProviderProps,
  CoreBaseConfig,
  CoreBaseContextValue,
  
  // Hook types
  UseErrorHandlerReturn,
  UseErrorHandlerOptions,
  UseConfigManagerReturn,
  UseConfigManagerOptions,
  UseSystemMonitorReturn,
  UseSystemMonitorOptions,
  UseNetworkManagerReturn,
  UseNetworkManagerOptions,
  
  // Component types
  SystemMonitorDisplayProps,
  NetworkStatsDisplayProps,
  
  // Event types
  CoreBaseErrorEvent,
  CoreBaseReadyEvent,
  SystemAlertEvent,
  NetworkErrorEvent,
  ConfigChangeEvent
} from './types';

// Re-export types from nodejs bindings
export type {
  LogLevel,
  PlatformInfo,
  CpuInfo,
  MemoryInfo,
  DiskInfo,
  NetworkInfo,
  ProcessInfo,
  LoadInfo,
  TemperatureInfo,
  PerformanceMetrics,
  SystemAlert,
  HttpResponse,
  HttpRequest,
  HttpHeaders,
  WebSocketMessage,
  ConnectionStats,
  BandwidthUsage,
  TcpServerOptions,
  UdpServerOptions,
  SslConfig
} from '@corebase/nodejs';