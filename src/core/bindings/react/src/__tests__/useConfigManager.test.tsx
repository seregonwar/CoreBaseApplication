import React from 'react';
import { renderHook, act } from '@testing-library/react';
import { jest } from '@jest/globals';

import { useConfigManager } from '../hooks/useConfigManager';
import { CoreBaseProvider } from '../contexts/CoreBaseContext';

// Mock @corebase/nodejs
const mockConfigManager = {
  get: jest.fn(),
  set: jest.fn(),
  has: jest.fn(),
  remove: jest.fn(),
  clear: jest.fn(),
  getAll: jest.fn(),
  save: jest.fn(),
  load: jest.fn(),
  reload: jest.fn(),
  watch: jest.fn(),
  unwatch: jest.fn(),
  getConfigPath: jest.fn(),
  setConfigPath: jest.fn(),
  validate: jest.fn(),
  getSchema: jest.fn(),
  setSchema: jest.fn(),
};

jest.mock('@corebase/nodejs', () => ({
  CoreBaseManager: jest.fn().mockImplementation(() => ({
    initialize: jest.fn().mockResolvedValue(undefined),
    shutdown: jest.fn().mockResolvedValue(undefined),
    getConfigManager: jest.fn().mockReturnValue(mockConfigManager),
  })),
}));

const TestWrapper: React.FC<{ children: React.ReactNode }> = ({ children }) => (
  <CoreBaseProvider config={{ autoInitialize: true }}>
    {children}
  </CoreBaseProvider>
);

describe('useConfigManager', () => {
  beforeEach(() => {
    jest.clearAllMocks();
  });

  it('should return config manager functions', () => {
    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    expect(result.current.get).toBeDefined();
    expect(result.current.set).toBeDefined();
    expect(result.current.has).toBeDefined();
    expect(result.current.remove).toBeDefined();
    expect(result.current.clear).toBeDefined();
    expect(result.current.getAll).toBeDefined();
    expect(result.current.save).toBeDefined();
    expect(result.current.load).toBeDefined();
    expect(result.current.reload).toBeDefined();
    expect(result.current.watch).toBeDefined();
    expect(result.current.unwatch).toBeDefined();
    expect(result.current.getConfigPath).toBeDefined();
    expect(result.current.setConfigPath).toBeDefined();
    expect(result.current.validate).toBeDefined();
    expect(result.current.getSchema).toBeDefined();
    expect(result.current.setSchema).toBeDefined();
  });

  it('should get configuration values', async () => {
    mockConfigManager.get.mockResolvedValue('test-value');

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const value = await result.current.get('test.key');
      expect(value).toBe('test-value');
    });

    expect(mockConfigManager.get).toHaveBeenCalledWith('test.key');
  });

  it('should get configuration values with default', async () => {
    mockConfigManager.get.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const value = await result.current.get('missing.key', 'default-value');
      expect(value).toBe('default-value');
    });

    expect(mockConfigManager.get).toHaveBeenCalledWith('missing.key', 'default-value');
  });

  it('should set configuration values', async () => {
    mockConfigManager.set.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.set('test.key', 'new-value');
    });

    expect(mockConfigManager.set).toHaveBeenCalledWith('test.key', 'new-value');
  });

  it('should check if configuration key exists', async () => {
    mockConfigManager.has.mockResolvedValue(true);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const exists = await result.current.has('test.key');
      expect(exists).toBe(true);
    });

    expect(mockConfigManager.has).toHaveBeenCalledWith('test.key');
  });

  it('should remove configuration keys', async () => {
    mockConfigManager.remove.mockResolvedValue(true);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const removed = await result.current.remove('test.key');
      expect(removed).toBe(true);
    });

    expect(mockConfigManager.remove).toHaveBeenCalledWith('test.key');
  });

  it('should clear all configuration', async () => {
    mockConfigManager.clear.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.clear();
    });

    expect(mockConfigManager.clear).toHaveBeenCalled();
  });

  it('should get all configuration', async () => {
    const mockConfig = {
      'app.name': 'Test App',
      'app.version': '1.0.0',
      'database.host': 'localhost',
    };
    mockConfigManager.getAll.mockResolvedValue(mockConfig);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const config = await result.current.getAll();
      expect(config).toEqual(mockConfig);
    });

    expect(mockConfigManager.getAll).toHaveBeenCalled();
  });

  it('should save configuration', async () => {
    mockConfigManager.save.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.save();
    });

    expect(mockConfigManager.save).toHaveBeenCalled();
  });

  it('should save configuration to specific path', async () => {
    mockConfigManager.save.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.save('/custom/config/path.json');
    });

    expect(mockConfigManager.save).toHaveBeenCalledWith('/custom/config/path.json');
  });

  it('should load configuration', async () => {
    mockConfigManager.load.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.load();
    });

    expect(mockConfigManager.load).toHaveBeenCalled();
  });

  it('should load configuration from specific path', async () => {
    mockConfigManager.load.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.load('/custom/config/path.json');
    });

    expect(mockConfigManager.load).toHaveBeenCalledWith('/custom/config/path.json');
  });

  it('should reload configuration', async () => {
    mockConfigManager.reload.mockResolvedValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.reload();
    });

    expect(mockConfigManager.reload).toHaveBeenCalled();
  });

  it('should watch configuration changes', async () => {
    const mockCallback = jest.fn();
    mockConfigManager.watch.mockResolvedValue('watch-id');

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const watchId = await result.current.watch('test.key', mockCallback);
      expect(watchId).toBe('watch-id');
    });

    expect(mockConfigManager.watch).toHaveBeenCalledWith('test.key', mockCallback);
  });

  it('should watch all configuration changes', async () => {
    const mockCallback = jest.fn();
    mockConfigManager.watch.mockResolvedValue('watch-all-id');

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const watchId = await result.current.watch(mockCallback);
      expect(watchId).toBe('watch-all-id');
    });

    expect(mockConfigManager.watch).toHaveBeenCalledWith(mockCallback);
  });

  it('should unwatch configuration changes', async () => {
    mockConfigManager.unwatch.mockResolvedValue(true);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const unwatched = await result.current.unwatch('watch-id');
      expect(unwatched).toBe(true);
    });

    expect(mockConfigManager.unwatch).toHaveBeenCalledWith('watch-id');
  });

  it('should get configuration path', () => {
    mockConfigManager.getConfigPath.mockReturnValue('/path/to/config.json');

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    const path = result.current.getConfigPath();
    expect(path).toBe('/path/to/config.json');
    expect(mockConfigManager.getConfigPath).toHaveBeenCalled();
  });

  it('should set configuration path', () => {
    mockConfigManager.setConfigPath.mockReturnValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.setConfigPath('/new/config/path.json');
    });

    expect(mockConfigManager.setConfigPath).toHaveBeenCalledWith('/new/config/path.json');
  });

  it('should validate configuration', async () => {
    const mockValidationResult = {
      valid: true,
      errors: [],
    };
    mockConfigManager.validate.mockResolvedValue(mockValidationResult);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const validation = await result.current.validate();
      expect(validation).toEqual(mockValidationResult);
    });

    expect(mockConfigManager.validate).toHaveBeenCalled();
  });

  it('should validate specific configuration', async () => {
    const testConfig = { 'app.name': 'Test App' };
    const mockValidationResult = {
      valid: true,
      errors: [],
    };
    mockConfigManager.validate.mockResolvedValue(mockValidationResult);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      const validation = await result.current.validate(testConfig);
      expect(validation).toEqual(mockValidationResult);
    });

    expect(mockConfigManager.validate).toHaveBeenCalledWith(testConfig);
  });

  it('should get configuration schema', () => {
    const mockSchema = {
      type: 'object',
      properties: {
        'app.name': { type: 'string' },
        'app.version': { type: 'string' },
      },
    };
    mockConfigManager.getSchema.mockReturnValue(mockSchema);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    const schema = result.current.getSchema();
    expect(schema).toEqual(mockSchema);
    expect(mockConfigManager.getSchema).toHaveBeenCalled();
  });

  it('should set configuration schema', () => {
    const mockSchema = {
      type: 'object',
      properties: {
        'app.name': { type: 'string', required: true },
      },
    };
    mockConfigManager.setSchema.mockReturnValue(undefined);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    act(() => {
      result.current.setSchema(mockSchema);
    });

    expect(mockConfigManager.setSchema).toHaveBeenCalledWith(mockSchema);
  });

  it('should handle complex configuration objects', async () => {
    const complexConfig = {
      database: {
        host: 'localhost',
        port: 5432,
        credentials: {
          username: 'admin',
          password: 'secret',
        },
      },
      features: {
        enableLogging: true,
        maxRetries: 3,
        timeout: 30000,
      },
    };

    mockConfigManager.set.mockResolvedValue(undefined);
    mockConfigManager.get.mockResolvedValue(complexConfig.database);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      await result.current.set('database', complexConfig.database);
      const retrieved = await result.current.get('database');
      expect(retrieved).toEqual(complexConfig.database);
    });

    expect(mockConfigManager.set).toHaveBeenCalledWith('database', complexConfig.database);
    expect(mockConfigManager.get).toHaveBeenCalledWith('database');
  });

  it('should handle configuration errors gracefully', async () => {
    const error = new Error('Configuration error');
    mockConfigManager.get.mockRejectedValue(error);

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    await act(async () => {
      try {
        await result.current.get('invalid.key');
      } catch (e) {
        expect(e).toBe(error);
      }
    });

    expect(mockConfigManager.get).toHaveBeenCalledWith('invalid.key');
  });

  it('should throw error when used outside provider', () => {
    const consoleSpy = jest.spyOn(console, 'error').mockImplementation(() => {});

    expect(() => {
      renderHook(() => useConfigManager());
    }).toThrow('useConfigManager must be used within a CoreBaseProvider');

    consoleSpy.mockRestore();
  });

  it('should handle config manager not ready', async () => {
    const TestWrapperNotReady: React.FC<{ children: React.ReactNode }> = ({ children }) => (
      <CoreBaseProvider config={{ autoInitialize: false }}>
        {children}
      </CoreBaseProvider>
    );

    const { result } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapperNotReady,
    });

    // Should not throw, but functions should handle the case gracefully
    await act(async () => {
      try {
        await result.current.get('test.key');
      } catch (e) {
        // Expected to fail when config manager is not ready
      }
    });
  });

  it('should maintain function references across re-renders', () => {
    const { result, rerender } = renderHook(() => useConfigManager(), {
      wrapper: TestWrapper,
    });

    const firstRenderFunctions = {
      get: result.current.get,
      set: result.current.set,
      has: result.current.has,
      remove: result.current.remove,
      clear: result.current.clear,
      getAll: result.current.getAll,
      save: result.current.save,
      load: result.current.load,
      reload: result.current.reload,
      watch: result.current.watch,
      unwatch: result.current.unwatch,
      getConfigPath: result.current.getConfigPath,
      setConfigPath: result.current.setConfigPath,
      validate: result.current.validate,
      getSchema: result.current.getSchema,
      setSchema: result.current.setSchema,
    };

    rerender();

    // Functions should be memoized and maintain references
    expect(result.current.get).toBe(firstRenderFunctions.get);
    expect(result.current.set).toBe(firstRenderFunctions.set);
    expect(result.current.has).toBe(firstRenderFunctions.has);
    expect(result.current.remove).toBe(firstRenderFunctions.remove);
    expect(result.current.clear).toBe(firstRenderFunctions.clear);
    expect(result.current.getAll).toBe(firstRenderFunctions.getAll);
    expect(result.current.save).toBe(firstRenderFunctions.save);
    expect(result.current.load).toBe(firstRenderFunctions.load);
    expect(result.current.reload).toBe(firstRenderFunctions.reload);
    expect(result.current.watch).toBe(firstRenderFunctions.watch);
    expect(result.current.unwatch).toBe(firstRenderFunctions.unwatch);
    expect(result.current.getConfigPath).toBe(firstRenderFunctions.getConfigPath);
    expect(result.current.setConfigPath).toBe(firstRenderFunctions.setConfigPath);
    expect(result.current.validate).toBe(firstRenderFunctions.validate);
    expect(result.current.getSchema).toBe(firstRenderFunctions.getSchema);
    expect(result.current.setSchema).toBe(firstRenderFunctions.setSchema);
  });
});