//! System monitoring module for CoreBase Rust bindings
//!
//! This module provides system monitoring functionality that wraps the C++ SystemMonitor class.

use std::os::raw::c_double;
use std::time::{Duration, Instant};
use std::collections::VecDeque;
use serde::{Deserialize, Serialize};

use crate::error::{CoreBaseError, CoreBaseResult};

/// System resource usage information
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct SystemResources {
    pub cpu_usage_percent: f64,
    pub available_memory_bytes: f64,
    pub total_memory_bytes: f64,
    pub available_disk_bytes: f64,
    pub total_disk_bytes: f64,
    pub network_usage_percent: f64,
    pub gpu_usage_percent: f64,
    pub timestamp: u64,
}

impl SystemResources {
    /// Get memory usage percentage
    pub fn memory_usage_percent(&self) -> f64 {
        if self.total_memory_bytes > 0.0 {
            ((self.total_memory_bytes - self.available_memory_bytes) / self.total_memory_bytes) * 100.0
        } else {
            0.0
        }
    }
    
    /// Get disk usage percentage
    pub fn disk_usage_percent(&self) -> f64 {
        if self.total_disk_bytes > 0.0 {
            ((self.total_disk_bytes - self.available_disk_bytes) / self.total_disk_bytes) * 100.0
        } else {
            0.0
        }
    }
    
    /// Get used memory in bytes
    pub fn used_memory_bytes(&self) -> f64 {
        self.total_memory_bytes - self.available_memory_bytes
    }
    
    /// Get used disk space in bytes
    pub fn used_disk_bytes(&self) -> f64 {
        self.total_disk_bytes - self.available_disk_bytes
    }
    
    /// Convert bytes to human-readable format
    pub fn format_bytes(bytes: f64) -> String {
        const UNITS: &[&str] = &["B", "KB", "MB", "GB", "TB", "PB"];
        let mut size = bytes;
        let mut unit_index = 0;
        
        while size >= 1024.0 && unit_index < UNITS.len() - 1 {
            size /= 1024.0;
            unit_index += 1;
        }
        
        format!("{:.2} {}", size, UNITS[unit_index])
    }
    
    /// Get formatted memory usage
    pub fn format_memory_usage(&self) -> String {
        format!(
            "{} / {} ({:.1}%)",
            Self::format_bytes(self.used_memory_bytes()),
            Self::format_bytes(self.total_memory_bytes),
            self.memory_usage_percent()
        )
    }
    
    /// Get formatted disk usage
    pub fn format_disk_usage(&self) -> String {
        format!(
            "{} / {} ({:.1}%)",
            Self::format_bytes(self.used_disk_bytes()),
            Self::format_bytes(self.total_disk_bytes),
            self.disk_usage_percent()
        )
    }
}

impl Default for SystemResources {
    fn default() -> Self {
        SystemResources {
            cpu_usage_percent: 0.0,
            available_memory_bytes: 0.0,
            total_memory_bytes: 0.0,
            available_disk_bytes: 0.0,
            total_disk_bytes: 0.0,
            network_usage_percent: 0.0,
            gpu_usage_percent: 0.0,
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs(),
        }
    }
}

/// Historical data point for monitoring trends
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MonitoringDataPoint {
    pub timestamp: u64,
    pub cpu_usage: f64,
    pub memory_usage: f64,
    pub disk_usage: f64,
    pub network_usage: f64,
    pub gpu_usage: f64,
}

impl From<&SystemResources> for MonitoringDataPoint {
    fn from(resources: &SystemResources) -> Self {
        MonitoringDataPoint {
            timestamp: resources.timestamp,
            cpu_usage: resources.cpu_usage_percent,
            memory_usage: resources.memory_usage_percent(),
            disk_usage: resources.disk_usage_percent(),
            network_usage: resources.network_usage_percent,
            gpu_usage: resources.gpu_usage_percent,
        }
    }
}

/// System monitoring configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MonitoringConfig {
    pub update_interval: Duration,
    pub history_size: usize,
    pub enable_cpu_monitoring: bool,
    pub enable_memory_monitoring: bool,
    pub enable_disk_monitoring: bool,
    pub enable_network_monitoring: bool,
    pub enable_gpu_monitoring: bool,
    pub cpu_threshold: f64,
    pub memory_threshold: f64,
    pub disk_threshold: f64,
    pub network_threshold: f64,
    pub gpu_threshold: f64,
}

impl Default for MonitoringConfig {
    fn default() -> Self {
        MonitoringConfig {
            update_interval: Duration::from_secs(1),
            history_size: 100,
            enable_cpu_monitoring: true,
            enable_memory_monitoring: true,
            enable_disk_monitoring: true,
            enable_network_monitoring: true,
            enable_gpu_monitoring: true,
            cpu_threshold: 80.0,
            memory_threshold: 85.0,
            disk_threshold: 90.0,
            network_threshold: 80.0,
            gpu_threshold: 80.0,
        }
    }
}

/// System monitor wrapper for the C++ SystemMonitor class
#[derive(Debug)]
pub struct SystemMonitor {
    initialized: bool,
    config: MonitoringConfig,
    history: VecDeque<MonitoringDataPoint>,
    last_update: Option<Instant>,
}

impl SystemMonitor {
    /// Create a new SystemMonitor instance
    pub fn new() -> CoreBaseResult<Self> {
        Ok(SystemMonitor {
            initialized: true,
            config: MonitoringConfig::default(),
            history: VecDeque::new(),
            last_update: None,
        })
    }
    
    /// Create a new SystemMonitor with custom configuration
    pub fn with_config(config: MonitoringConfig) -> CoreBaseResult<Self> {
        Ok(SystemMonitor {
            initialized: true,
            config,
            history: VecDeque::new(),
            last_update: None,
        })
    }
    
    /// Get current system resource usage
    pub fn get_system_resources(&mut self) -> CoreBaseResult<SystemResources> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "SystemMonitor not initialized".to_string()
            ));
        }
        
        let mut resources = SystemResources::default();
        
        // Get CPU usage
        if self.config.enable_cpu_monitoring {
            unsafe {
                resources.cpu_usage_percent = crate::cba_monitor_get_cpu_usage();
            }
        }
        
        // Get memory usage
        if self.config.enable_memory_monitoring {
            let mut available = 0.0;
            let mut total = 0.0;
            unsafe {
                let result = crate::cba_monitor_get_memory_usage(&mut available, &mut total);
                if result == 0 {
                    resources.available_memory_bytes = available;
                    resources.total_memory_bytes = total;
                }
            }
        }
        
        // Get disk usage
        if self.config.enable_disk_monitoring {
            let mut available = 0.0;
            let mut total = 0.0;
            unsafe {
                let result = crate::cba_monitor_get_disk_usage(&mut available, &mut total);
                if result == 0 {
                    resources.available_disk_bytes = available;
                    resources.total_disk_bytes = total;
                }
            }
        }
        
        // Get network usage
        if self.config.enable_network_monitoring {
            unsafe {
                resources.network_usage_percent = crate::cba_monitor_get_network_usage();
            }
        }
        
        // Get GPU usage
        if self.config.enable_gpu_monitoring {
            unsafe {
                resources.gpu_usage_percent = crate::cba_monitor_get_gpu_usage();
            }
        }
        
        // Update timestamp
        resources.timestamp = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        
        // Add to history
        self.add_to_history(&resources);
        self.last_update = Some(Instant::now());
        
        Ok(resources)
    }
    
    /// Get CPU usage percentage
    pub fn get_cpu_usage(&self) -> CoreBaseResult<f64> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "SystemMonitor not initialized".to_string()
            ));
        }
        
        unsafe {
            Ok(crate::cba_monitor_get_cpu_usage())
        }
    }
    
    /// Get memory usage information
    pub fn get_memory_usage(&self) -> CoreBaseResult<(f64, f64)> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "SystemMonitor not initialized".to_string()
            ));
        }
        
        let mut available = 0.0;
        let mut total = 0.0;
        
        unsafe {
            let result = crate::cba_monitor_get_memory_usage(&mut available, &mut total);
            if result == 0 {
                Ok((available, total))
            } else {
                Err(CoreBaseError::MonitorError(
                    "Failed to get memory usage".to_string()
                ))
            }
        }
    }
    
    /// Get disk usage information
    pub fn get_disk_usage(&self) -> CoreBaseResult<(f64, f64)> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "SystemMonitor not initialized".to_string()
            ));
        }
        
        let mut available = 0.0;
        let mut total = 0.0;
        
        unsafe {
            let result = crate::cba_monitor_get_disk_usage(&mut available, &mut total);
            if result == 0 {
                Ok((available, total))
            } else {
                Err(CoreBaseError::MonitorError(
                    "Failed to get disk usage".to_string()
                ))
            }
        }
    }
    
    /// Get network usage percentage
    pub fn get_network_usage(&self) -> CoreBaseResult<f64> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "SystemMonitor not initialized".to_string()
            ));
        }
        
        unsafe {
            Ok(crate::cba_monitor_get_network_usage())
        }
    }
    
    /// Get GPU usage percentage
    pub fn get_gpu_usage(&self) -> CoreBaseResult<f64> {
        if !self.initialized {
            return Err(CoreBaseError::OperationFailed(
                "SystemMonitor not initialized".to_string()
            ));
        }
        
        unsafe {
            Ok(crate::cba_monitor_get_gpu_usage())
        }
    }
    
    /// Get monitoring configuration
    pub fn get_config(&self) -> &MonitoringConfig {
        &self.config
    }
    
    /// Update monitoring configuration
    pub fn set_config(&mut self, config: MonitoringConfig) {
        self.config = config;
        
        // Resize history if needed
        while self.history.len() > self.config.history_size {
            self.history.pop_front();
        }
    }
    
    /// Get historical monitoring data
    pub fn get_history(&self) -> &VecDeque<MonitoringDataPoint> {
        &self.history
    }
    
    /// Get historical data as vector
    pub fn get_history_vec(&self) -> Vec<MonitoringDataPoint> {
        self.history.iter().cloned().collect()
    }
    
    /// Clear monitoring history
    pub fn clear_history(&mut self) {
        self.history.clear();
    }
    
    /// Check if any resource usage exceeds thresholds
    pub fn check_thresholds(&self, resources: &SystemResources) -> Vec<String> {
        let mut alerts = Vec::new();
        
        if self.config.enable_cpu_monitoring && resources.cpu_usage_percent > self.config.cpu_threshold {
            alerts.push(format!(
                "CPU usage ({:.1}%) exceeds threshold ({:.1}%)",
                resources.cpu_usage_percent, self.config.cpu_threshold
            ));
        }
        
        if self.config.enable_memory_monitoring && resources.memory_usage_percent() > self.config.memory_threshold {
            alerts.push(format!(
                "Memory usage ({:.1}%) exceeds threshold ({:.1}%)",
                resources.memory_usage_percent(), self.config.memory_threshold
            ));
        }
        
        if self.config.enable_disk_monitoring && resources.disk_usage_percent() > self.config.disk_threshold {
            alerts.push(format!(
                "Disk usage ({:.1}%) exceeds threshold ({:.1}%)",
                resources.disk_usage_percent(), self.config.disk_threshold
            ));
        }
        
        if self.config.enable_network_monitoring && resources.network_usage_percent > self.config.network_threshold {
            alerts.push(format!(
                "Network usage ({:.1}%) exceeds threshold ({:.1}%)",
                resources.network_usage_percent, self.config.network_threshold
            ));
        }
        
        if self.config.enable_gpu_monitoring && resources.gpu_usage_percent > self.config.gpu_threshold {
            alerts.push(format!(
                "GPU usage ({:.1}%) exceeds threshold ({:.1}%)",
                resources.gpu_usage_percent, self.config.gpu_threshold
            ));
        }
        
        alerts
    }
    
    /// Get average usage over the history
    pub fn get_average_usage(&self) -> Option<MonitoringDataPoint> {
        if self.history.is_empty() {
            return None;
        }
        
        let count = self.history.len() as f64;
        let mut avg = MonitoringDataPoint {
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs(),
            cpu_usage: 0.0,
            memory_usage: 0.0,
            disk_usage: 0.0,
            network_usage: 0.0,
            gpu_usage: 0.0,
        };
        
        for point in &self.history {
            avg.cpu_usage += point.cpu_usage;
            avg.memory_usage += point.memory_usage;
            avg.disk_usage += point.disk_usage;
            avg.network_usage += point.network_usage;
            avg.gpu_usage += point.gpu_usage;
        }
        
        avg.cpu_usage /= count;
        avg.memory_usage /= count;
        avg.disk_usage /= count;
        avg.network_usage /= count;
        avg.gpu_usage /= count;
        
        Some(avg)
    }
    
    /// Get peak usage over the history
    pub fn get_peak_usage(&self) -> Option<MonitoringDataPoint> {
        if self.history.is_empty() {
            return None;
        }
        
        let mut peak = MonitoringDataPoint {
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs(),
            cpu_usage: 0.0,
            memory_usage: 0.0,
            disk_usage: 0.0,
            network_usage: 0.0,
            gpu_usage: 0.0,
        };
        
        for point in &self.history {
            peak.cpu_usage = peak.cpu_usage.max(point.cpu_usage);
            peak.memory_usage = peak.memory_usage.max(point.memory_usage);
            peak.disk_usage = peak.disk_usage.max(point.disk_usage);
            peak.network_usage = peak.network_usage.max(point.network_usage);
            peak.gpu_usage = peak.gpu_usage.max(point.gpu_usage);
        }
        
        Some(peak)
    }
    
    /// Check if it's time to update based on the configured interval
    pub fn should_update(&self) -> bool {
        match self.last_update {
            Some(last) => last.elapsed() >= self.config.update_interval,
            None => true,
        }
    }
    
    /// Add a data point to history
    fn add_to_history(&mut self, resources: &SystemResources) {
        let data_point = MonitoringDataPoint::from(resources);
        
        self.history.push_back(data_point);
        
        // Maintain history size limit
        while self.history.len() > self.config.history_size {
            self.history.pop_front();
        }
    }
}

impl Default for SystemMonitor {
    fn default() -> Self {
        Self::new().unwrap_or(SystemMonitor {
            initialized: false,
            config: MonitoringConfig::default(),
            history: VecDeque::new(),
            last_update: None,
        })
    }
}

/// Async monitoring operations (requires "async" feature)
#[cfg(feature = "async")]
pub mod async_ops {
    use super::*;
    use tokio::time::{interval, Duration};
    use tokio::sync::mpsc;
    
    /// Async system monitor that continuously monitors system resources
    pub struct AsyncSystemMonitor {
        monitor: SystemMonitor,
        sender: Option<mpsc::UnboundedSender<SystemResources>>,
    }
    
    impl AsyncSystemMonitor {
        /// Create a new async system monitor
        pub fn new(config: MonitoringConfig) -> CoreBaseResult<Self> {
            Ok(AsyncSystemMonitor {
                monitor: SystemMonitor::with_config(config)?,
                sender: None,
            })
        }
        
        /// Start continuous monitoring
        pub async fn start_monitoring(&mut self) -> CoreBaseResult<mpsc::UnboundedReceiver<SystemResources>> {
            let (sender, receiver) = mpsc::unbounded_channel();
            self.sender = Some(sender.clone());
            
            let update_interval = self.monitor.config.update_interval;
            let mut interval_timer = interval(update_interval);
            
            tokio::spawn(async move {
                loop {
                    interval_timer.tick().await;
                    
                    // In a real implementation, we would need to safely access the monitor
                    // For now, this is a placeholder for the async monitoring loop
                    if sender.is_closed() {
                        break;
                    }
                }
            });
            
            Ok(receiver)
        }
        
        /// Stop monitoring
        pub fn stop_monitoring(&mut self) {
            self.sender = None;
        }
        
        /// Get the underlying monitor
        pub fn monitor(&mut self) -> &mut SystemMonitor {
            &mut self.monitor
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_system_monitor_creation() {
        let monitor = SystemMonitor::new();
        assert!(monitor.is_ok());
        assert!(monitor.unwrap().initialized);
    }
    
    #[test]
    fn test_system_resources_calculations() {
        let resources = SystemResources {
            cpu_usage_percent: 50.0,
            available_memory_bytes: 2_000_000_000.0, // 2GB
            total_memory_bytes: 8_000_000_000.0,     // 8GB
            available_disk_bytes: 100_000_000_000.0, // 100GB
            total_disk_bytes: 500_000_000_000.0,     // 500GB
            network_usage_percent: 25.0,
            gpu_usage_percent: 75.0,
            timestamp: 1234567890,
        };
        
        assert_eq!(resources.memory_usage_percent(), 75.0); // (8-2)/8 * 100
        assert_eq!(resources.disk_usage_percent(), 80.0);   // (500-100)/500 * 100
        assert_eq!(resources.used_memory_bytes(), 6_000_000_000.0);
        assert_eq!(resources.used_disk_bytes(), 400_000_000_000.0);
    }
    
    #[test]
    fn test_format_bytes() {
        assert_eq!(SystemResources::format_bytes(1024.0), "1.00 KB");
        assert_eq!(SystemResources::format_bytes(1_048_576.0), "1.00 MB");
        assert_eq!(SystemResources::format_bytes(1_073_741_824.0), "1.00 GB");
        assert_eq!(SystemResources::format_bytes(500.0), "500.00 B");
    }
    
    #[test]
    fn test_monitoring_config() {
        let config = MonitoringConfig {
            update_interval: Duration::from_millis(500),
            history_size: 50,
            cpu_threshold: 90.0,
            ..Default::default()
        };
        
        let mut monitor = SystemMonitor::with_config(config.clone()).unwrap();
        assert_eq!(monitor.get_config().update_interval, Duration::from_millis(500));
        assert_eq!(monitor.get_config().history_size, 50);
        assert_eq!(monitor.get_config().cpu_threshold, 90.0);
    }
    
    #[test]
    fn test_threshold_checking() {
        let monitor = SystemMonitor::new().unwrap();
        let resources = SystemResources {
            cpu_usage_percent: 85.0, // Above default threshold of 80%
            available_memory_bytes: 1_000_000_000.0,
            total_memory_bytes: 8_000_000_000.0, // 87.5% usage, above 85% threshold
            ..Default::default()
        };
        
        let alerts = monitor.check_thresholds(&resources);
        assert_eq!(alerts.len(), 2); // CPU and memory alerts
        assert!(alerts[0].contains("CPU usage"));
        assert!(alerts[1].contains("Memory usage"));
    }
    
    #[test]
    fn test_default_system_monitor() {
        let monitor = SystemMonitor::default();
        assert_eq!(monitor.history.len(), 0);
        assert!(monitor.last_update.is_none());
    }
}