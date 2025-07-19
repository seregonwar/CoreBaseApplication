/**
 * @file index.ts
 * @brief Main entry point for CoreBase Node.js bindings
 */

import { 
  CoreBase, 
  ErrorHandlerClass, 
  ConfigManagerClass, 
  SystemMonitorClass, 
  NetworkManagerClass,
  PlatformInfo,
  Platform,
  Architecture
} from './types';

// Import the native addon
const addon = require('../build/Release/corebase_addon.node');

/**
 * Main CoreBase class that provides access to all components
 */
export class CoreBaseManager implements CoreBase {
  private _errorHandler: ErrorHandlerClass;
  private _configManager: ConfigManagerClass;
  private _systemMonitor: SystemMonitorClass;
  private _networkManager: NetworkManagerClass;
  private _initialized: boolean = false;

  constructor() {
    this._errorHandler = new addon.ErrorHandler();
    this._configManager = new addon.ConfigManager();
    this._systemMonitor = new addon.SystemMonitor();
    this._networkManager = new addon.NetworkManager();
  }

  /**
   * Initialize the CoreBase system
   * @returns {boolean} True if initialization was successful
   */
  public initialize(): boolean {
    try {
      if (this._initialized) {
        return true;
      }

      // Initialize addon
      const addonInit = addon.initialize();
      if (!addonInit) {
        throw new Error('Failed to initialize native addon');
      }

      // Initialize all components
      const errorHandlerInit = this._errorHandler.initialize();
      const configManagerInit = this._configManager.initialize();
      const systemMonitorInit = this._systemMonitor.initialize();
      const networkManagerInit = this._networkManager.initialize();

      if (!errorHandlerInit || !configManagerInit || !systemMonitorInit || !networkManagerInit) {
        throw new Error('Failed to initialize one or more components');
      }

      this._initialized = true;
      this._errorHandler.logInfo('CoreBase Node.js bindings initialized successfully');
      return true;
    } catch (error) {
      console.error('CoreBase initialization failed:', error);
      return false;
    }
  }

  /**
   * Shutdown the CoreBase system
   */
  public shutdown(): void {
    try {
      if (!this._initialized) {
        return;
      }

      this._errorHandler.logInfo('Shutting down CoreBase Node.js bindings');

      // Shutdown all components in reverse order
      this._networkManager.shutdown();
      this._systemMonitor.shutdown();
      this._configManager.shutdown();
      this._errorHandler.shutdown();

      // Shutdown addon
      addon.shutdown();

      this._initialized = false;
    } catch (error) {
      console.error('CoreBase shutdown failed:', error);
    }
  }

  /**
   * Get platform information
   * @returns {PlatformInfo} Platform details
   */
  public getPlatformInfo(): PlatformInfo {
    try {
      const platformInfo = addon.getPlatformInfo();
      return {
        platform: platformInfo.platform as Platform,
        architecture: platformInfo.architecture as Architecture,
        version: platformInfo.version,
        buildTarget: platformInfo.buildTarget || 'unknown'
      };
    } catch (error) {
      console.error('Failed to get platform info:', error);
      return {
        platform: 'linux' as Platform,
        architecture: 'x64' as Architecture,
        version: 'unknown',
        buildTarget: 'unknown'
      };
    }
  }

  /**
   * Get CoreBase version
   * @returns {string} Version string
   */
  public getVersion(): string {
    try {
      return addon.getVersion();
    } catch (error) {
      console.error('Failed to get version:', error);
      return '1.0.0';
    }
  }

  /**
   * Check if CoreBase is initialized
   * @returns {boolean} True if initialized
   */
  public isInitialized(): boolean {
    return this._initialized;
  }

  // Getters for component instances
  public get errorHandler(): ErrorHandlerClass {
    return this._errorHandler;
  }

  public get configManager(): ConfigManagerClass {
    return this._configManager;
  }

  public get systemMonitor(): SystemMonitorClass {
    return this._systemMonitor;
  }

  public get networkManager(): NetworkManagerClass {
    return this._networkManager;
  }
}

/**
 * Factory function to create a new CoreBase instance
 * @returns {CoreBaseManager} New CoreBase instance
 */
export function createCoreBase(): CoreBaseManager {
  return new CoreBaseManager();
}

/**
 * Singleton instance for convenience
 */
let _instance: CoreBaseManager | null = null;

/**
 * Get the singleton CoreBase instance
 * @returns {CoreBaseManager} Singleton instance
 */
export function getCoreBase(): CoreBaseManager {
  if (!_instance) {
    _instance = new CoreBaseManager();
  }
  return _instance;
}

/**
 * Initialize the singleton CoreBase instance
 * @returns {boolean} True if initialization was successful
 */
export function initializeCoreBase(): boolean {
  const instance = getCoreBase();
  return instance.initialize();
}

/**
 * Shutdown the singleton CoreBase instance
 */
export function shutdownCoreBase(): void {
  if (_instance) {
    _instance.shutdown();
    _instance = null;
  }
}

// Export individual component classes for direct use
export { ErrorHandlerClass as ErrorHandler } from './types';
export { ConfigManagerClass as ConfigManager } from './types';
export { SystemMonitorClass as SystemMonitor } from './types';
export { NetworkManagerClass as NetworkManager } from './types';

// Export all types
export * from './types';

// Default export
export default {
  CoreBaseManager,
  createCoreBase,
  getCoreBase,
  initializeCoreBase,
  shutdownCoreBase,
  ErrorHandler: ErrorHandlerClass,
  ConfigManager: ConfigManagerClass,
  SystemMonitor: SystemMonitorClass,
  NetworkManager: NetworkManagerClass
};