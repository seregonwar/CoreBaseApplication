#pragma once

#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <functional>

namespace CoreNS {

// Forward declaration
class ErrorHandler;

enum class IPCType {
    SHARED_MEMORY,
    NAMED_PIPE,
    SOCKET,
    MESSAGE_QUEUE
};

enum class IPCRole {
    SERVER,
    CLIENT
};

struct ChannelInfo {
    std::string name;
    IPCType type;
    bool isServer{false};
    std::string address;
    int port{0};
    bool isActive{false};
    size_t size{0};
};

// Struttura interna per tracciare i canali IPC
struct IPCChannelInfo {
    std::string name;
    IPCType type;
    bool isActive{false};
    size_t size{0};
};

// Callback per i messaggi
using MessageCallback = std::function<void(const std::string&, const void*, size_t)>;

class IPCManagerImpl;

class IPCManager {
public:
    IPCManager();
    explicit IPCManager(ErrorHandler& errorHandler);
    ~IPCManager();

    bool initialize(const ChannelInfo& info);
    bool initialize(IPCRole role, const std::string& channelName);
    bool sendData(const std::string& data);
    bool sendData(const std::string& channelName, const void* data, size_t size);
    bool receiveData(std::string& data);
    std::string receiveData();
    bool receiveData(const std::string& channelName, void* buffer, size_t bufferSize, size_t& bytesRead);
    void close();

    // Gestione dei canali
    bool isChannelOpen(const std::string& channelName) const;
    bool closeChannel(const std::string& channelName);

    // Metodi per memoria condivisa
    bool createSharedMemory(const std::string& name, size_t size);
    bool releaseSharedMemory(const std::string& name);

    // Metodi per named pipe
    bool createNamedPipe(const std::string& name);
    bool writeToNamedPipe(const std::string& name, const void* data, size_t size);
    bool readFromNamedPipe(const std::string& name, void* buffer, size_t size);
    bool closeNamedPipe(const std::string& name);

    // Metodi per socket
    bool createSocket(const std::string& address, int port);
    bool writeToSocket(const std::string& address, const void* data, size_t size);
    bool readFromSocket(const std::string& address, void* buffer, size_t size);
    bool closeSocket(const std::string& address);

    // Metodi per code di messaggi
    bool createMessageQueue(const std::string& name);
    bool sendMessage(const std::string& name, const void* data, size_t size);
    bool receiveMessage(const std::string& name, void* buffer, size_t size);
    bool closeMessageQueue(const std::string& name);

    // Gestione callback per messaggi
    int registerMessageCallback(const std::string& messageName, MessageCallback callback);
    bool unregisterMessageCallback(const std::string& messageName, int callbackId);
    
    // Alias per compatibilità con CoreAPI
    int registerCallback(const std::string& channelName, MessageCallback callback) {
        return registerMessageCallback(channelName, callback);
    }
    
    bool unregisterCallback(const std::string& channelName, int callbackId) {
        return unregisterMessageCallback(channelName, callbackId);
    }

    // Metodo utilità
    bool channelExists(const std::string& name, IPCType type) const;

private:
    std::unique_ptr<IPCManagerImpl> m_impl;
    std::mutex m_mutex;
    std::map<std::string, IPCChannelInfo> m_channels;
    std::map<int, MessageCallback> m_messageCallbacks;
    int m_lastCallbackId{0};
};

} // namespace CoreNS
