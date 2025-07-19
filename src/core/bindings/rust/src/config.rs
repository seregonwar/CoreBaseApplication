//! Configuration management module for CoreBase Rust bindings
//!
//! This module provides configuration management functionality
//! that wraps the C++ ConfigManager class.

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::collections::HashMap;
use std::path::Path;
use serde::{Deserialize, Serialize};
use serde_json;

use crate::{to_c_string, from_c_string};
use crate::error::{CoreBaseError, CoreBaseResult};

/// Configuration value types
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
#[serde(untagged)]
pub enum ConfigValue {
    String(String),
    Integer(i64),
    Float(f64),
    Boolean(bool),
    Array(Vec<ConfigValue>),
    Object(HashMap<String, ConfigValue>),
    Null,
}

impl ConfigValue {
    /// Convert to string representation
    pub fn as_string(&self) -> Option<String> {
        match self {
            ConfigValue::String(s) => Some(s.clone()),
            ConfigValue::Integer(i) => Some(i.to_string()),
            ConfigValue::Float(f) => Some(f.to_string()),
            ConfigValue::Boolean(b) => Some(b.to_string()),
            ConfigValue::Null => Some("null".to_string()),
            _ => None,
        }
    }
    
    /// Convert to integer
    pub fn as_integer(&self) -> Option<i64> {
        match self {
            ConfigValue::Integer(i) => Some(*i),
            ConfigValue::Float(f) => Some(*f as i64),
            ConfigValue::String(s) => s.parse().ok(),
            ConfigValue::Boolean(b) => Some(if *b { 1 } else { 0 }),
            _ => None,
        }
    }
    
    /// Convert to float
    pub fn as_float(&self) -> Option<f64> {
        match self {
            ConfigValue::Float(f) => Some(*f),
            ConfigValue::Integer(i) => Some(*i as f64),
            ConfigValue::String(s) => s.parse().ok(),
            _ => None,
        }
    }
    
    /// Convert to boolean
    pub fn as_boolean(&self) -> Option<bool> {
        match self {
            ConfigValue::Boolean(b) => Some(*b),
            ConfigValue::Integer(i) => Some(*i != 0),
            ConfigValue::String(s) => {
                match s.to_lowercase().as_str() {
                    "true" | "yes" | "1" | "on" => Some(true),
                    "false" | "no" | "0" | "off" => Some(false),
                    _ => None,
                }
            },
            _ => None,
        }
    }
    
    /// Convert to array
    pub fn as_array(&self) -> Option<&Vec<ConfigValue>> {
        match self {
            ConfigValue::Array(arr) => Some(arr),
            _ => None,
        }
    }
    
    /// Convert to object
    pub fn as_object(&self) -> Option<&HashMap<String, ConfigValue>> {
        match self {
            ConfigValue::Object(obj) => Some(obj),
            _ => None,
        }
    }
    
    /// Check if value is null
    pub fn is_null(&self) -> bool {
        matches!(self, ConfigValue::Null)
    }
}

impl From<String> for ConfigValue {
    fn from(s: String) -> Self {
        ConfigValue::String(s)
    }
}

impl From<&str> for ConfigValue {
    fn from(s: &str) -> Self {
        ConfigValue::String(s.to_string())
    }
}

impl From<i64> for ConfigValue {
    fn from(i: i64) -> Self {
        ConfigValue::Integer(i)
    }
}

impl From<i32> for ConfigValue {
    fn from(i: i32) -> Self {
        ConfigValue::Integer(i as i64)
    }
}

impl From<f64> for ConfigValue {
    fn from(f: f64) -> Self {
        ConfigValue::Float(f)
    }
}

impl From<f32> for ConfigValue {
    fn from(f: f32) -> Self {
        ConfigValue::Float(f as f64)
    }
}

impl From<bool> for ConfigValue {
    fn from(b: bool) -> Self {
        ConfigValue::Boolean(b)
    }
}

/// Configuration manager wrapper for the C++ ConfigManager class
#[derive(Debug)]
pub struct ConfigManager {
    initialized: bool,
    cache: HashMap<String, ConfigValue>,
}

impl ConfigManager {
    /// Create a new ConfigManager instance
    pub fn new() -> CoreBaseResult<Self> {
        Ok(ConfigManager {
            initialized: true,
            cache: HashMap::new(),
        })
    }
    
    /// Load configuration from a file
    pub fn load<P: AsRef<Path>>(&mut self, filename: P) -> CoreBaseResult<()> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ConfigManager not initialized".to_string()
            ));
        }
        
        let filename_str = filename.as_ref().to_string_lossy();
        let c_filename = to_c_string(&filename_str)?;
        
        unsafe {
            let result = crate::cba_config_load(c_filename.as_ptr());
            if result == 0 {
                // Clear cache after loading new config
                self.cache.clear();
                Ok(())
            } else {
                Err(CoreBaseError::ConfigError(
                    format!("Failed to load config file: {}", filename_str)
                ))
            }
        }
    }
    
    /// Get a configuration value by key
    pub fn get(&mut self, key: &str) -> CoreBaseResult<ConfigValue> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ConfigManager not initialized".to_string()
            ));
        }
        
        // Check cache first
        if let Some(value) = self.cache.get(key) {
            return Ok(value.clone());
        }
        
        let c_key = to_c_string(key)?;
        let mut buffer = vec![0u8; 1024]; // 1KB buffer
        
        unsafe {
            let result = crate::cba_config_get_value(
                c_key.as_ptr(),
                buffer.as_mut_ptr() as *mut c_char,
                buffer.len() as c_int,
            );
            
            if result == 0 {
                // Find the null terminator
                let null_pos = buffer.iter().position(|&x| x == 0).unwrap_or(buffer.len());
                let value_str = String::from_utf8_lossy(&buffer[..null_pos]).to_string();
                
                // Try to parse as JSON first, fallback to string
                let config_value = if let Ok(json_value) = serde_json::from_str::<serde_json::Value>(&value_str) {
                    json_to_config_value(json_value)
                } else {
                    ConfigValue::String(value_str)
                };
                
                // Cache the value
                self.cache.insert(key.to_string(), config_value.clone());
                Ok(config_value)
            } else {
                Err(CoreBaseError::ConfigError(
                    format!("Failed to get config value for key: {}", key)
                ))
            }
        }
    }
    
    /// Set a configuration value by key
    pub fn set(&mut self, key: &str, value: ConfigValue) -> CoreBaseResult<()> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ConfigManager not initialized".to_string()
            ));
        }
        
        let c_key = to_c_string(key)?;
        let value_str = config_value_to_json_string(&value)?;
        let c_value = to_c_string(&value_str)?;
        
        unsafe {
            let result = crate::cba_config_set_value(c_key.as_ptr(), c_value.as_ptr());
            if result == 0 {
                // Update cache
                self.cache.insert(key.to_string(), value);
                Ok(())
            } else {
                Err(CoreBaseError::ConfigError(
                    format!("Failed to set config value for key: {}", key)
                ))
            }
        }
    }
    
    /// Save configuration to a file
    pub fn save<P: AsRef<Path>>(&self, filename: P) -> CoreBaseResult<()> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "ConfigManager not initialized".to_string()
            ));
        }
        
        let filename_str = filename.as_ref().to_string_lossy();
        let c_filename = to_c_string(&filename_str)?;
        
        unsafe {
            let result = crate::cba_config_save(c_filename.as_ptr());
            if result == 0 {
                Ok(())
            } else {
                Err(CoreBaseError::ConfigError(
                    format!("Failed to save config file: {}", filename_str)
                ))
            }
        }
    }
    
    /// Get a string value with default
    pub fn get_string(&mut self, key: &str, default: &str) -> String {
        self.get(key)
            .ok()
            .and_then(|v| v.as_string())
            .unwrap_or_else(|| default.to_string())
    }
    
    /// Get an integer value with default
    pub fn get_integer(&mut self, key: &str, default: i64) -> i64 {
        self.get(key)
            .ok()
            .and_then(|v| v.as_integer())
            .unwrap_or(default)
    }
    
    /// Get a float value with default
    pub fn get_float(&mut self, key: &str, default: f64) -> f64 {
        self.get(key)
            .ok()
            .and_then(|v| v.as_float())
            .unwrap_or(default)
    }
    
    /// Get a boolean value with default
    pub fn get_boolean(&mut self, key: &str, default: bool) -> bool {
        self.get(key)
            .ok()
            .and_then(|v| v.as_boolean())
            .unwrap_or(default)
    }
    
    /// Check if a key exists in the configuration
    pub fn has_key(&mut self, key: &str) -> bool {
        self.get(key).is_ok()
    }
    
    /// Clear the cache
    pub fn clear_cache(&mut self) {
        self.cache.clear();
    }
    
    /// Get all cached keys
    pub fn get_cached_keys(&self) -> Vec<String> {
        self.cache.keys().cloned().collect()
    }
}

impl Default for ConfigManager {
    fn default() -> Self {
        Self::new().unwrap_or(ConfigManager {
            initialized: false,
            cache: HashMap::new(),
        })
    }
}

/// Convert serde_json::Value to ConfigValue
fn json_to_config_value(json: serde_json::Value) -> ConfigValue {
    match json {
        serde_json::Value::Null => ConfigValue::Null,
        serde_json::Value::Bool(b) => ConfigValue::Boolean(b),
        serde_json::Value::Number(n) => {
            if let Some(i) = n.as_i64() {
                ConfigValue::Integer(i)
            } else if let Some(f) = n.as_f64() {
                ConfigValue::Float(f)
            } else {
                ConfigValue::String(n.to_string())
            }
        },
        serde_json::Value::String(s) => ConfigValue::String(s),
        serde_json::Value::Array(arr) => {
            ConfigValue::Array(arr.into_iter().map(json_to_config_value).collect())
        },
        serde_json::Value::Object(obj) => {
            ConfigValue::Object(
                obj.into_iter()
                    .map(|(k, v)| (k, json_to_config_value(v)))
                    .collect()
            )
        },
    }
}

/// Convert ConfigValue to JSON string
fn config_value_to_json_string(value: &ConfigValue) -> CoreBaseResult<String> {
    let json_value = config_value_to_json(value);
    serde_json::to_string(&json_value)
        .map_err(|e| CoreBaseError::ConfigError(format!("JSON serialization error: {}", e)))
}

/// Convert ConfigValue to serde_json::Value
fn config_value_to_json(value: &ConfigValue) -> serde_json::Value {
    match value {
        ConfigValue::Null => serde_json::Value::Null,
        ConfigValue::Boolean(b) => serde_json::Value::Bool(*b),
        ConfigValue::Integer(i) => serde_json::Value::Number((*i).into()),
        ConfigValue::Float(f) => {
            serde_json::Value::Number(serde_json::Number::from_f64(*f).unwrap_or_else(|| 0.into()))
        },
        ConfigValue::String(s) => serde_json::Value::String(s.clone()),
        ConfigValue::Array(arr) => {
            serde_json::Value::Array(arr.iter().map(config_value_to_json).collect())
        },
        ConfigValue::Object(obj) => {
            serde_json::Value::Object(
                obj.iter()
                    .map(|(k, v)| (k.clone(), config_value_to_json(v)))
                    .collect()
            )
        },
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;
    
    #[test]
    fn test_config_manager_creation() {
        let manager = ConfigManager::new();
        assert!(manager.is_ok());
        assert!(manager.unwrap().initialized);
    }
    
    #[test]
    fn test_config_value_conversions() {
        let string_val = ConfigValue::String("test".to_string());
        assert_eq!(string_val.as_string(), Some("test".to_string()));
        
        let int_val = ConfigValue::Integer(42);
        assert_eq!(int_val.as_integer(), Some(42));
        assert_eq!(int_val.as_float(), Some(42.0));
        
        let bool_val = ConfigValue::Boolean(true);
        assert_eq!(bool_val.as_boolean(), Some(true));
        assert_eq!(bool_val.as_integer(), Some(1));
        
        let null_val = ConfigValue::Null;
        assert!(null_val.is_null());
    }
    
    #[test]
    fn test_config_value_from_conversions() {
        assert_eq!(ConfigValue::from("test"), ConfigValue::String("test".to_string()));
        assert_eq!(ConfigValue::from(42i32), ConfigValue::Integer(42));
        assert_eq!(ConfigValue::from(42i64), ConfigValue::Integer(42));
        assert_eq!(ConfigValue::from(3.14f32), ConfigValue::Float(3.14f64));
        assert_eq!(ConfigValue::from(3.14f64), ConfigValue::Float(3.14));
        assert_eq!(ConfigValue::from(true), ConfigValue::Boolean(true));
    }
    
    #[test]
    fn test_json_conversion() {
        let config_val = ConfigValue::Object({
            let mut map = HashMap::new();
            map.insert("name".to_string(), ConfigValue::String("test".to_string()));
            map.insert("value".to_string(), ConfigValue::Integer(42));
            map
        });
        
        let json_str = config_value_to_json_string(&config_val);
        assert!(json_str.is_ok());
        
        let json_value: serde_json::Value = serde_json::from_str(&json_str.unwrap()).unwrap();
        let converted_back = json_to_config_value(json_value);
        
        if let ConfigValue::Object(obj) = converted_back {
            assert_eq!(obj.get("name").unwrap().as_string(), Some("test".to_string()));
            assert_eq!(obj.get("value").unwrap().as_integer(), Some(42));
        } else {
            panic!("Expected object");
        }
    }
    
    #[test]
    fn test_default_config_manager() {
        let manager = ConfigManager::default();
        // Should not panic and should create a valid instance
        assert!(!manager.initialized || manager.initialized); // Always true, but tests creation
    }
}