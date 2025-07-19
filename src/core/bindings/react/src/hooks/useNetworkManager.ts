import { useState, useEffect, useCallback } from 'react';
import { useCoreBase } from '../contexts/CoreBaseContext';
import {
  UseNetworkManagerReturn,
  UseNetworkManagerOptions,
  ConnectionStats,
  BandwidthUsage,
  HttpResponse,
  TcpServerInfo,
  UdpServerInfo
} from '../types';

/**
 * Hook for managing network functionality
 * @param options Configuration options for the network manager
 * @returns Network manager functions and state
 */
export const useNetworkManager = (options: UseNetworkManagerOptions = {}): UseNetworkManagerReturn => {
  const { networkManager, isInitialized } = useCoreBase();
  const [isManagerInitialized, setIsManagerInitialized] = useState(false);
  const [connectionStats, setConnectionStats] = useState<ConnectionStats | null>(null);
  const [bandwidthUsage, setBandwidthUsage] = useState<BandwidthUsage | null>(null);
  const [isConnected, setIsConnected] = useState(false);

  // Initialize network manager
  useEffect(() => {
    const initializeManager = async () => {
      if (!networkManager || !isInitialized) {
        setIsManagerInitialized(false);
        return;
      }

      try {
        // Configure timeout if provided
        if (options.timeout) {
          await networkManager.setTimeout(options.timeout);
        }

        // Configure retry count if provided
        if (options.retryCount) {
          await networkManager.setRetryCount(options.retryCount);
        }

        // Configure user agent if provided
        if (options.userAgent) {
          await networkManager.setUserAgent(options.userAgent);
        }

        // Check initial connection status
        if (options.autoConnect) {
          const connected = await networkManager.isConnected();
          setIsConnected(connected);
        }

        setIsManagerInitialized(true);
      } catch (error) {
        console.error('Failed to initialize network manager:', error);
        if (options.onError) {
          options.onError(error instanceof Error ? error : new Error(String(error)));
        }
        setIsManagerInitialized(false);
      }
    };

    initializeManager();
  }, [networkManager, isInitialized, options.timeout, options.retryCount, options.userAgent, options.autoConnect, options.onError]);

  // Update connection stats periodically
  useEffect(() => {
    if (!isManagerInitialized || !networkManager) {
      return;
    }

    const updateStats = async () => {
      try {
        const [stats, bandwidth, connected] = await Promise.allSettled([
          networkManager.getConnectionStats(),
          networkManager.getBandwidthUsage(),
          networkManager.isConnected()
        ]);

        if (stats.status === 'fulfilled') {
          setConnectionStats(stats.value);
        }
        if (bandwidth.status === 'fulfilled') {
          setBandwidthUsage(bandwidth.value);
        }
        if (connected.status === 'fulfilled') {
          setIsConnected(connected.value);
        }
      } catch (error) {
        console.error('Failed to update network stats:', error);
      }
    };

    // Initial update
    updateStats();

    // Update every 5 seconds
    const interval = setInterval(updateStats, 5000);

    return () => clearInterval(interval);
  }, [isManagerInitialized, networkManager]);

  // HTTP GET request
  const httpGet = useCallback(async (url: string, headers?: Record<string, string>): Promise<HttpResponse> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.httpGet(url, headers);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // HTTP POST request
  const httpPost = useCallback(async (url: string, data: any, headers?: Record<string, string>): Promise<HttpResponse> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.httpPost(url, data, headers);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // HTTP PUT request
  const httpPut = useCallback(async (url: string, data: any, headers?: Record<string, string>): Promise<HttpResponse> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.httpPut(url, data, headers);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // HTTP DELETE request
  const httpDelete = useCallback(async (url: string, headers?: Record<string, string>): Promise<HttpResponse> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.httpDelete(url, headers);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Create WebSocket
  const createWebSocket = useCallback(async (url: string): Promise<string> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.createWebSocket(url);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Connect WebSocket
  const connectWebSocket = useCallback(async (id: string): Promise<void> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      await networkManager.connectWebSocket(id);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Send WebSocket message
  const sendWebSocketMessage = useCallback(async (id: string, message: string): Promise<void> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      await networkManager.sendWebSocketMessage(id, message);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Close WebSocket
  const closeWebSocket = useCallback(async (id: string): Promise<void> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      await networkManager.closeWebSocket(id);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Check WebSocket connection status
  const isWebSocketConnected = useCallback(async (id: string): Promise<boolean> => {
    if (!networkManager || !isManagerInitialized) {
      return false;
    }

    try {
      return await networkManager.isWebSocketConnected(id);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      return false;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Create TCP server
  const createTcpServer = useCallback(async (port: number, host?: string): Promise<TcpServerInfo> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.createTcpServer(port, host);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Create UDP server
  const createUdpServer = useCallback(async (port: number, host?: string): Promise<UdpServerInfo> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.createUdpServer(port, host);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Set timeout
  const setTimeout = useCallback(async (timeout: number): Promise<void> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      await networkManager.setTimeout(timeout);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Set retry count
  const setRetryCount = useCallback(async (count: number): Promise<void> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      await networkManager.setRetryCount(count);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Set user agent
  const setUserAgent = useCallback(async (userAgent: string): Promise<void> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      await networkManager.setUserAgent(userAgent);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Ping host
  const ping = useCallback(async (host: string, timeout?: number): Promise<number> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.ping(host, timeout);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  // Traceroute to host
  const traceroute = useCallback(async (host: string): Promise<string[]> => {
    if (!networkManager || !isManagerInitialized) {
      throw new Error('Network manager not initialized');
    }

    try {
      return await networkManager.traceroute(host);
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      if (options.onError) {
        options.onError(err);
      }
      throw err;
    }
  }, [networkManager, isManagerInitialized, options.onError]);

  return {
    networkManager,
    connectionStats,
    bandwidthUsage,
    isConnected,
    httpGet,
    httpPost,
    httpPut,
    httpDelete,
    createWebSocket,
    connectWebSocket,
    sendWebSocketMessage,
    closeWebSocket,
    isWebSocketConnected,
    createTcpServer,
    createUdpServer,
    setTimeout,
    setRetryCount,
    setUserAgent,
    ping,
    traceroute,
    isInitialized: isManagerInitialized
  };
};

export default useNetworkManager;