/**
 * @file globalSetup.ts
 * @brief Global setup for Jest tests
 */

export default async (): Promise<void> => {
  console.log('🚀 Starting CoreBase Node.js bindings test suite...');
  
  // Set environment variables for testing
  process.env.NODE_ENV = 'test';
  process.env.COREBASE_LOG_LEVEL = 'ERROR'; // Reduce log noise during tests
  process.env.COREBASE_TEST_MODE = 'true';
  
  // Initialize any global test resources here
  try {
    // Check if native addon can be loaded
    const addon = require('../build/Release/corebase_addon.node');
    console.log('✅ Native addon loaded successfully');
    
    // Perform basic initialization test
    const initResult = addon.initialize();
    if (initResult) {
      console.log('✅ Native addon initialization test passed');
      addon.shutdown();
    } else {
      console.warn('⚠️  Native addon initialization test failed');
    }
  } catch (error) {
    console.warn('⚠️  Native addon not available for testing:', error.message);
    console.log('📝 Tests will run with mocked implementations');
    
    // Set flag to indicate we're running without native addon
    process.env.COREBASE_MOCK_NATIVE = 'true';
  }
  
  // Set up test database or other resources if needed
  console.log('🔧 Global test setup completed');
};