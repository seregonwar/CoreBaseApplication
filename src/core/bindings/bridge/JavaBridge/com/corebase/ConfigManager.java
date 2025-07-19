package com.corebase;

import java.util.Properties;
import java.io.*;
import java.util.Map;
import java.util.HashMap;

/**
 * Java wrapper for the C++ ConfigManager class.
 * Provides configuration management functionality.
 */
public class ConfigManager {
    
    // Load the native library
    static {
        try {
            System.loadLibrary("CoreBaseApplication");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Failed to load CoreBaseApplication native library: " + e.getMessage());
            throw e;
        }
    }
    
    /**
     * Load configuration from a file.
     * 
     * @param filename The path to the configuration file
     * @return true if the configuration was loaded successfully, false otherwise
     */
    public native boolean loadConfig(String filename);
    
    /**
     * Get a configuration value by key.
     * 
     * @param key The configuration key
     * @return The configuration value, or empty string if not found
     */
    public native String getValue(String key);
    
    /**
     * Set a configuration value.
     * 
     * @param key The configuration key
     * @param value The configuration value
     */
    public native void setValue(String key, String value);
    
    /**
     * Save configuration to a file.
     * 
     * @param filename The path to save the configuration file
     * @return true if the configuration was saved successfully, false otherwise
     */
    public native boolean saveConfig(String filename);
    
    /**
     * Get a configuration value with a default fallback.
     * 
     * @param key The configuration key
     * @param defaultValue The default value to return if key is not found
     * @return The configuration value or the default value
     */
    public String getValue(String key, String defaultValue) {
        String value = getValue(key);
        return (value != null && !value.isEmpty()) ? value : defaultValue;
    }
    
    /**
     * Get a configuration value as an integer.
     * 
     * @param key The configuration key
     * @param defaultValue The default value to return if key is not found or invalid
     * @return The configuration value as integer or the default value
     */
    public int getIntValue(String key, int defaultValue) {
        try {
            String value = getValue(key);
            return (value != null && !value.isEmpty()) ? Integer.parseInt(value) : defaultValue;
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }
    
    /**
     * Get a configuration value as a boolean.
     * 
     * @param key The configuration key
     * @param defaultValue The default value to return if key is not found
     * @return The configuration value as boolean or the default value
     */
    public boolean getBooleanValue(String key, boolean defaultValue) {
        String value = getValue(key);
        if (value == null || value.isEmpty()) {
            return defaultValue;
        }
        
        value = value.toLowerCase().trim();
        return "true".equals(value) || "1".equals(value) || "yes".equals(value) || "on".equals(value);
    }
    
    /**
     * Get a configuration value as a double.
     * 
     * @param key The configuration key
     * @param defaultValue The default value to return if key is not found or invalid
     * @return The configuration value as double or the default value
     */
    public double getDoubleValue(String key, double defaultValue) {
        try {
            String value = getValue(key);
            return (value != null && !value.isEmpty()) ? Double.parseDouble(value) : defaultValue;
        } catch (NumberFormatException e) {
            return defaultValue;
        }
    }
    
    /**
     * Set an integer configuration value.
     * 
     * @param key The configuration key
     * @param value The integer value
     */
    public void setIntValue(String key, int value) {
        setValue(key, String.valueOf(value));
    }
    
    /**
     * Set a boolean configuration value.
     * 
     * @param key The configuration key
     * @param value The boolean value
     */
    public void setBooleanValue(String key, boolean value) {
        setValue(key, String.valueOf(value));
    }
    
    /**
     * Set a double configuration value.
     * 
     * @param key The configuration key
     * @param value The double value
     */
    public void setDoubleValue(String key, double value) {
        setValue(key, String.valueOf(value));
    }
    
    /**
     * Check if a configuration key exists.
     * 
     * @param key The configuration key
     * @return true if the key exists, false otherwise
     */
    public boolean hasKey(String key) {
        String value = getValue(key);
        return value != null && !value.isEmpty();
    }
    
    /**
     * Load configuration from Java Properties object.
     * This is a convenience method that doesn't use the native implementation.
     * 
     * @param properties The Properties object to load from
     */
    public void loadFromProperties(Properties properties) {
        for (Map.Entry<Object, Object> entry : properties.entrySet()) {
            setValue(entry.getKey().toString(), entry.getValue().toString());
        }
    }
    
    /**
     * Export configuration to Java Properties object.
     * Note: This method requires iterating through all keys, which is not
     * directly supported by the current native interface.
     * 
     * @param keys The keys to export
     * @return Properties object with the exported configuration
     */
    public Properties exportToProperties(String[] keys) {
        Properties properties = new Properties();
        for (String key : keys) {
            String value = getValue(key);
            if (value != null && !value.isEmpty()) {
                properties.setProperty(key, value);
            }
        }
        return properties;
    }
    
    /**
     * Load configuration from a Properties file using Java I/O.
     * This is a convenience method that combines Java Properties loading
     * with native configuration storage.
     * 
     * @param filename The path to the properties file
     * @return true if loaded successfully, false otherwise
     */
    public boolean loadFromPropertiesFile(String filename) {
        try (FileInputStream fis = new FileInputStream(filename)) {
            Properties properties = new Properties();
            properties.load(fis);
            loadFromProperties(properties);
            return true;
        } catch (IOException e) {
            System.err.println("Failed to load properties file: " + e.getMessage());
            return false;
        }
    }
    
    /**
     * Save configuration to a Properties file using Java I/O.
     * Note: This requires knowing which keys to export.
     * 
     * @param filename The path to save the properties file
     * @param keys The keys to export
     * @param comments Comments to add to the properties file
     * @return true if saved successfully, false otherwise
     */
    public boolean saveToPropertiesFile(String filename, String[] keys, String comments) {
        try (FileOutputStream fos = new FileOutputStream(filename)) {
            Properties properties = exportToProperties(keys);
            properties.store(fos, comments);
            return true;
        } catch (IOException e) {
            System.err.println("Failed to save properties file: " + e.getMessage());
            return false;
        }
    }
    
    /**
     * Create a configuration section namespace.
     * This is a convenience method for organizing configuration keys.
     * 
     * @param section The section name
     * @return A ConfigSection object for the specified section
     */
    public ConfigSection getSection(String section) {
        return new ConfigSection(this, section);
    }
    
    /**
     * Inner class for handling configuration sections.
     */
    public static class ConfigSection {
        private final ConfigManager configManager;
        private final String sectionPrefix;
        
        public ConfigSection(ConfigManager configManager, String section) {
            this.configManager = configManager;
            this.sectionPrefix = section + ".";
        }
        
        public String getValue(String key) {
            return configManager.getValue(sectionPrefix + key);
        }
        
        public String getValue(String key, String defaultValue) {
            return configManager.getValue(sectionPrefix + key, defaultValue);
        }
        
        public void setValue(String key, String value) {
            configManager.setValue(sectionPrefix + key, value);
        }
        
        public int getIntValue(String key, int defaultValue) {
            return configManager.getIntValue(sectionPrefix + key, defaultValue);
        }
        
        public boolean getBooleanValue(String key, boolean defaultValue) {
            return configManager.getBooleanValue(sectionPrefix + key, defaultValue);
        }
        
        public double getDoubleValue(String key, double defaultValue) {
            return configManager.getDoubleValue(sectionPrefix + key, defaultValue);
        }
        
        public void setIntValue(String key, int value) {
            configManager.setIntValue(sectionPrefix + key, value);
        }
        
        public void setBooleanValue(String key, boolean value) {
            configManager.setBooleanValue(sectionPrefix + key, value);
        }
        
        public void setDoubleValue(String key, double value) {
            configManager.setDoubleValue(sectionPrefix + key, value);
        }
        
        public boolean hasKey(String key) {
            return configManager.hasKey(sectionPrefix + key);
        }
    }
}