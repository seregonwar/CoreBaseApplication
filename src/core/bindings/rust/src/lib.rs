//! # CoreBase Rust Bindings
//!
//! This crate provides Rust bindings for the CoreBaseApplication C++ framework.
//! It allows Rust applications to interact with the core functionality including
//! error handling, configuration management, networking, and system monitoring.

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_double};
use std::ptr;
use std::sync::{Arc, Mutex, Once};
use std::collections::HashMap;

pub mod error;
pub mod config;
pub mod network;
pub mod monitor;

use error::*;
use config::*;
use network::*;
use monitor::*;

/// Log levels matching the C++ LogLevel enum
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4,
}

impl From<c_int> for LogLevel {
    fn from(value: c_int) -> Self {
        match value {
            0 => LogLevel::Debug,
            1 => LogLevel::Info,
            2 => LogLevel::Warning,
            3 => LogLevel::Error,
            4 => LogLevel::Critical,
            _ => LogLevel::Info, // Default fallback
        }
    }
}

impl From<LogLevel> for c_int {
    fn from(level: LogLevel) -> Self {
        level as c_int
    }
}

/// External C++ function declarations
extern "C" {
    // ErrorHandler functions
    fn cba_error_handler_initialize() -> c_int;
    fn cba_error_handler_shutdown() -> c_int;
    fn cba_error_handler_handle_error(message: *const c_char, file: *const c_char, line: c_int, function: *const c_char) -> c_int;
    fn cba_error_handler_set_log_level(level: c_int) -> c_int;
    fn cba_error_handler_get_log_level() -> c_int;
    fn cba_error_handler_log(level: c_int, message: *const c_char) -> c_int;
    
    // ConfigManager functions
    fn cba_config_load(filename: *const c_char) -> c_int;
    fn cba_config_get_value(key: *const c_char, buffer: *mut c_char, buffer_size: c_int) -> c_int;
    fn cba_config_set_value(key: *const c_char, value: *const c_char) -> c_int;
    fn cba_config_save(filename: *const c_char) -> c_int;
    
    // NetworkManager functions
    fn cba_network_initialize() -> c_int;
    fn cba_network_create_connection(host: *const c_char, port: c_int, protocol: c_int) -> *mut c_char;
    fn cba_network_send_message(connection_id: *const c_char, message: *const c_char) -> c_int;
    fn cba_network_receive_message(connection_id: *const c_char, buffer: *mut c_char, buffer_size: c_int) -> c_int;
    fn cba_network_close_connection(connection_id: *const c_char) -> c_int;
    
    // SystemMonitor functions
    fn cba_monitor_get_cpu_usage() -> c_double;
    fn cba_monitor_get_memory_usage(available: *mut c_double, total: *mut c_double) -> c_int;
    fn cba_monitor_get_disk_usage(available: *mut c_double, total: *mut c_double) -> c_int;
    fn cba_monitor_get_network_usage() -> c_double;
    fn cba_monitor_get_gpu_usage() -> c_double;
}

/// Global initialization state
static INIT: Once = Once::new();
static mut INITIALIZED: bool = false;

/// Initialize the CoreBase library
/// 
/// This function must be called before using any other CoreBase functionality.
/// It's safe to call multiple times - subsequent calls will be ignored.
/// 
/// # Returns
/// 
/// `Ok(())` if initialization was successful, `Err(CoreBaseError)` otherwise.
pub fn initialize() -> Result<(), CoreBaseError> {
    let mut result = Ok(());
    
    INIT.call_once(|| {
        unsafe {
            let error_init = cba_error_handler_initialize();
            let network_init = cba_network_initialize();
            
            if error_init == 0 && network_init == 0 {
                INITIALIZED = true;
            } else {
                result = Err(CoreBaseError::InitializationFailed(
                    "Failed to initialize CoreBase components".to_string()
                ));
            }
        }
    });
    
    result
}

/// Shutdown the CoreBase library
/// 
/// This function should be called when the application is shutting down
/// to properly clean up resources.
pub fn shutdown() -> Result<(), CoreBaseError> {
    unsafe {
        if INITIALIZED {
            let result = cba_error_handler_shutdown();
            if result == 0 {
                INITIALIZED = false;
                Ok(())
            } else {
                Err(CoreBaseError::ShutdownFailed(
                    "Failed to shutdown CoreBase components".to_string()
                ))
            }
        } else {
            Ok(()) // Already shutdown or never initialized
        }
    }
}

/// Check if the CoreBase library is initialized
pub fn is_initialized() -> bool {
    unsafe { INITIALIZED }
}

/// Utility function to convert Rust string to C string
fn to_c_string(s: &str) -> Result<CString, CoreBaseError> {
    CString::new(s).map_err(|e| CoreBaseError::InvalidString(e.to_string()))
}

/// Utility function to convert C string to Rust string
fn from_c_string(ptr: *const c_char) -> Result<String, CoreBaseError> {
    if ptr.is_null() {
        return Ok(String::new());
    }
    
    unsafe {
        CStr::from_ptr(ptr)
            .to_str()
            .map(|s| s.to_string())
            .map_err(|e| CoreBaseError::InvalidString(e.to_string()))
    }
}

/// Main CoreBase client for managing all functionality
#[derive(Debug)]
pub struct CoreBase {
    error_handler: ErrorHandler,
    config_manager: ConfigManager,
    network_manager: NetworkManager,
    system_monitor: SystemMonitor,
}

impl CoreBase {
    /// Create a new CoreBase instance
    /// 
    /// This will automatically initialize the library if not already done.
    pub fn new() -> Result<Self, CoreBaseError> {
        initialize()?;
        
        Ok(CoreBase {
            error_handler: ErrorHandler::new()?,
            config_manager: ConfigManager::new()?,
            network_manager: NetworkManager::new()?,
            system_monitor: SystemMonitor::new()?,
        })
    }
    
    /// Get a reference to the error handler
    pub fn error_handler(&self) -> &ErrorHandler {
        &self.error_handler
    }
    
    /// Get a mutable reference to the error handler
    pub fn error_handler_mut(&mut self) -> &mut ErrorHandler {
        &mut self.error_handler
    }
    
    /// Get a reference to the config manager
    pub fn config_manager(&self) -> &ConfigManager {
        &self.config_manager
    }
    
    /// Get a mutable reference to the config manager
    pub fn config_manager_mut(&mut self) -> &mut ConfigManager {
        &mut self.config_manager
    }
    
    /// Get a reference to the network manager
    pub fn network_manager(&self) -> &NetworkManager {
        &self.network_manager
    }
    
    /// Get a mutable reference to the network manager
    pub fn network_manager_mut(&mut self) -> &mut NetworkManager {
        &mut self.network_manager
    }
    
    /// Get a reference to the system monitor
    pub fn system_monitor(&self) -> &SystemMonitor {
        &self.system_monitor
    }
}

impl Drop for CoreBase {
    fn drop(&mut self) {
        // Shutdown will be called when the last CoreBase instance is dropped
        let _ = shutdown();
    }
}

/// Convenience macro for logging with automatic file/line information
#[macro_export]
macro_rules! cba_log {
    ($level:expr, $($arg:tt)*) => {
        if let Ok(mut cba) = $crate::CoreBase::new() {
            let message = format!($($arg)*);
            let _ = cba.error_handler_mut().log($level, &message);
        }
    };
}

/// Convenience macro for debug logging
#[macro_export]
macro_rules! cba_debug {
    ($($arg:tt)*) => {
        $crate::cba_log!($crate::LogLevel::Debug, $($arg)*)
    };
}

/// Convenience macro for info logging
#[macro_export]
macro_rules! cba_info {
    ($($arg:tt)*) => {
        $crate::cba_log!($crate::LogLevel::Info, $($arg)*)
    };
}

/// Convenience macro for warning logging
#[macro_export]
macro_rules! cba_warning {
    ($($arg:tt)*) => {
        $crate::cba_log!($crate::LogLevel::Warning, $($arg)*)
    };
}

/// Convenience macro for error logging
#[macro_export]
macro_rules! cba_error {
    ($($arg:tt)*) => {
        $crate::cba_log!($crate::LogLevel::Error, $($arg)*)
    };
}

/// Convenience macro for critical logging
#[macro_export]
macro_rules! cba_critical {
    ($($arg:tt)*) => {
        $crate::cba_log!($crate::LogLevel::Critical, $($arg)*)
    };
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_initialization() {
        assert!(initialize().is_ok());
        assert!(is_initialized());
        assert!(shutdown().is_ok());
    }
    
    #[test]
    fn test_corebase_creation() {
        let cba = CoreBase::new();
        assert!(cba.is_ok());
    }
    
    #[test]
    fn test_log_level_conversion() {
        assert_eq!(LogLevel::from(0), LogLevel::Debug);
        assert_eq!(LogLevel::from(1), LogLevel::Info);
        assert_eq!(LogLevel::from(2), LogLevel::Warning);
        assert_eq!(LogLevel::from(3), LogLevel::Error);
        assert_eq!(LogLevel::from(4), LogLevel::Critical);
        assert_eq!(LogLevel::from(999), LogLevel::Info); // Default fallback
        
        assert_eq!(c_int::from(LogLevel::Debug), 0);
        assert_eq!(c_int::from(LogLevel::Info), 1);
        assert_eq!(c_int::from(LogLevel::Warning), 2);
        assert_eq!(c_int::from(LogLevel::Error), 3);
        assert_eq!(c_int::from(LogLevel::Critical), 4);
    }
}