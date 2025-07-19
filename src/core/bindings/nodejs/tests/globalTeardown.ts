/**
 * @file globalTeardown.ts
 * @brief Global teardown for Jest tests
 */

export default async (): Promise<void> => {
  console.log('🧹 Starting CoreBase Node.js bindings test suite cleanup...');
  
  try {
    // Clean up any global test resources
    if (process.env.COREBASE_MOCK_NATIVE !== 'true') {
      // If we have the native addon, ensure it's properly shut down
      try {
        const addon = require('../build/Release/corebase_addon.node');
        addon.shutdown();
        console.log('✅ Native addon shutdown completed');
      } catch (error) {
        console.warn('⚠️  Error during native addon shutdown:', error.message);
      }
    }
    
    // Clean up temporary files if any were created
    // This would be where you'd clean up test databases, temp files, etc.
    
    // Reset environment variables
    delete process.env.COREBASE_LOG_LEVEL;
    delete process.env.COREBASE_TEST_MODE;
    delete process.env.COREBASE_MOCK_NATIVE;
    
    console.log('✅ Global test cleanup completed');
  } catch (error) {
    console.error('❌ Error during global test cleanup:', error);
    // Don't throw here as it would cause Jest to fail
  }
  
  console.log('🏁 CoreBase Node.js bindings test suite finished');
};