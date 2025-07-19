#pragma once
#include <napi.h>
#include "../../../../NetworkManager.h"
#include <memory>

/**
 * @file network_manager_wrapper.h
 * @brief Node.js wrapper for CoreBase NetworkManager
 */

class NetworkManagerWrapper : public Napi::ObjectWrap<NetworkManagerWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static Napi::Value CreateNew(const Napi::CallbackInfo& info);
    
    NetworkManagerWrapper(const Napi::CallbackInfo& info);
    virtual ~NetworkManagerWrapper();

private:
    // Core methods
    Napi::Value Initialize(const Napi::CallbackInfo& info);
    Napi::Value Shutdown(const Napi::CallbackInfo& info);
    Napi::Value IsInitialized(const Napi::CallbackInfo& info);
    
    // HTTP methods
    Napi::Value HttpGet(const Napi::CallbackInfo& info);
    Napi::Value HttpPost(const Napi::CallbackInfo& info);
    Napi::Value HttpPut(const Napi::CallbackInfo& info);
    Napi::Value HttpDelete(const Napi::CallbackInfo& info);
    Napi::Value HttpRequest(const Napi::CallbackInfo& info);
    
    // WebSocket methods
    Napi::Value CreateWebSocket(const Napi::CallbackInfo& info);
    Napi::Value ConnectWebSocket(const Napi::CallbackInfo& info);
    Napi::Value SendWebSocketMessage(const Napi::CallbackInfo& info);
    Napi::Value CloseWebSocket(const Napi::CallbackInfo& info);
    Napi::Value IsWebSocketConnected(const Napi::CallbackInfo& info);
    
    // TCP/UDP methods
    Napi::Value CreateTcpServer(const Napi::CallbackInfo& info);
    Napi::Value CreateTcpClient(const Napi::CallbackInfo& info);
    Napi::Value CreateUdpSocket(const Napi::CallbackInfo& info);
    Napi::Value SendTcpData(const Napi::CallbackInfo& info);
    Napi::Value SendUdpData(const Napi::CallbackInfo& info);
    Napi::Value CloseTcpConnection(const Napi::CallbackInfo& info);
    Napi::Value CloseUdpSocket(const Napi::CallbackInfo& info);
    
    // Configuration methods
    Napi::Value SetTimeout(const Napi::CallbackInfo& info);
    Napi::Value GetTimeout(const Napi::CallbackInfo& info);
    Napi::Value SetRetryCount(const Napi::CallbackInfo& info);
    Napi::Value GetRetryCount(const Napi::CallbackInfo& info);
    Napi::Value SetUserAgent(const Napi::CallbackInfo& info);
    Napi::Value GetUserAgent(const Napi::CallbackInfo& info);
    Napi::Value SetHeaders(const Napi::CallbackInfo& info);
    Napi::Value GetHeaders(const Napi::CallbackInfo& info);
    
    // SSL/TLS methods
    Napi::Value EnableSSL(const Napi::CallbackInfo& info);
    Napi::Value DisableSSL(const Napi::CallbackInfo& info);
    Napi::Value SetCertificate(const Napi::CallbackInfo& info);
    Napi::Value SetPrivateKey(const Napi::CallbackInfo& info);
    Napi::Value SetCACertificate(const Napi::CallbackInfo& info);
    Napi::Value VerifySSL(const Napi::CallbackInfo& info);
    
    // Monitoring methods
    Napi::Value GetConnectionStats(const Napi::CallbackInfo& info);
    Napi::Value GetBandwidthUsage(const Napi::CallbackInfo& info);
    Napi::Value GetActiveConnections(const Napi::CallbackInfo& info);
    Napi::Value ClearStats(const Napi::CallbackInfo& info);
    
    // Utility methods
    Napi::Value IsConnected(const Napi::CallbackInfo& info);
    Napi::Value GetLocalIP(const Napi::CallbackInfo& info);
    Napi::Value GetPublicIP(const Napi::CallbackInfo& info);
    Napi::Value ResolveHostname(const Napi::CallbackInfo& info);
    Napi::Value PingHost(const Napi::CallbackInfo& info);
    Napi::Value TraceRoute(const Napi::CallbackInfo& info);
    
    // Helper methods
    static Napi::Value HttpResponseToJS(Napi::Env env, const HttpResponse& response);
    static Napi::Value ConnectionStatsToJS(Napi::Env env, const ConnectionStats& stats);
    static Napi::Value BandwidthUsageToJS(Napi::Env env, const BandwidthUsage& usage);
    static HttpRequest JSToHttpRequest(const Napi::Value& value);
    static std::map<std::string, std::string> JSToHeaders(const Napi::Value& value);
    static Napi::Value HeadersToJS(Napi::Env env, const std::map<std::string, std::string>& headers);
    
    std::unique_ptr<NetworkManager> networkManager_;
};