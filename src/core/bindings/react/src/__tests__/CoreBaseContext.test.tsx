import React from 'react';
import { render, screen, waitFor, act } from '@testing-library/react';
import { CoreBaseProvider, useCoreBase, useCoreBaseReady, useCoreBaseError } from '../contexts/CoreBaseContext';
import type { CoreBaseConfig } from '../types';

// Mock the nodejs bindings
jest.mock('@corebase/nodejs', () => ({
  CoreBaseManager: jest.fn().mockImplementation(() => ({
    initialize: jest.fn().mockResolvedValue(undefined),
    shutdown: jest.fn().mockResolvedValue(undefined),
    getPlatformInfo: jest.fn().mockReturnValue({
      platform: 'win32',
      architecture: 'x64',
      version: '10.0.19041',
      hostname: 'test-machine',
      username: 'test-user'
    }),
    getVersion: jest.fn().mockReturnValue('1.0.0'),
    getErrorHandler: jest.fn().mockReturnValue({
      setLogLevel: jest.fn(),
      getLogLevel: jest.fn().mockReturnValue('info')
    }),
    getConfigManager: jest.fn().mockReturnValue({
      setValue: jest.fn(),
      getValue: jest.fn()
    }),
    getSystemMonitor: jest.fn().mockReturnValue({
      startMonitoring: jest.fn(),
      stopMonitoring: jest.fn()
    }),
    getNetworkManager: jest.fn().mockReturnValue({
      initialize: jest.fn(),
      setTimeout: jest.fn()
    })
  }))
}));

describe('CoreBaseContext', () => {
  const TestComponent: React.FC = () => {
    const coreBase = useCoreBase();
    const isReady = useCoreBaseReady();
    const error = useCoreBaseError();

    return (
      <div>
        <div data-testid="ready-status">{isReady ? 'ready' : 'not-ready'}</div>
        <div data-testid="error-status">{error || 'no-error'}</div>
        <div data-testid="corebase-status">{coreBase ? 'available' : 'not-available'}</div>
      </div>
    );
  };

  beforeEach(() => {
    jest.clearAllMocks();
  });

  it('should provide CoreBase context to children', async () => {
    const config: CoreBaseConfig = {
      logLevel: 'info',
      autoInitialize: true
    };

    render(
      <CoreBaseProvider config={config}>
        <TestComponent />
      </CoreBaseProvider>
    );

    // Initially not ready
    expect(screen.getByTestId('ready-status')).toHaveTextContent('not-ready');
    expect(screen.getByTestId('error-status')).toHaveTextContent('no-error');
    expect(screen.getByTestId('corebase-status')).toHaveTextContent('not-available');

    // Wait for initialization
    await waitFor(() => {
      expect(screen.getByTestId('ready-status')).toHaveTextContent('ready');
    });

    expect(screen.getByTestId('corebase-status')).toHaveTextContent('available');
  });

  it('should handle initialization errors', async () => {
    const mockCoreBase = require('@corebase/nodejs').CoreBaseManager;
    mockCoreBase.mockImplementation(() => ({
      initialize: jest.fn().mockRejectedValue(new Error('Initialization failed')),
      shutdown: jest.fn().mockResolvedValue(undefined),
      getPlatformInfo: jest.fn(),
      getVersion: jest.fn()
    }));

    const config: CoreBaseConfig = {
      logLevel: 'error',
      autoInitialize: true
    };

    render(
      <CoreBaseProvider config={config}>
        <TestComponent />
      </CoreBaseProvider>
    );

    await waitFor(() => {
      expect(screen.getByTestId('error-status')).toHaveTextContent('Initialization failed');
    });

    expect(screen.getByTestId('ready-status')).toHaveTextContent('not-ready');
  });

  it('should not auto-initialize when autoInitialize is false', async () => {
    const config: CoreBaseConfig = {
      logLevel: 'debug',
      autoInitialize: false
    };

    render(
      <CoreBaseProvider config={config}>
        <TestComponent />
      </CoreBaseProvider>
    );

    // Should remain not ready
    await act(async () => {
      await new Promise(resolve => setTimeout(resolve, 100));
    });

    expect(screen.getByTestId('ready-status')).toHaveTextContent('not-ready');
    expect(screen.getByTestId('corebase-status')).toHaveTextContent('not-available');
  });

  it('should cleanup on unmount', async () => {
    const mockShutdown = jest.fn().mockResolvedValue(undefined);
    const mockCoreBase = require('@corebase/nodejs').CoreBaseManager;
    mockCoreBase.mockImplementation(() => ({
      initialize: jest.fn().mockResolvedValue(undefined),
      shutdown: mockShutdown,
      getPlatformInfo: jest.fn().mockReturnValue({}),
      getVersion: jest.fn().mockReturnValue('1.0.0'),
      getErrorHandler: jest.fn().mockReturnValue({}),
      getConfigManager: jest.fn().mockReturnValue({}),
      getSystemMonitor: jest.fn().mockReturnValue({}),
      getNetworkManager: jest.fn().mockReturnValue({})
    }));

    const config: CoreBaseConfig = {
      logLevel: 'info',
      autoInitialize: true
    };

    const { unmount } = render(
      <CoreBaseProvider config={config}>
        <TestComponent />
      </CoreBaseProvider>
    );

    await waitFor(() => {
      expect(screen.getByTestId('ready-status')).toHaveTextContent('ready');
    });

    unmount();

    await waitFor(() => {
      expect(mockShutdown).toHaveBeenCalled();
    });
  });

  it('should throw error when hooks are used outside provider', () => {
    // Suppress console.error for this test
    const consoleSpy = jest.spyOn(console, 'error').mockImplementation(() => {});

    expect(() => {
      render(<TestComponent />);
    }).toThrow('useCoreBase must be used within a CoreBaseProvider');

    consoleSpy.mockRestore();
  });

  it('should handle config changes', async () => {
    const config: CoreBaseConfig = {
      logLevel: 'info',
      autoInitialize: true
    };

    const { rerender } = render(
      <CoreBaseProvider config={config}>
        <TestComponent />
      </CoreBaseProvider>
    );

    await waitFor(() => {
      expect(screen.getByTestId('ready-status')).toHaveTextContent('ready');
    });

    // Change config
    const newConfig: CoreBaseConfig = {
      logLevel: 'debug',
      autoInitialize: true
    };

    rerender(
      <CoreBaseProvider config={newConfig}>
        <TestComponent />
      </CoreBaseProvider>
    );

    // Should still be ready
    expect(screen.getByTestId('ready-status')).toHaveTextContent('ready');
  });
});