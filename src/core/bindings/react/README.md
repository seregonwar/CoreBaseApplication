# CoreBase React Bindings

React bindings for CoreBase - A comprehensive system monitoring and management library with React hooks and components.

## Features

- **React Hooks**: Easy-to-use hooks for all CoreBase functionality
- **React Components**: Pre-built components for system monitoring and network stats
- **TypeScript Support**: Full TypeScript definitions and type safety
- **Context Provider**: Centralized state management with React Context
- **Error Handling**: Comprehensive error handling with React error boundaries
- **Performance Optimized**: Efficient re-rendering and memory management
- **Cross-Platform**: Works on Windows, macOS, and Linux

## Installation

### Prerequisites

- Node.js 16.x or higher
- React 17.x or higher
- TypeScript 4.5 or higher (optional but recommended)

### Install Package

```bash
npm install @corebase/react
# or
yarn add @corebase/react
# or
pnpm add @corebase/react
```

## Quick Start

### 1. Setup Provider

Wrap your app with the CoreBase provider:

```tsx
import React from 'react';
import { CoreBaseProvider } from '@corebase/react';
import App from './App';

function Root() {
  return (
    <CoreBaseProvider
      config={{
        logLevel: 'info',
        enableSystemMonitor: true,
        enableNetworkManager: true,
        autoInitialize: true
      }}
    >
      <App />
    </CoreBaseProvider>
  );
}

export default Root;
```

### 2. Use Hooks

```tsx
import React from 'react';
import {
  useSystemMonitor,
  useNetworkManager,
  useErrorHandler,
  useConfigManager
} from '@corebase/react';

function MyComponent() {
  const { metrics, isMonitoring, startMonitoring, stopMonitoring } = useSystemMonitor();
  const { connectionStats, makeRequest } = useNetworkManager();
  const { logError, logInfo } = useErrorHandler();
  const { getValue, setValue } = useConfigManager();

  React.useEffect(() => {
    startMonitoring();
    return () => stopMonitoring();
  }, []);

  const handleApiCall = async () => {
    try {
      const response = await makeRequest('GET', 'https://api.example.com/data');
      logInfo('API call successful', { response });
    } catch (error) {
      logError('API call failed', error);
    }
  };

  return (
    <div>
      <h2>System Metrics</h2>
      {metrics && (
        <div>
          <p>CPU Usage: {metrics.cpu?.usage}%</p>
          <p>Memory Usage: {metrics.memory?.usage}%</p>
          <p>Disk Usage: {metrics.disk?.usage}%</p>
        </div>
      )}
      
      <h2>Network Stats</h2>
      {connectionStats && (
        <div>
          <p>Active Connections: {connectionStats.activeConnections}</p>
          <p>Bytes Sent: {connectionStats.bytesSent}</p>
          <p>Bytes Received: {connectionStats.bytesReceived}</p>
        </div>
      )}
      
      <button onClick={handleApiCall}>Make API Call</button>
    </div>
  );
}
```

### 3. Use Pre-built Components

```tsx
import React from 'react';
import { SystemMonitorDisplay, NetworkStatsDisplay } from '@corebase/react';

function Dashboard() {
  return (
    <div>
      <h1>System Dashboard</h1>
      
      <SystemMonitorDisplay
        autoStart={true}
        refreshInterval={5000}
        showCpu={true}
        showMemory={true}
        showDisk={true}
        showNetwork={true}
        onAlert={(alert) => console.log('System Alert:', alert)}
        onError={(error) => console.error('Monitor Error:', error)}
      />
      
      <NetworkStatsDisplay
        autoRefresh={true}
        refreshInterval={3000}
        onError={(error) => console.error('Network Error:', error)}
      />
    </div>
  );
}
```

## API Reference

### CoreBaseProvider

The main provider component that initializes CoreBase and provides context to child components.

```tsx
interface CoreBaseProviderProps {
  children: React.ReactNode;
  config?: CoreBaseConfig;
  onError?: (error: Error) => void;
  onReady?: () => void;
}
```

#### Configuration Options

```tsx
interface CoreBaseConfig {
  logLevel?: 'debug' | 'info' | 'warning' | 'error';
  enableErrorHandler?: boolean;
  enableConfigManager?: boolean;
  enableSystemMonitor?: boolean;
  enableNetworkManager?: boolean;
  autoInitialize?: boolean;
  systemMonitor?: {
    autoStart?: boolean;
    interval?: number;
  };
  networkManager?: {
    timeout?: number;
    retryCount?: number;
    userAgent?: string;
  };
}
```

### Hooks

#### useSystemMonitor

Hook for system monitoring functionality.

```tsx
const {
  metrics,
  alerts,
  isMonitoring,
  isLoading,
  error,
  startMonitoring,
  stopMonitoring,
  refreshData,
  setMonitoringInterval,
  exportData
} = useSystemMonitor({
  autoStart?: boolean;
  interval?: number;
  onAlert?: (alert: SystemAlert) => void;
  onError?: (error: Error) => void;
});
```

#### useNetworkManager

Hook for network operations.

```tsx
const {
  connectionStats,
  bandwidthUsage,
  isConnected,
  isLoading,
  error,
  makeRequest,
  createWebSocket,
  createTcpServer,
  createUdpServer,
  ping,
  traceroute,
  refreshStats
} = useNetworkManager({
  timeout?: number;
  retryCount?: number;
  userAgent?: string;
  onError?: (error: Error) => void;
});
```

#### useErrorHandler

Hook for error handling and logging.

```tsx
const {
  logLevel,
  setLogLevel,
  getLogLevel,
  logError,
  logWarning,
  logInfo,
  logDebug,
  handleError,
  handleException
} = useErrorHandler();
```

#### useConfigManager

Hook for configuration management.

```tsx
const {
  getValue,
  setValue,
  hasKey,
  removeKey,
  validateConfig,
  loadConfig,
  saveConfig,
  isLoading,
  error
} = useConfigManager();
```

### Components

#### SystemMonitorDisplay

Pre-built component for displaying system metrics.

```tsx
interface SystemMonitorDisplayProps {
  autoStart?: boolean;
  refreshInterval?: number;
  showCpu?: boolean;
  showMemory?: boolean;
  showDisk?: boolean;
  showNetwork?: boolean;
  showProcess?: boolean;
  showLoad?: boolean;
  showTemperature?: boolean;
  showPerformance?: boolean;
  className?: string;
  style?: React.CSSProperties;
  onAlert?: (alert: SystemAlert) => void;
  onError?: (error: Error) => void;
}
```

#### NetworkStatsDisplay

Pre-built component for displaying network statistics.

```tsx
interface NetworkStatsDisplayProps {
  autoRefresh?: boolean;
  refreshInterval?: number;
  showConnectionStats?: boolean;
  showBandwidthUsage?: boolean;
  showConnectionStatus?: boolean;
  className?: string;
  style?: React.CSSProperties;
  onError?: (error: Error) => void;
}
```

## Development

### Scripts

```bash
# Install dependencies
npm install

# Build the library
npm run build

# Run tests
npm test

# Run tests in watch mode
npm run test:watch

# Run tests with coverage
npm run test:coverage

# Lint code
npm run lint

# Fix linting issues
npm run lint:fix

# Format code
npm run format

# Type check
npm run type-check

# Clean build artifacts
npm run clean

# Run Storybook
npm run storybook

# Build Storybook
npm run build-storybook
```

### Project Structure

```
src/
├── components/          # React components
│   ├── SystemMonitorDisplay.tsx
│   ├── NetworkStatsDisplay.tsx
│   └── index.ts
├── contexts/           # React contexts
│   ├── CoreBaseContext.tsx
│   └── index.ts
├── hooks/              # React hooks
│   ├── useSystemMonitor.ts
│   ├── useNetworkManager.ts
│   ├── useErrorHandler.ts
│   ├── useConfigManager.ts
│   └── index.ts
├── types/              # TypeScript types
│   └── index.ts
├── __tests__/          # Test files
│   ├── setup.ts
│   ├── globalSetup.ts
│   ├── globalTeardown.ts
│   └── *.test.tsx
└── index.ts            # Main entry point
```

### Testing

The library includes comprehensive tests using Jest and React Testing Library:

```bash
# Run all tests
npm test

# Run tests for a specific file
npm test -- SystemMonitorDisplay

# Run tests in watch mode
npm run test:watch

# Generate coverage report
npm run test:coverage
```

### Building

The library is built using Rollup with multiple output formats:

- **CommonJS**: `dist/index.js`
- **ES Modules**: `dist/index.esm.js`
- **TypeScript Definitions**: `dist/index.d.ts`

```bash
npm run build
```

## Error Handling

The library provides comprehensive error handling:

```tsx
import { CoreBaseProvider, useErrorHandler } from '@corebase/react';

function App() {
  const handleGlobalError = (error: Error) => {
    console.error('CoreBase Error:', error);
    // Send to error reporting service
  };

  return (
    <CoreBaseProvider onError={handleGlobalError}>
      <MyComponent />
    </CoreBaseProvider>
  );
}

function MyComponent() {
  const { handleError } = useErrorHandler();

  const riskyOperation = async () => {
    try {
      // Some risky operation
    } catch (error) {
      handleError(error, { context: 'riskyOperation' });
    }
  };
}
```

## Performance Considerations

- **Memoization**: All hooks use React.useMemo and React.useCallback for optimization
- **Selective Updates**: Components only re-render when relevant data changes
- **Cleanup**: Automatic cleanup of timers, listeners, and resources
- **Lazy Loading**: Components and hooks are loaded only when needed

## Cross-Platform Support

The React bindings work on all platforms supported by the CoreBase native library:

- **Windows**: Full support for all features
- **macOS**: Full support for all features
- **Linux**: Full support for all features

## Troubleshooting

### Common Issues

1. **Native Module Not Found**
   ```
   Error: Cannot find module 'corebase_addon.node'
   ```
   - Ensure the native module is built for your platform
   - Run `npm rebuild` to rebuild native dependencies

2. **React Version Compatibility**
   ```
   Error: Invalid hook call
   ```
   - Ensure React version is 17.x or higher
   - Check for multiple React instances in node_modules

3. **TypeScript Errors**
   ```
   Error: Cannot find type definitions
   ```
   - Ensure TypeScript version is 4.5 or higher
   - Install `@types/react` and `@types/react-dom`

### Debug Mode

Enable debug logging to troubleshoot issues:

```tsx
<CoreBaseProvider
  config={{
    logLevel: 'debug'
  }}
>
  <App />
</CoreBaseProvider>
```

## License

MIT License - see LICENSE file for details.

## Contributing

Contributions are welcome! Please read our contributing guidelines and submit pull requests to our repository.

## Support

For support and questions:

- GitHub Issues: [Create an issue](https://github.com/seregonwar/corebase/issues)
- Documentation: [Full documentation](https://docs.corebase.dev)
- Discord: [Join our community](https://discord.gg/corebase)