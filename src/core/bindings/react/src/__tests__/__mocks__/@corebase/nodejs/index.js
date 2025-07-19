// Mock implementation of @corebase/nodejs for testing

// Mock CoreBaseManager
const mockCoreBaseManager = {
  initialize: jest.fn().mockResolvedValue(true),
  shutdown: jest.fn().mockResolvedValue(true),
  isInitialized: jest.fn().mockReturnValue(true),
  getVersion: jest.fn().mockReturnValue('1.0.0'),
  getStatus: jest.fn().mockReturnValue('ready')
};

// Mock SystemMonitor
const mockSystemMonitor = {
  start: jest.fn().mockResolvedValue(true),
  stop: jest.fn().mockResolvedValue(true),
  isRunning: jest.fn().mockReturnValue(false),
  getSystemInfo: jest.fn().mockReturnValue({
    cpu: {
      usage: 45.2,
      cores: 8,
      model: 'Intel Core i7-9700K',
      frequency: 3600
    },
    memory: {
      total: 16777216, // 16GB in KB
      used: 8388608,   // 8GB in KB
      free: 8388608,   // 8GB in KB
      usage: 50.0
    },
    disk: {
      total: 1048576000, // 1TB in KB
      used: 524288000,   // 500GB in KB
      free: 524288000,   // 500GB in KB
      usage: 50.0
    }
  }),
  getCpuUsage: jest.fn().mockReturnValue(45.2),
  getMemoryUsage: jest.fn().mockReturnValue({
    total: 16777216,
    used: 8388608,
    free: 8388608,
    usage: 50.0
  }),
  getDiskUsage: jest.fn().mockReturnValue({
    total: 1048576000,
    used: 524288000,
    free: 524288000,
    usage: 50.0
  }),
  setRefreshInterval: jest.fn(),
  getRefreshInterval: jest.fn().mockReturnValue(1000),
  exportData: jest.fn().mockReturnValue({
    timestamp: Date.now(),
    cpu: { usage: 45.2 },
    memory: { usage: 50.0 },
    disk: { usage: 50.0 }
  }),
  clearData: jest.fn()
};

// Mock NetworkManager
const mockNetworkManager = {
  start: jest.fn().mockResolvedValue(true),
  stop: jest.fn().mockResolvedValue(true),
  isRunning: jest.fn().mockReturnValue(false),
  refresh: jest.fn().mockResolvedValue(true),
  getConnectionStats: jest.fn().mockReturnValue({
    activeConnections: 12,
    totalConnections: 156,
    failedConnections: 3,
    successRate: 98.1
  }),
  getBandwidthUsage: jest.fn().mockReturnValue({
    download: 1024000, // 1MB/s
    upload: 512000,    // 512KB/s
    total: 1536000     // 1.5MB/s
  }),
  getConnectionStatus: jest.fn().mockReturnValue({
    isConnected: true,
    connectionType: 'ethernet',
    signalStrength: 95,
    latency: 15
  }),
  getNetworkInterfaces: jest.fn().mockReturnValue([
    {
      name: 'Ethernet',
      type: 'ethernet',
      status: 'up',
      ipAddress: '192.168.1.100',
      macAddress: '00:11:22:33:44:55'
    },
    {
      name: 'Wi-Fi',
      type: 'wireless',
      status: 'down',
      ipAddress: null,
      macAddress: '66:77:88:99:AA:BB'
    }
  ]),
  getActiveConnections: jest.fn().mockReturnValue([
    {
      id: '1',
      protocol: 'TCP',
      localAddress: '192.168.1.100:8080',
      remoteAddress: '203.0.113.1:443',
      state: 'ESTABLISHED'
    },
    {
      id: '2',
      protocol: 'UDP',
      localAddress: '192.168.1.100:53',
      remoteAddress: '8.8.8.8:53',
      state: 'CONNECTED'
    }
  ]),
  getNetworkConfig: jest.fn().mockReturnValue({
    autoRefresh: true,
    refreshInterval: 2000,
    enableLogging: true,
    maxConnections: 1000
  }),
  setNetworkConfig: jest.fn(),
  setRefreshInterval: jest.fn(),
  getRefreshInterval: jest.fn().mockReturnValue(2000),
  exportData: jest.fn().mockReturnValue({
    timestamp: Date.now(),
    connections: { active: 12, total: 156 },
    bandwidth: { download: 1024000, upload: 512000 },
    status: { isConnected: true, latency: 15 }
  }),
  clearData: jest.fn()
};

// Mock ConfigManager
const mockConfigManager = {
  get: jest.fn((key, defaultValue) => {
    const mockConfig = {
      'app.name': 'CoreBase Test App',
      'app.version': '1.0.0',
      'logging.level': 'info',
      'network.timeout': 5000,
      'system.autoStart': true
    };
    return mockConfig[key] || defaultValue;
  }),
  set: jest.fn(),
  has: jest.fn((key) => {
    const mockConfig = {
      'app.name': 'CoreBase Test App',
      'app.version': '1.0.0',
      'logging.level': 'info',
      'network.timeout': 5000,
      'system.autoStart': true
    };
    return key in mockConfig;
  }),
  remove: jest.fn(),
  clear: jest.fn(),
  getAll: jest.fn().mockReturnValue({
    'app.name': 'CoreBase Test App',
    'app.version': '1.0.0',
    'logging.level': 'info',
    'network.timeout': 5000,
    'system.autoStart': true
  }),
  save: jest.fn().mockResolvedValue(true),
  load: jest.fn().mockResolvedValue(true),
  reload: jest.fn().mockResolvedValue(true),
  watch: jest.fn(),
  unwatch: jest.fn(),
  getConfigPath: jest.fn().mockReturnValue('/mock/config/path'),
  setConfigPath: jest.fn(),
  validate: jest.fn().mockReturnValue({ isValid: true, errors: [] }),
  getSchema: jest.fn().mockReturnValue({}),
  setSchema: jest.fn()
};

// Mock ErrorHandler
const mockErrorHandler = {
  logError: jest.fn(),
  logWarning: jest.fn(),
  logInfo: jest.fn(),
  logDebug: jest.fn(),
  getLogLevel: jest.fn().mockReturnValue('info'),
  setLogLevel: jest.fn()
};

// Export all mocks
module.exports = {
  CoreBaseManager: mockCoreBaseManager,
  SystemMonitor: mockSystemMonitor,
  NetworkManager: mockNetworkManager,
  ConfigManager: mockConfigManager,
  ErrorHandler: mockErrorHandler
};