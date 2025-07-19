import { GlobalSetupContext } from '@jest/types';

/**
 * Global setup for React bindings tests
 */
export default async function globalSetup(globalConfig: any, projectConfig: GlobalSetupContext): Promise<void> {
  console.log('🚀 Setting up React bindings tests...');
  
  // Set environment variables for testing
  process.env.NODE_ENV = 'test';
  process.env.COREBASE_LOG_LEVEL = 'error'; // Reduce log noise in tests
  process.env.COREBASE_TEST_MODE = 'true';
  process.env.REACT_APP_TESTING = 'true';
  
  // Mock native addon availability
  process.env.COREBASE_MOCK_NATIVE = 'true';
  
  // Set up test database or external services if needed
  // This is where you would initialize test databases, start mock servers, etc.
  
  try {
    // Try to load the native addon for testing
    // If it fails, we'll use mocks
    const { CoreBaseManager } = require('@corebase/nodejs');
    
    // Test basic functionality
    const manager = new CoreBaseManager();
    await manager.initialize();
    await manager.shutdown();
    
    console.log('✅ Native CoreBase addon is available for testing');
    process.env.COREBASE_MOCK_NATIVE = 'false';
  } catch (error) {
    console.log('⚠️  Native CoreBase addon not available, using mocks for testing');
    console.log(`   Reason: ${error instanceof Error ? error.message : 'Unknown error'}`);
    process.env.COREBASE_MOCK_NATIVE = 'true';
  }
  
  // Set up React testing environment
  if (typeof window !== 'undefined') {
    // Browser environment setup
    window.ResizeObserver = window.ResizeObserver || class ResizeObserver {
      constructor(callback: ResizeObserverCallback) {}
      observe(target: Element, options?: ResizeObserverOptions): void {}
      unobserve(target: Element): void {}
      disconnect(): void {}
    };
    
    window.IntersectionObserver = window.IntersectionObserver || class IntersectionObserver {
      constructor(callback: IntersectionObserverCallback, options?: IntersectionObserverInit) {}
      observe(target: Element): void {}
      unobserve(target: Element): void {}
      disconnect(): void {}
      readonly root: Element | null = null;
      readonly rootMargin: string = '';
      readonly thresholds: ReadonlyArray<number> = [];
      takeRecords(): IntersectionObserverEntry[] { return []; }
    };
  }
  
  // Set up performance monitoring for tests
  if (typeof performance !== 'undefined') {
    performance.mark = performance.mark || (() => {});
    performance.measure = performance.measure || (() => {});
    performance.getEntriesByName = performance.getEntriesByName || (() => []);
    performance.clearMarks = performance.clearMarks || (() => {});
    performance.clearMeasures = performance.clearMeasures || (() => {});
  }
  
  console.log('✅ React bindings test environment setup complete');
}