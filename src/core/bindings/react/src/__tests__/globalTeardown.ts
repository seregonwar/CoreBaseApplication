import { GlobalTeardownContext } from '@jest/types';
import * as fs from 'fs';
import * as path from 'path';

/**
 * Global teardown for React bindings tests
 */
export default async function globalTeardown(globalConfig: any, projectConfig: GlobalTeardownContext): Promise<void> {
  console.log('🧹 Cleaning up React bindings tests...');
  
  try {
    // Shutdown CoreBase if it was initialized and not mocked
    if (process.env.COREBASE_MOCK_NATIVE !== 'true') {
      try {
        const { CoreBaseManager } = require('@corebase/nodejs');
        const manager = new CoreBaseManager();
        await manager.shutdown();
        console.log('✅ CoreBase manager shut down successfully');
      } catch (error) {
        console.log('⚠️  Error shutting down CoreBase manager:', error instanceof Error ? error.message : 'Unknown error');
      }
    }
    
    // Clean up temporary files created during tests
    const tempDirs = [
      path.join(process.cwd(), 'temp'),
      path.join(process.cwd(), '.tmp'),
      path.join(process.cwd(), 'test-temp')
    ];
    
    for (const tempDir of tempDirs) {
      if (fs.existsSync(tempDir)) {
        try {
          fs.rmSync(tempDir, { recursive: true, force: true });
          console.log(`🗑️  Cleaned up temporary directory: ${tempDir}`);
        } catch (error) {
          console.log(`⚠️  Could not clean up ${tempDir}:`, error instanceof Error ? error.message : 'Unknown error');
        }
      }
    }
    
    // Clean up test configuration files
    const testConfigFiles = [
      path.join(process.cwd(), 'test-config.json'),
      path.join(process.cwd(), '.test-env')
    ];
    
    for (const configFile of testConfigFiles) {
      if (fs.existsSync(configFile)) {
        try {
          fs.unlinkSync(configFile);
          console.log(`🗑️  Cleaned up test config file: ${configFile}`);
        } catch (error) {
          console.log(`⚠️  Could not clean up ${configFile}:`, error instanceof Error ? error.message : 'Unknown error');
        }
      }
    }
    
    // Reset environment variables
    delete process.env.COREBASE_LOG_LEVEL;
    delete process.env.COREBASE_TEST_MODE;
    delete process.env.COREBASE_MOCK_NATIVE;
    delete process.env.REACT_APP_TESTING;
    
    // Clear any global timers or intervals that might be running
    if (typeof global !== 'undefined') {
      // Clear any global state that might have been set during tests
      if ((global as any).__COREBASE_TEST_STATE__) {
        delete (global as any).__COREBASE_TEST_STATE__;
      }
    }
    
    // Force garbage collection if available
    if (global.gc) {
      global.gc();
      console.log('🗑️  Forced garbage collection');
    }
    
    console.log('✅ React bindings test cleanup complete');
    
  } catch (error) {
    console.error('❌ Error during test cleanup:', error instanceof Error ? error.message : 'Unknown error');
    // Don't throw here as it would cause Jest to fail
  }
}