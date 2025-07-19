package com.corebase;

/**
 * Java wrapper for the C++ ErrorHandler class.
 * Provides logging and error handling functionality.
 */
public class ErrorHandler {
    
    // Log levels matching the C++ LogLevel enum
    public static final int LOG_LEVEL_DEBUG = 0;
    public static final int LOG_LEVEL_INFO = 1;
    public static final int LOG_LEVEL_WARNING = 2;
    public static final int LOG_LEVEL_ERROR = 3;
    public static final int LOG_LEVEL_CRITICAL = 4;
    
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
     * Initialize the error handler.
     */
    public native void initialize();
    
    /**
     * Shutdown the error handler.
     */
    public native void shutdown();
    
    /**
     * Handle an error with detailed information.
     * 
     * @param message The error message
     * @param file The source file where the error occurred
     * @param line The line number where the error occurred
     * @param function The function name where the error occurred
     */
    public native void handleError(String message, String file, int line, String function);
    
    /**
     * Set the current log level.
     * 
     * @param level The log level (use LOG_LEVEL_* constants)
     */
    public native void setLogLevel(int level);
    
    /**
     * Get the current log level.
     * 
     * @return The current log level
     */
    public native int getLogLevel();
    
    /**
     * Log a message with the specified level.
     * 
     * @param level The log level
     * @param message The message to log
     */
    public native void log(int level, String message);
    
    /**
     * Log a debug message.
     * 
     * @param message The message to log
     */
    public native void debug(String message);
    
    /**
     * Log an info message.
     * 
     * @param message The message to log
     */
    public native void info(String message);
    
    /**
     * Log a warning message.
     * 
     * @param message The message to log
     */
    public native void warning(String message);
    
    /**
     * Log an error message.
     * 
     * @param message The message to log
     */
    public native void error(String message);
    
    /**
     * Log a critical message.
     * 
     * @param message The message to log
     */
    public native void critical(String message);
    
    /**
     * Convenience method to handle an error with current stack trace.
     * 
     * @param message The error message
     */
    public void handleError(String message) {
        StackTraceElement[] stack = Thread.currentThread().getStackTrace();
        if (stack.length > 2) {
            StackTraceElement caller = stack[2]; // Skip getStackTrace() and this method
            handleError(message, caller.getFileName(), caller.getLineNumber(), caller.getMethodName());
        } else {
            handleError(message, "unknown", 0, "unknown");
        }
    }
    
    /**
     * Convenience method to handle an exception.
     * 
     * @param exception The exception to handle
     */
    public void handleException(Exception exception) {
        StackTraceElement[] stack = exception.getStackTrace();
        if (stack.length > 0) {
            StackTraceElement source = stack[0];
            handleError(exception.getMessage(), source.getFileName(), 
                       source.getLineNumber(), source.getMethodName());
        } else {
            handleError(exception.getMessage(), "unknown", 0, "unknown");
        }
    }
    
    /**
     * Check if a log level is enabled.
     * 
     * @param level The log level to check
     * @return true if the level is enabled, false otherwise
     */
    public boolean isLogLevelEnabled(int level) {
        return level >= getLogLevel();
    }
    
    /**
     * Get a string representation of a log level.
     * 
     * @param level The log level
     * @return String representation of the level
     */
    public static String logLevelToString(int level) {
        switch (level) {
            case LOG_LEVEL_DEBUG: return "DEBUG";
            case LOG_LEVEL_INFO: return "INFO";
            case LOG_LEVEL_WARNING: return "WARNING";
            case LOG_LEVEL_ERROR: return "ERROR";
            case LOG_LEVEL_CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * Parse a log level from string.
     * 
     * @param levelStr The string representation of the log level
     * @return The log level constant, or LOG_LEVEL_INFO if not recognized
     */
    public static int parseLogLevel(String levelStr) {
        if (levelStr == null) return LOG_LEVEL_INFO;
        
        switch (levelStr.toUpperCase()) {
            case "DEBUG": return LOG_LEVEL_DEBUG;
            case "INFO": return LOG_LEVEL_INFO;
            case "WARNING": case "WARN": return LOG_LEVEL_WARNING;
            case "ERROR": return LOG_LEVEL_ERROR;
            case "CRITICAL": case "CRIT": return LOG_LEVEL_CRITICAL;
            default: return LOG_LEVEL_INFO;
        }
    }
}