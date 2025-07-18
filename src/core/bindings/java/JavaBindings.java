package core.bindings.java;

public class JavaBindings {
    static {
        System.loadLibrary("JavaBindings"); // Carica la DLL/so/dylib generata
    }

    // Funzioni native che corrispondono a quelle di ErrorHandler
    public native void initialize();
    public native void shutdown();
    public native void handleError(String message, String file, int line, String function);
    public native void setLogLevel(int level);
    public native int getLogLevel();
    public native void log(int level, String message);
    public native void debug(String message);
    public native void info(String message);
    public native void warning(String message);
    public native void error(String message);
    public native void critical(String message);

    // Eventuali metodi di utilità Java...
} 