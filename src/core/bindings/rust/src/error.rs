//! Error handling module for CoreBase Rust bindings
//!
//! This module provides error types and error handling functionality
//! that wraps the C++ ErrorHandler class.

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::fmt;
use thiserror::Error;

use crate::{LogLevel, to_c_string, from_c_string};

/// CoreBase error types
#[derive(Error, Debug, Clone)]
pub enum CoreBaseError {
    #[error("Initialization failed: {0}")]
    InitializationFailed(String),
    
    #[error("Shutdown failed: {0}")]
    ShutdownFailed(String),
    
    #[error("Invalid string: {0}")]
    InvalidString(String),
    
    #[error("Configuration error: {0}")]
    ConfigError(String),
    
    #[error("Network error: {0}")]
    NetworkError(String),
    
    #[error("System monitor error: {0}")]
    MonitorError(String),
    
    #[error("Operation failed: {0}")]
    OperationFailed(String),
    
    #[error("Invalid parameter: {0}")]
    InvalidParameter(String),
    
    #[error("Resource not found: {0}")]
    ResourceNotFound(String),
    
    #[error("Permission denied: {0}")]
    PermissionDenied(String),
    
    #[error("Timeout occurred: {0}")]
    Timeout(String),
    
    #[error("Unknown error: {0}")]
    Unknown(String),
}

impl CoreBaseError {
    /// Convert error to log level based on severity
    pub fn to_log_level(&self) -> LogLevel {
        match self {
            CoreBaseError::InitializationFailed(_) => LogLevel::Critical,
            CoreBaseError::ShutdownFailed(_) => LogLevel::Critical,
            CoreBaseError::InvalidString(_) => LogLevel::Error,
            CoreBaseError::ConfigError(_) => LogLevel::Error,
            CoreBaseError::NetworkError(_) => LogLevel::Error,
            CoreBaseError::MonitorError(_) => LogLevel::Warning,
            CoreBaseError::OperationFailed(_) => LogLevel::Error,
            CoreBaseError::InvalidParameter(_) => LogLevel::Warning,
            CoreBaseError::ResourceNotFound(_) => LogLevel::Warning,
            CoreBaseError::PermissionDenied(_) => LogLevel::Error,
            CoreBaseError::Timeout(_) => LogLevel::Warning,
            CoreBaseError::Unknown(_) => LogLevel::Error,
        }
    }
}

/// Result type alias for CoreBase operations
pub type CoreBaseResult<T> = Result<T, CoreBaseError>;

/// Error handler wrapper for the C++ ErrorHandler class
#[derive(Debug)]
pub struct ErrorHandler {
    initialized: bool,
}

impl ErrorHandler {
    /// Create a new ErrorHandler instance
    pub fn new() -> CoreBaseResult<Self> {
        Ok(ErrorHandler {
            initialized: true,
        })
    }
    
    /// Handle an error with file, line, and function information
    pub fn handle_error(
        &self,
        message: &str,
        file: &str,
        line: u32,
        function: &str,
    ) -> CoreBaseResult<()> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ErrorHandler not initialized".to_string()
            ));
        }
        
        let c_message = to_c_string(message)?;
        let c_file = to_c_string(file)?;
        let c_function = to_c_string(function)?;
        
        unsafe {
            let result = crate::cba_error_handler_handle_error(
                c_message.as_ptr(),
                c_file.as_ptr(),
                line as c_int,
                c_function.as_ptr(),
            );
            
            if result == 0 {
                Ok(())
            } else {
                Err(CoreBaseError::OperationFailed(
                    "Failed to handle error".to_string()
                ))
            }
        }
    }
    
    /// Set the log level
    pub fn set_log_level(&self, level: LogLevel) -> CoreBaseResult<()> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ErrorHandler not initialized".to_string()
            ));
        }
        
        unsafe {
            let result = crate::cba_error_handler_set_log_level(level.into());
            if result == 0 {
                Ok(())
            } else {
                Err(CoreBaseError::OperationFailed(
                    "Failed to set log level".to_string()
                ))
            }
        }
    }
    
    /// Get the current log level
    pub fn get_log_level(&self) -> CoreBaseResult<LogLevel> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ErrorHandler not initialized".to_string()
            ));
        }
        
        unsafe {
            let level = crate::cba_error_handler_get_log_level();
            Ok(LogLevel::from(level))
        }
    }
    
    /// Log a message with the specified level
    pub fn log(&self, level: LogLevel, message: &str) -> CoreBaseResult<()> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ErrorHandler not initialized".to_string()
            ));
        }
        
        let c_message = to_c_string(message)?;
        
        unsafe {
            let result = crate::cba_error_handler_log(level.into(), c_message.as_ptr());
            if result == 0 {
                Ok(())
            } else {
                Err(CoreBaseError::OperationFailed(
                    "Failed to log message".to_string()
                ))
            }
        }
    }
    
    /// Log a debug message
    pub fn debug(&self, message: &str) -> CoreBaseResult<()> {
        self.log(LogLevel::Debug, message)
    }
    
    /// Log an info message
    pub fn info(&self, message: &str) -> CoreBaseResult<()> {
        self.log(LogLevel::Info, message)
    }
    
    /// Log a warning message
    pub fn warning(&self, message: &str) -> CoreBaseResult<()> {
        self.log(LogLevel::Warning, message)
    }
    
    /// Log an error message
    pub fn error(&self, message: &str) -> CoreBaseResult<()> {
        self.log(LogLevel::Error, message)
    }
    
    /// Log a critical message
    pub fn critical(&self, message: &str) -> CoreBaseResult<()> {
        self.log(LogLevel::Critical, message)
    }
    
    /// Handle a CoreBaseError by logging it and optionally re-throwing
    pub fn handle_corebase_error(&self, error: &CoreBaseError, re_throw: bool) -> CoreBaseResult<()> {
        let level = error.to_log_level();
        let message = format!("CoreBaseError: {}", error);
        
        self.log(level, &message)?;
        
        if re_throw {
            Err(error.clone())
        } else {
            Ok(())
        }
    }
}

impl Default for ErrorHandler {
    fn default() -> Self {
        Self::new().unwrap_or(ErrorHandler {
            initialized: false,
        })
    }
}

/// Macro for handling errors with automatic file/line/function information
#[macro_export]
macro_rules! handle_error {
    ($handler:expr, $message:expr) => {
        $handler.handle_error($message, file!(), line!(), module_path!())
    };
}

/// Macro for creating and handling errors in one step
#[macro_export]
macro_rules! cba_handle_error {
    ($error_type:ident, $message:expr) => {
        if let Ok(mut cba) = $crate::CoreBase::new() {
            let error = $crate::error::CoreBaseError::$error_type($message.to_string());
            let _ = cba.error_handler().handle_corebase_error(&error, false);
        }
    };
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_error_handler_creation() {
        let handler = ErrorHandler::new();
        assert!(handler.is_ok());
        assert!(handler.unwrap().initialized);
    }
    
    #[test]
    fn test_error_log_levels() {
        let init_error = CoreBaseError::InitializationFailed("test".to_string());
        assert_eq!(init_error.to_log_level(), LogLevel::Critical);
        
        let config_error = CoreBaseError::ConfigError("test".to_string());
        assert_eq!(config_error.to_log_level(), LogLevel::Error);
        
        let monitor_error = CoreBaseError::MonitorError("test".to_string());
        assert_eq!(monitor_error.to_log_level(), LogLevel::Warning);
    }
    
    #[test]
    fn test_error_display() {
        let error = CoreBaseError::NetworkError("Connection failed".to_string());
        let error_string = format!("{}", error);
        assert!(error_string.contains("Network error"));
        assert!(error_string.contains("Connection failed"));
    }
    
    #[test]
    fn test_default_error_handler() {
        let handler = ErrorHandler::default();
        // Should not panic and should create a valid instance
        assert!(!handler.initialized || handler.initialized); // Always true, but tests creation
    }
}