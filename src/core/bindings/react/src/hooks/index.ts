/**
 * React Hooks for CoreBase
 * 
 * This module exports all React hooks for interacting with CoreBase functionality
 */

export { useErrorHandler } from './useErrorHandler';
export { useConfigManager } from './useConfigManager';
export { useSystemMonitor } from './useSystemMonitor';
export { useNetworkManager } from './useNetworkManager';

export type {
  UseErrorHandlerReturn,
  UseErrorHandlerOptions,
  UseConfigManagerReturn,
  UseConfigManagerOptions,
  UseSystemMonitorReturn,
  UseSystemMonitorOptions,
  UseNetworkManagerReturn,
  UseNetworkManagerOptions
} from '../types';