/**
 * @file corebase.test.ts
 * @brief Jest tests for CoreBase Node.js bindings
 */

import {
  CoreBaseManager,
  createCoreBase,
  getCoreBase,
  initializeCoreBase,
  shutdownCoreBase
} from '../src/index';

describe('CoreBase Node.js Bindings', () => {
  let corebase: CoreBaseManager;

  beforeEach(() => {
    corebase = createCoreBase();
  });

  afterEach(() => {
    if (corebase && corebase.isInitialized()) {
      corebase.shutdown();
    }
  });

  describe('CoreBaseManager', () => {
    test('should create instance successfully', () => {
      expect(corebase).toBeDefined();
      expect(corebase).toBeInstanceOf(CoreBaseManager);
    });

    test('should initialize successfully', () => {
      const result = corebase.initialize();
      expect(result).toBe(true);
      expect(corebase.isInitialized()).toBe(true);
    });

    test('should shutdown successfully', () => {
      corebase.initialize();
      expect(corebase.isInitialized()).toBe(true);
      
      corebase.shutdown();
      expect(corebase.isInitialized()).toBe(false);
    });

    test('should get platform info', () => {
      corebase.initialize();
      const platformInfo = corebase.getPlatformInfo();
      
      expect(platformInfo).toBeDefined();
      expect(platformInfo.platform).toBeDefined();
      expect(platformInfo.architecture).toBeDefined();
      expect(platformInfo.version).toBeDefined();
      expect(['windows', 'macos', 'linux', 'ios', 'android']).toContain(platformInfo.platform);
      expect(['x64', 'x86', 'arm64', 'arm']).toContain(platformInfo.architecture);
    });

    test('should get version', () => {
      corebase.initialize();
      const version = corebase.getVersion();
      
      expect(version).toBeDefined();
      expect(typeof version).toBe('string');
      expect(version.length).toBeGreaterThan(0);
    });

    test('should provide access to all components', () => {
      corebase.initialize();
      
      expect(corebase.errorHandler).toBeDefined();
      expect(corebase.configManager).toBeDefined();
      expect(corebase.systemMonitor).toBeDefined();
      expect(corebase.networkManager).toBeDefined();
    });
  });

  describe('ErrorHandler', () => {
    beforeEach(() => {
      corebase.initialize();
    });

    test('should set and get log level', () => {
      const errorHandler = corebase.errorHandler;
      const logLevels = errorHandler.getLogLevels();
      
      errorHandler.setLogLevel(logLevels.INFO);
      expect(errorHandler.getLogLevel()).toBe(logLevels.INFO);
      
      errorHandler.setLogLevel(logLevels.DEBUG);
      expect(errorHandler.getLogLevel()).toBe(logLevels.DEBUG);
    });

    test('should log messages without throwing', () => {
      const errorHandler = corebase.errorHandler;
      
      expect(() => {
        errorHandler.logError('Test error message');
        errorHandler.logWarning('Test warning message');
        errorHandler.logInfo('Test info message');
        errorHandler.logDebug('Test debug message');
      }).not.toThrow();
    });

    test('should handle errors and exceptions', () => {
      const errorHandler = corebase.errorHandler;
      
      expect(() => {
        errorHandler.handleError('Test error');
        errorHandler.handleException('Test exception');
      }).not.toThrow();
    });
  });

  describe('ConfigManager', () => {
    beforeEach(() => {
      corebase.initialize();
    });

    test('should set and get values', () => {
      const configManager = corebase.configManager;
      
      const result = configManager.setValue('test.key', 'test value');
      expect(result).toBe(true);
      
      const value = configManager.getValue('test.key');
      expect(value).toBe('test value');
    });

    test('should check if key exists', () => {
      const configManager = corebase.configManager;
      
      configManager.setValue('existing.key', 'value');
      expect(configManager.hasKey('existing.key')).toBe(true);
      expect(configManager.hasKey('non.existing.key')).toBe(false);
    });

    test('should remove keys', () => {
      const configManager = corebase.configManager;
      
      configManager.setValue('removable.key', 'value');
      expect(configManager.hasKey('removable.key')).toBe(true);
      
      const result = configManager.removeKey('removable.key');
      expect(result).toBe(true);
      expect(configManager.hasKey('removable.key')).toBe(false);
    });

    test('should get all keys', () => {
      const configManager = corebase.configManager;
      
      configManager.setValue('key1', 'value1');
      configManager.setValue('key2', 'value2');
      
      const keys = configManager.getAllKeys();
      expect(Array.isArray(keys)).toBe(true);
      expect(keys).toContain('key1');
      expect(keys).toContain('key2');
    });

    test('should validate config', () => {
      const configManager = corebase.configManager;
      
      const result = configManager.validateConfig();
      expect(typeof result).toBe('boolean');
    });
  });

  describe('SystemMonitor', () => {
    beforeEach(() => {
      corebase.initialize();
    });

    test('should start and stop monitoring', () => {
      const systemMonitor = corebase.systemMonitor;
      
      const startResult = systemMonitor.startMonitoring();
      expect(startResult).toBe(true);
      expect(systemMonitor.isMonitoring()).toBe(true);
      
      systemMonitor.stopMonitoring();
      expect(systemMonitor.isMonitoring()).toBe(false);
    });

    test('should get CPU info', () => {
      const systemMonitor = corebase.systemMonitor;
      systemMonitor.startMonitoring();
      
      const cpuInfo = systemMonitor.getCpuInfo();
      expect(cpuInfo).toBeDefined();
      expect(typeof cpuInfo.usage).toBe('number');
      expect(typeof cpuInfo.cores).toBe('number');
      expect(typeof cpuInfo.frequency).toBe('number');
      expect(cpuInfo.cores).toBeGreaterThan(0);
    });

    test('should get memory info', () => {
      const systemMonitor = corebase.systemMonitor;
      systemMonitor.startMonitoring();
      
      const memoryInfo = systemMonitor.getMemoryInfo();
      expect(memoryInfo).toBeDefined();
      expect(typeof memoryInfo.totalPhysical).toBe('number');
      expect(typeof memoryInfo.usedPhysical).toBe('number');
      expect(typeof memoryInfo.usagePercentage).toBe('number');
      expect(memoryInfo.totalPhysical).toBeGreaterThan(0);
    });

    test('should get disk info', () => {
      const systemMonitor = corebase.systemMonitor;
      systemMonitor.startMonitoring();
      
      const diskInfo = systemMonitor.getDiskInfo();
      expect(Array.isArray(diskInfo)).toBe(true);
      
      if (diskInfo.length > 0) {
        const disk = diskInfo[0];
        expect(typeof disk.totalSpace).toBe('number');
        expect(typeof disk.freeSpace).toBe('number');
        expect(typeof disk.usagePercentage).toBe('number');
        expect(disk.totalSpace).toBeGreaterThan(0);
      }
    });

    test('should set and get monitoring interval', () => {
      const systemMonitor = corebase.systemMonitor;
      
      systemMonitor.setMonitoringInterval(5000);
      expect(systemMonitor.getMonitoringInterval()).toBe(5000);
      
      systemMonitor.setMonitoringInterval(1000);
      expect(systemMonitor.getMonitoringInterval()).toBe(1000);
    });
  });

  describe('NetworkManager', () => {
    beforeEach(() => {
      corebase.initialize();
    });

    test('should be initialized', () => {
      const networkManager = corebase.networkManager;
      expect(networkManager.isInitialized()).toBe(true);
    });

    test('should set and get timeout', () => {
      const networkManager = corebase.networkManager;
      
      networkManager.setTimeout(10000);
      expect(networkManager.getTimeout()).toBe(10000);
      
      networkManager.setTimeout(5000);
      expect(networkManager.getTimeout()).toBe(5000);
    });

    test('should create WebSocket', () => {
      const networkManager = corebase.networkManager;
      
      const socketId = networkManager.createWebSocket('ws://localhost:8080');
      expect(typeof socketId).toBe('number');
      expect(socketId).toBeGreaterThanOrEqual(0);
    });

    test('should create TCP server', () => {
      const networkManager = corebase.networkManager;
      
      const serverId = networkManager.createTcpServer(0); // Use port 0 for auto-assignment
      expect(typeof serverId).toBe('number');
      expect(serverId).toBeGreaterThanOrEqual(0);
    });

    test('should get connection stats', () => {
      const networkManager = corebase.networkManager;
      
      const stats = networkManager.getConnectionStats();
      expect(stats).toBeDefined();
      expect(typeof stats.totalConnections).toBe('number');
      expect(typeof stats.activeConnections).toBe('number');
      expect(typeof stats.bytesReceived).toBe('number');
      expect(typeof stats.bytesSent).toBe('number');
    });
  });

  describe('Singleton Functions', () => {
    afterEach(() => {
      shutdownCoreBase();
    });

    test('should get singleton instance', () => {
      const instance1 = getCoreBase();
      const instance2 = getCoreBase();
      
      expect(instance1).toBe(instance2); // Same instance
    });

    test('should initialize singleton', () => {
      const result = initializeCoreBase();
      expect(result).toBe(true);
      
      const instance = getCoreBase();
      expect(instance.isInitialized()).toBe(true);
    });

    test('should shutdown singleton', () => {
      initializeCoreBase();
      const instance = getCoreBase();
      expect(instance.isInitialized()).toBe(true);
      
      shutdownCoreBase();
      
      // Getting instance again should create a new one
      const newInstance = getCoreBase();
      expect(newInstance.isInitialized()).toBe(false);
    });
  });
});