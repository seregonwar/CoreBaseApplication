//! Network management module for CoreBase Rust bindings
//!
//! This module provides network functionality that wraps the C++ NetworkManager class.

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::time::Duration;
use serde::{Deserialize, Serialize};

use crate::{to_c_string, from_c_string};
use crate::error::{CoreBaseError, CoreBaseResult};

/// Network protocol types matching the C++ NetworkProtocol enum
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum NetworkProtocol {
    TCP = 0,
    UDP = 1,
    HTTP = 2,
    HTTPS = 3,
    WebSocket = 4,
    MQTT = 5,
    AMQP = 6,
    GRPC = 7,
    Custom = 8,
}

impl From<c_int> for NetworkProtocol {
    fn from(value: c_int) -> Self {
        match value {
            0 => NetworkProtocol::TCP,
            1 => NetworkProtocol::UDP,
            2 => NetworkProtocol::HTTP,
            3 => NetworkProtocol::HTTPS,
            4 => NetworkProtocol::WebSocket,
            5 => NetworkProtocol::MQTT,
            6 => NetworkProtocol::AMQP,
            7 => NetworkProtocol::GRPC,
            8 => NetworkProtocol::Custom,
            _ => NetworkProtocol::TCP, // Default fallback
        }
    }
}

impl From<NetworkProtocol> for c_int {
    fn from(protocol: NetworkProtocol) -> Self {
        protocol as c_int
    }
}

/// Connection state
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting,
    Error,
}

/// Network configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkConfig {
    pub host: String,
    pub port: u16,
    pub protocol: NetworkProtocol,
    pub timeout_ms: u32,
    pub max_retries: u32,
    pub retry_delay_ms: u32,
    pub use_ssl: bool,
    pub verify_ssl: bool,
    pub username: Option<String>,
    pub password: Option<String>,
    pub headers: HashMap<String, String>,
    pub custom_params: HashMap<String, String>,
}

impl Default for NetworkConfig {
    fn default() -> Self {
        NetworkConfig {
            host: "localhost".to_string(),
            port: 8080,
            protocol: NetworkProtocol::TCP,
            timeout_ms: 5000,
            max_retries: 3,
            retry_delay_ms: 1000,
            use_ssl: false,
            verify_ssl: true,
            username: None,
            password: None,
            headers: HashMap::new(),
            custom_params: HashMap::new(),
        }
    }
}

impl NetworkConfig {
    /// Create a new TCP configuration
    pub fn tcp(host: &str, port: u16) -> Self {
        NetworkConfig {
            host: host.to_string(),
            port,
            protocol: NetworkProtocol::TCP,
            ..Default::default()
        }
    }
    
    /// Create a new UDP configuration
    pub fn udp(host: &str, port: u16) -> Self {
        NetworkConfig {
            host: host.to_string(),
            port,
            protocol: NetworkProtocol::UDP,
            ..Default::default()
        }
    }
    
    /// Create a new HTTP configuration
    pub fn http(host: &str, port: u16) -> Self {
        NetworkConfig {
            host: host.to_string(),
            port,
            protocol: NetworkProtocol::HTTP,
            ..Default::default()
        }
    }
    
    /// Create a new HTTPS configuration
    pub fn https(host: &str, port: u16) -> Self {
        NetworkConfig {
            host: host.to_string(),
            port,
            protocol: NetworkProtocol::HTTPS,
            use_ssl: true,
            ..Default::default()
        }
    }
    
    /// Set timeout
    pub fn with_timeout(mut self, timeout: Duration) -> Self {
        self.timeout_ms = timeout.as_millis() as u32;
        self
    }
    
    /// Set authentication
    pub fn with_auth(mut self, username: &str, password: &str) -> Self {
        self.username = Some(username.to_string());
        self.password = Some(password.to_string());
        self
    }
    
    /// Add header
    pub fn with_header(mut self, key: &str, value: &str) -> Self {
        self.headers.insert(key.to_string(), value.to_string());
        self
    }
    
    /// Add custom parameter
    pub fn with_param(mut self, key: &str, value: &str) -> Self {
        self.custom_params.insert(key.to_string(), value.to_string());
        self
    }
}

/// Network message
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkMessage {
    pub data: Vec<u8>,
    pub topic: Option<String>,
    pub headers: HashMap<String, String>,
    pub timestamp: u64,
    pub sender: Option<String>,
}

impl NetworkMessage {
    /// Create a new message with string data
    pub fn new_text(data: &str) -> Self {
        NetworkMessage {
            data: data.as_bytes().to_vec(),
            topic: None,
            headers: HashMap::new(),
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs(),
            sender: None,
        }
    }
    
    /// Create a new message with binary data
    pub fn new_binary(data: Vec<u8>) -> Self {
        NetworkMessage {
            data,
            topic: None,
            headers: HashMap::new(),
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs(),
            sender: None,
        }
    }
    
    /// Get data as string
    pub fn as_text(&self) -> CoreBaseResult<String> {
        String::from_utf8(self.data.clone())
            .map_err(|e| CoreBaseError::NetworkError(format!("Invalid UTF-8: {}", e)))
    }
    
    /// Get data as bytes
    pub fn as_bytes(&self) -> &[u8] {
        &self.data
    }
    
    /// Set topic
    pub fn with_topic(mut self, topic: &str) -> Self {
        self.topic = Some(topic.to_string());
        self
    }
    
    /// Add header
    pub fn with_header(mut self, key: &str, value: &str) -> Self {
        self.headers.insert(key.to_string(), value.to_string());
        self
    }
    
    /// Set sender
    pub fn with_sender(mut self, sender: &str) -> Self {
        self.sender = Some(sender.to_string());
        self
    }
}

/// Network connection handle
#[derive(Debug, Clone)]
pub struct NetworkConnection {
    pub id: String,
    pub config: NetworkConfig,
    pub state: ConnectionState,
}

impl NetworkConnection {
    /// Send a message through this connection
    pub fn send(&self, message: &NetworkMessage) -> CoreBaseResult<()> {
        let message_str = String::from_utf8(message.data.clone())
            .map_err(|e| CoreBaseError::NetworkError(format!("Invalid message data: {}", e)))?;
        
        let c_connection_id = to_c_string(&self.id)?;
        let c_message = to_c_string(&message_str)?;
        
        unsafe {
            let result = crate::cba_network_send_message(
                c_connection_id.as_ptr(),
                c_message.as_ptr(),
            );
            
            if result == 0 {
                Ok(())
            } else {
                Err(CoreBaseError::NetworkError(
                    "Failed to send message".to_string()
                ))
            }
        }
    }
    
    /// Receive a message from this connection
    pub fn receive(&self) -> CoreBaseResult<NetworkMessage> {
        let c_connection_id = to_c_string(&self.id)?;
        let mut buffer = vec![0u8; 4096]; // 4KB buffer
        
        unsafe {
            let result = crate::cba_network_receive_message(
                c_connection_id.as_ptr(),
                buffer.as_mut_ptr() as *mut c_char,
                buffer.len() as c_int,
            );
            
            if result == 0 {
                // Find the null terminator
                let null_pos = buffer.iter().position(|&x| x == 0).unwrap_or(buffer.len());
                let data = buffer[..null_pos].to_vec();
                
                Ok(NetworkMessage {
                    data,
                    topic: None,
                    headers: HashMap::new(),
                    timestamp: std::time::SystemTime::now()
                        .duration_since(std::time::UNIX_EPOCH)
                        .unwrap_or_default()
                        .as_secs(),
                    sender: None,
                })
            } else {
                Err(CoreBaseError::NetworkError(
                    "Failed to receive message".to_string()
                ))
            }
        }
    }
    
    /// Close this connection
    pub fn close(&self) -> CoreBaseResult<()> {
        let c_connection_id = to_c_string(&self.id)?;
        
        unsafe {
            let result = crate::cba_network_close_connection(c_connection_id.as_ptr());
            if result == 0 {
                Ok(())
            } else {
                Err(CoreBaseError::NetworkError(
                    "Failed to close connection".to_string()
                ))
            }
        }
    }
}

/// Network manager wrapper for the C++ NetworkManager class
#[derive(Debug)]
pub struct NetworkManager {
    initialized: bool,
    connections: Arc<Mutex<HashMap<String, NetworkConnection>>>,
}

impl NetworkManager {
    /// Create a new NetworkManager instance
    pub fn new() -> CoreBaseResult<Self> {
        Ok(NetworkManager {
            initialized: true,
            connections: Arc::new(Mutex::new(HashMap::new())),
        })
    }
    
    /// Create a new network connection
    pub fn create_connection(&self, config: NetworkConfig) -> CoreBaseResult<NetworkConnection> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "NetworkManager not initialized".to_string()
            ));
        }
        
        let c_host = to_c_string(&config.host)?;
        
        unsafe {
            let connection_id_ptr = crate::cba_network_create_connection(
                c_host.as_ptr(),
                config.port as c_int,
                config.protocol.into(),
            );
            
            if connection_id_ptr.is_null() {
                return Err(CoreBaseError::NetworkError(
                    "Failed to create network connection".to_string()
                ));
            }
            
            let connection_id = from_c_string(connection_id_ptr)?;
            
            let connection = NetworkConnection {
                id: connection_id.clone(),
                config: config.clone(),
                state: ConnectionState::Connected,
            };
            
            // Store connection in our map
            if let Ok(mut connections) = self.connections.lock() {
                connections.insert(connection_id.clone(), connection.clone());
            }
            
            Ok(connection)
        }
    }
    
    /// Get an existing connection by ID
    pub fn get_connection(&self, connection_id: &str) -> CoreBaseResult<NetworkConnection> {
        if let Ok(connections) = self.connections.lock() {
            connections.get(connection_id)
                .cloned()
                .ok_or_else(|| CoreBaseError::ResourceNotFound(
                    format!("Connection not found: {}", connection_id)
                ))
        } else {
            Err(CoreBaseError::OperationFailed(
                "Failed to access connections".to_string()
            ))
        }
    }
    
    /// List all active connections
    pub fn list_connections(&self) -> CoreBaseResult<Vec<NetworkConnection>> {
        if let Ok(connections) = self.connections.lock() {
            Ok(connections.values().cloned().collect())
        } else {
            Err(CoreBaseError::OperationFailed(
                "Failed to access connections".to_string()
            ))
        }
    }
    
    /// Close a connection by ID
    pub fn close_connection(&self, connection_id: &str) -> CoreBaseResult<()> {
        if let Ok(connection) = self.get_connection(connection_id) {
            connection.close()?;
            
            // Remove from our map
            if let Ok(mut connections) = self.connections.lock() {
                connections.remove(connection_id);
            }
            
            Ok(())
        } else {
            Err(CoreBaseError::ResourceNotFound(
                format!("Connection not found: {}", connection_id)
            ))
        }
    }
    
    /// Close all connections
    pub fn close_all_connections(&self) -> CoreBaseResult<()> {
        let connection_ids: Vec<String> = if let Ok(connections) = self.connections.lock() {
            connections.keys().cloned().collect()
        } else {
            return Err(CoreBaseError::OperationFailed(
                "Failed to access connections".to_string()
            ));
        };
        
        for connection_id in connection_ids {
            let _ = self.close_connection(&connection_id); // Continue even if some fail
        }
        
        Ok(())
    }
    
    /// Send a message to a specific connection
    pub fn send_message(&self, connection_id: &str, message: &NetworkMessage) -> CoreBaseResult<()> {
        let connection = self.get_connection(connection_id)?;
        connection.send(message)
    }
    
    /// Receive a message from a specific connection
    pub fn receive_message(&self, connection_id: &str) -> CoreBaseResult<NetworkMessage> {
        let connection = self.get_connection(connection_id)?;
        connection.receive()
    }
    
    /// Broadcast a message to all connections
    pub fn broadcast_message(&self, message: &NetworkMessage) -> CoreBaseResult<Vec<String>> {
        let connections = self.list_connections()?;
        let mut failed_connections = Vec::new();
        
        for connection in connections {
            if let Err(_) = connection.send(message) {
                failed_connections.push(connection.id);
            }
        }
        
        Ok(failed_connections)
    }
    
    /// Get connection count
    pub fn connection_count(&self) -> usize {
        if let Ok(connections) = self.connections.lock() {
            connections.len()
        } else {
            0
        }
    }
}

impl Default for NetworkManager {
    fn default() -> Self {
        Self::new().unwrap_or(NetworkManager {
            initialized: false,
            connections: Arc::new(Mutex::new(HashMap::new())),
        })
    }
}

impl Drop for NetworkManager {
    fn drop(&mut self) {
        // Close all connections when dropping
        let _ = self.close_all_connections();
    }
}

/// Async network operations (requires "async" feature)
#[cfg(feature = "async")]
pub mod async_ops {
    use super::*;
    use tokio::time::{timeout, Duration};
    
    impl NetworkManager {
        /// Async version of create_connection
        pub async fn create_connection_async(&self, config: NetworkConfig) -> CoreBaseResult<NetworkConnection> {
            let timeout_duration = Duration::from_millis(config.timeout_ms as u64);
            
            timeout(timeout_duration, async {
                // In a real implementation, this would be truly async
                // For now, we'll use the sync version
                self.create_connection(config)
            })
            .await
            .map_err(|_| CoreBaseError::Timeout("Connection timeout".to_string()))?
        }
        
        /// Async version of send_message
        pub async fn send_message_async(
            &self,
            connection_id: &str,
            message: &NetworkMessage,
        ) -> CoreBaseResult<()> {
            let connection = self.get_connection(connection_id)?;
            
            timeout(Duration::from_millis(5000), async {
                connection.send(message)
            })
            .await
            .map_err(|_| CoreBaseError::Timeout("Send timeout".to_string()))?
        }
        
        /// Async version of receive_message
        pub async fn receive_message_async(
            &self,
            connection_id: &str,
        ) -> CoreBaseResult<NetworkMessage> {
            let connection = self.get_connection(connection_id)?;
            
            timeout(Duration::from_millis(5000), async {
                connection.receive()
            })
            .await
            .map_err(|_| CoreBaseError::Timeout("Receive timeout".to_string()))?
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_network_manager_creation() {
        let manager = NetworkManager::new();
        assert!(manager.is_ok());
        assert!(manager.unwrap().initialized);
    }
    
    #[test]
    fn test_network_config_builders() {
        let tcp_config = NetworkConfig::tcp("localhost", 8080);
        assert_eq!(tcp_config.protocol, NetworkProtocol::TCP);
        assert_eq!(tcp_config.host, "localhost");
        assert_eq!(tcp_config.port, 8080);
        
        let https_config = NetworkConfig::https("example.com", 443)
            .with_timeout(Duration::from_secs(10))
            .with_auth("user", "pass")
            .with_header("Content-Type", "application/json");
        
        assert_eq!(https_config.protocol, NetworkProtocol::HTTPS);
        assert!(https_config.use_ssl);
        assert_eq!(https_config.timeout_ms, 10000);
        assert_eq!(https_config.username, Some("user".to_string()));
        assert_eq!(https_config.headers.get("Content-Type"), Some(&"application/json".to_string()));
    }
    
    #[test]
    fn test_network_message() {
        let message = NetworkMessage::new_text("Hello, World!")
            .with_topic("test/topic")
            .with_header("Content-Type", "text/plain")
            .with_sender("test_sender");
        
        assert_eq!(message.as_text().unwrap(), "Hello, World!");
        assert_eq!(message.topic, Some("test/topic".to_string()));
        assert_eq!(message.headers.get("Content-Type"), Some(&"text/plain".to_string()));
        assert_eq!(message.sender, Some("test_sender".to_string()));
    }
    
    #[test]
    fn test_protocol_conversion() {
        assert_eq!(NetworkProtocol::from(0), NetworkProtocol::TCP);
        assert_eq!(NetworkProtocol::from(2), NetworkProtocol::HTTP);
        assert_eq!(NetworkProtocol::from(999), NetworkProtocol::TCP); // Default fallback
        
        assert_eq!(c_int::from(NetworkProtocol::UDP), 1);
        assert_eq!(c_int::from(NetworkProtocol::HTTPS), 3);
    }
    
    #[test]
    fn test_default_network_manager() {
        let manager = NetworkManager::default();
        assert_eq!(manager.connection_count(), 0);
    }
}