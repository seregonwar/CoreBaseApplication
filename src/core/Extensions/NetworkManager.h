#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <atomic>
#include <unordered_map>
#include <chrono>
#include <variant>
#include <optional>

namespace Core {
namespace Extensions {

/**
 * @brief Enum che definisce i protocolli di rete supportati.
 */
enum class NetworkProtocol {
    TCP,        ///< Transmission Control Protocol
    UDP,        ///< User Datagram Protocol
    HTTP,       ///< Hypertext Transfer Protocol
    HTTPS,      ///< HTTP Secure
    WEBSOCKET,  ///< WebSocket
    MQTT,       ///< Message Queuing Telemetry Transport
    AMQP,       ///< Advanced Message Queuing Protocol
    GRPC,       ///< gRPC
    CUSTOM      ///< Protocollo personalizzato
};

/**
 * @brief Enum che definisce gli stati di una connessione.
 */
enum class ConnectionState {
    DISCONNECTED,    ///< Non connesso
    CONNECTING,      ///< In fase di connessione
    CONNECTED,       ///< Connesso
    DISCONNECTING,   ///< In fase di disconnessione
    ERROR            ///< Errore
};

/**
 * @brief Struct che rappresenta una configurazione di rete.
 */
struct NetworkConfig {
    std::string host;                   ///< Host di destinazione
    uint16_t port;                      ///< Porta di destinazione
    NetworkProtocol protocol;           ///< Protocollo da utilizzare
    std::chrono::seconds timeout;       ///< Timeout per le operazioni
    bool reconnectAutomatically;        ///< Se true, tenta di riconnettersi automaticamente
    int maxReconnectAttempts;           ///< Numero massimo di tentativi di riconnessione
    std::chrono::seconds reconnectDelay; ///< Ritardo tra i tentativi di riconnessione
    
    // Certificati per connessioni sicure (per HTTPS, WSS, ecc.)
    std::optional<std::string> certificatePath;    ///< Percorso del certificato client
    std::optional<std::string> keyPath;           ///< Percorso della chiave privata
    std::optional<std::string> caCertPath;        ///< Percorso del certificato CA
    bool verifyPeer;                            ///< Se true, verifica il certificato del peer
    
    // Parametri di autenticazione
    std::optional<std::string> username;           ///< Nome utente
    std::optional<std::string> password;           ///< Password
    std::optional<std::string> authToken;          ///< Token di autenticazione
    
    // Parametri specifici per protocollo
    std::unordered_map<std::string, std::string> protocolSpecificParams;
    
    /**
     * @brief Costruttore con parametri minimi.
     */
    NetworkConfig(const std::string& _host = "", uint16_t _port = 0, 
                 NetworkProtocol _protocol = NetworkProtocol::TCP)
        : host(_host), port(_port), protocol(_protocol),
          timeout(30), reconnectAutomatically(true),
          maxReconnectAttempts(5), reconnectDelay(5),
          verifyPeer(true) {}
};

/**
 * @brief Struct che rappresenta un messaggio di rete.
 */
struct NetworkMessage {
    std::vector<uint8_t> data;          ///< Dati del messaggio
    std::string topic;                  ///< Topic del messaggio (usato in MQTT, ecc.)
    std::chrono::system_clock::time_point timestamp; ///< Timestamp del messaggio
    std::string sender;                 ///< Identificatore del mittente
    std::unordered_map<std::string, std::string> headers; ///< Header del messaggio
    
    /**
     * @brief Costruttore per dati binari.
     */
    NetworkMessage(const std::vector<uint8_t>& _data, const std::string& _topic = "")
        : data(_data), topic(_topic), timestamp(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Costruttore per stringhe.
     */
    NetworkMessage(const std::string& _data, const std::string& _topic = "")
        : topic(_topic), timestamp(std::chrono::system_clock::now()) {
        data.assign(_data.begin(), _data.end());
    }
    
    /**
     * @brief Ottiene i dati come stringa.
     */
    std::string getDataAsString() const {
        return std::string(data.begin(), data.end());
    }
    
    /**
     * @brief Imposta un header.
     */
    void setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }
    
    /**
     * @brief Ottiene un header.
     */
    std::optional<std::string> getHeader(const std::string& key) const {
        auto it = headers.find(key);
        if (it != headers.end()) {
            return it->second;
        }
        return std::nullopt;
    }
};

/**
 * @brief Struct che rappresenta un errore di rete.
 */
struct NetworkError {
    int errorCode;                   ///< Codice di errore
    std::string errorMessage;        ///< Messaggio di errore
    std::string source;              ///< Origine dell'errore
    std::chrono::system_clock::time_point timestamp; ///< Timestamp dell'errore
    
    /**
     * @brief Costruttore.
     */
    NetworkError(int code, const std::string& message, const std::string& src = "")
        : errorCode(code), errorMessage(message), source(src),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Interfaccia per i gestori di protocollo.
 */
class IProtocolHandler {
public:
    virtual ~IProtocolHandler() = default;
    
    /**
     * @brief Inizializza il gestore.
     * @param config Configurazione di rete
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    virtual bool initialize(const NetworkConfig& config) = 0;
    
    /**
     * @brief Connette al server.
     * @return true se la connessione è avvenuta con successo, false altrimenti
     */
    virtual bool connect() = 0;
    
    /**
     * @brief Disconnette dal server.
     * @return true se la disconnessione è avvenuta con successo, false altrimenti
     */
    virtual bool disconnect() = 0;
    
    /**
     * @brief Invia un messaggio.
     * @param message Messaggio da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    virtual bool send(const NetworkMessage& message) = 0;
    
    /**
     * @brief Riceve un messaggio (sincrono).
     * @param timeout Timeout per la ricezione
     * @return Messaggio ricevuto, o nullopt in caso di errore o timeout
     */
    virtual std::optional<NetworkMessage> receive(std::chrono::milliseconds timeout) = 0;
    
    /**
     * @brief Registra una callback da chiamare quando arriva un messaggio.
     * @param callback Funzione da chiamare
     * @return ID della callback, necessario per rimuoverla
     */
    virtual int registerMessageCallback(std::function<void(const NetworkMessage&)> callback) = 0;
    
    /**
     * @brief Rimuove una callback registrata.
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false se non esisteva
     */
    virtual bool unregisterMessageCallback(int callbackId) = 0;
    
    /**
     * @brief Ottiene lo stato della connessione.
     * @return Stato della connessione
     */
    virtual ConnectionState getState() const = 0;
    
    /**
     * @brief Ottiene l'ultimo errore.
     * @return Ultimo errore, o nullopt se non ci sono errori
     */
    virtual std::optional<NetworkError> getLastError() const = 0;
};

/**
 * @brief Implementazione di base per i gestori di protocollo.
 */
class BaseProtocolHandler : public IProtocolHandler {
public:
    /**
     * @brief Costruttore.
     */
    BaseProtocolHandler() : m_state(ConnectionState::DISCONNECTED), m_nextCallbackId(0) {}
    
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~BaseProtocolHandler() {
        if (m_state == ConnectionState::CONNECTED) {
            disconnect();
        }
    }
    
    /**
     * @brief Inizializza il gestore.
     * @param config Configurazione di rete
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(const NetworkConfig& config) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_config = config;
        return doInitialize();
    }
    
    /**
     * @brief Registra una callback da chiamare quando arriva un messaggio.
     * @param callback Funzione da chiamare
     * @return ID della callback, necessario per rimuoverla
     */
    int registerMessageCallback(std::function<void(const NetworkMessage&)> callback) override {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        
        int id = m_nextCallbackId++;
        m_messageCallbacks[id] = callback;
        return id;
    }
    
    /**
     * @brief Rimuove una callback registrata.
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false se non esisteva
     */
    bool unregisterMessageCallback(int callbackId) override {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        
        auto it = m_messageCallbacks.find(callbackId);
        if (it != m_messageCallbacks.end()) {
            m_messageCallbacks.erase(it);
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief Ottiene lo stato della connessione.
     * @return Stato della connessione
     */
    ConnectionState getState() const override {
        return m_state;
    }
    
    /**
     * @brief Ottiene l'ultimo errore.
     * @return Ultimo errore, o nullopt se non ci sono errori
     */
    std::optional<NetworkError> getLastError() const override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_lastError;
    }

protected:
    /**
     * @brief Implementazione specifica dell'inizializzazione.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    virtual bool doInitialize() = 0;
    
    /**
     * @brief Chiama tutte le callback registrate con un messaggio.
     * @param message Messaggio da passare alle callback
     */
    void notifyMessageCallbacks(const NetworkMessage& message) {
        std::unordered_map<int, std::function<void(const NetworkMessage&)>> callbacksCopy;
        
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            callbacksCopy = m_messageCallbacks;
        }
        
        for (const auto& [id, callback] : callbacksCopy) {
            callback(message);
        }
    }
    
    /**
     * @brief Imposta lo stato della connessione.
     * @param state Nuovo stato
     */
    void setState(ConnectionState state) {
        m_state = state;
    }
    
    /**
     * @brief Imposta l'ultimo errore.
     * @param error Errore da impostare
     */
    void setLastError(const NetworkError& error) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lastError = error;
    }
    
    NetworkConfig m_config;
    std::atomic<ConnectionState> m_state;
    mutable std::mutex m_mutex;
    std::optional<NetworkError> m_lastError;
    
    std::mutex m_callbackMutex;
    int m_nextCallbackId;
    std::unordered_map<int, std::function<void(const NetworkMessage&)>> m_messageCallbacks;
};

/**
 * @brief Gestore di rete che fornisce funzionalità di rete avanzate.
 */
class NetworkManager {
public:
    /**
     * @brief Ottiene l'istanza singleton del gestore di rete.
     * @return Riferimento all'istanza singleton
     */
    static NetworkManager& getInstance() {
        static NetworkManager instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il gestore di rete.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        // Registra i gestori di protocollo predefiniti
        registerDefaultProtocolHandlers();
        
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Crea una connessione di rete.
     * @param config Configurazione di rete
     * @return ID della connessione, o empty in caso di errore
     */
    std::optional<std::string> createConnection(const NetworkConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) {
            if (!initialize()) return std::nullopt;
        }
        
        // Ottiene il gestore di protocollo appropriato
        auto protocolHandler = getProtocolHandler(config.protocol);
        if (!protocolHandler) {
            setLastError(NetworkError(1, "Protocol handler not available", "createConnection"));
            return std::nullopt;
        }
        
        // Inizializza il gestore
        if (!protocolHandler->initialize(config)) {
            setLastError(NetworkError(2, "Failed to initialize protocol handler", "createConnection"));
            return std::nullopt;
        }
        
        // Genera un ID univoco per la connessione
        std::string connectionId = generateConnectionId();
        
        // Memorizza il gestore
        m_connections[connectionId] = protocolHandler;
        
        return connectionId;
    }
    
    /**
     * @brief Connette una connessione esistente.
     * @param connectionId ID della connessione
     * @return true se la connessione è avvenuta con successo, false altrimenti
     */
    bool connect(const std::string& connectionId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            setLastError(NetworkError(3, "Connection not found", "connect"));
            return false;
        }
        
        return it->second->connect();
    }
    
    /**
     * @brief Disconnette una connessione esistente.
     * @param connectionId ID della connessione
     * @return true se la disconnessione è avvenuta con successo, false altrimenti
     */
    bool disconnect(const std::string& connectionId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            setLastError(NetworkError(3, "Connection not found", "disconnect"));
            return false;
        }
        
        return it->second->disconnect();
    }
    
    /**
     * @brief Chiude e rimuove una connessione.
     * @param connectionId ID della connessione
     * @return true se la connessione è stata rimossa, false altrimenti
     */
    bool closeConnection(const std::string& connectionId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            return false;
        }
        
        if (it->second->getState() == ConnectionState::CONNECTED) {
            it->second->disconnect();
        }
        
        m_connections.erase(it);
        return true;
    }
    
    /**
     * @brief Invia un messaggio tramite una connessione.
     * @param connectionId ID della connessione
     * @param message Messaggio da inviare
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    bool send(const std::string& connectionId, const NetworkMessage& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            setLastError(NetworkError(3, "Connection not found", "send"));
            return false;
        }
        
        return it->second->send(message);
    }
    
    /**
     * @brief Riceve un messaggio tramite una connessione (sincrono).
     * @param connectionId ID della connessione
     * @param timeout Timeout per la ricezione
     * @return Messaggio ricevuto, o nullopt in caso di errore o timeout
     */
    std::optional<NetworkMessage> receive(const std::string& connectionId, 
                                        std::chrono::milliseconds timeout = std::chrono::seconds(5)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            setLastError(NetworkError(3, "Connection not found", "receive"));
            return std::nullopt;
        }
        
        return it->second->receive(timeout);
    }
    
    /**
     * @brief Registra una callback da chiamare quando arriva un messaggio su una connessione.
     * @param connectionId ID della connessione
     * @param callback Funzione da chiamare
     * @return ID della callback, necessario per rimuoverla, o -1 in caso di errore
     */
    int registerMessageCallback(const std::string& connectionId, 
                               std::function<void(const NetworkMessage&)> callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            setLastError(NetworkError(3, "Connection not found", "registerMessageCallback"));
            return -1;
        }
        
        return it->second->registerMessageCallback(callback);
    }
    
    /**
     * @brief Rimuove una callback registrata.
     * @param connectionId ID della connessione
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false altrimenti
     */
    bool unregisterMessageCallback(const std::string& connectionId, int callbackId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            setLastError(NetworkError(3, "Connection not found", "unregisterMessageCallback"));
            return false;
        }
        
        return it->second->unregisterMessageCallback(callbackId);
    }
    
    /**
     * @brief Ottiene lo stato di una connessione.
     * @param connectionId ID della connessione
     * @return Stato della connessione, o ConnectionState::ERROR in caso di errore
     */
    ConnectionState getConnectionState(const std::string& connectionId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_connections.find(connectionId);
        if (it == m_connections.end()) {
            return ConnectionState::ERROR;
        }
        
        return it->second->getState();
    }
    
    /**
     * @brief Ottiene l'ultimo errore.
     * @return Ultimo errore, o nullopt se non ci sono errori
     */
    std::optional<NetworkError> getLastError() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_lastError;
    }
    
    /**
     * @brief Registra un gestore di protocollo personalizzato.
     * @param protocol Protocollo
     * @param handler Gestore di protocollo
     * @return true se la registrazione è avvenuta con successo, false altrimenti
     */
    bool registerProtocolHandler(NetworkProtocol protocol, std::shared_ptr<IProtocolHandler> handler) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!handler) return false;
        
        m_protocolHandlers[protocol] = handler;
        return true;
    }
    
    /**
     * @brief Ottiene tutte le connessioni attive.
     * @return Vettore di ID delle connessioni
     */
    std::vector<std::string> getActiveConnections() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::string> connections;
        for (const auto& [id, handler] : m_connections) {
            if (handler->getState() == ConnectionState::CONNECTED) {
                connections.push_back(id);
            }
        }
        
        return connections;
    }
    
    /**
     * @brief Chiude tutte le connessioni attive.
     * @return Numero di connessioni chiuse
     */
    int closeAllConnections() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        int count = 0;
        for (auto it = m_connections.begin(); it != m_connections.end();) {
            if (it->second->getState() == ConnectionState::CONNECTED) {
                it->second->disconnect();
                count++;
            }
            it = m_connections.erase(it);
        }
        
        return count;
    }
    
    /**
     * @brief Effettua una richiesta HTTP/HTTPS.
     * @param url URL della richiesta
     * @param method Metodo HTTP (GET, POST, ecc.)
     * @param headers Header della richiesta
     * @param body Corpo della richiesta
     * @param timeout Timeout per la richiesta
     * @return Risposta HTTP, o nullopt in caso di errore
     */
    std::optional<std::unordered_map<std::string, std::variant<int, std::string, std::vector<uint8_t>>>>
    httpRequest(const std::string& url, const std::string& method,
                const std::unordered_map<std::string, std::string>& headers = {},
                const std::variant<std::string, std::vector<uint8_t>>& body = std::string(),
                std::chrono::seconds timeout = std::chrono::seconds(30)) {
        // Implementazione semplificata della richiesta HTTP
        // In un'implementazione reale, userebbe una libreria come libcurl o Boost.Beast
        
        // Crea una configurazione HTTP
        NetworkConfig config;
        config.protocol = NetworkProtocol::HTTP;
        config.timeout = timeout;
        
        // Estrai host e porta dall'URL
        size_t protocolEnd = url.find("://");
        if (protocolEnd == std::string::npos) {
            setLastError(NetworkError(4, "Invalid URL", "httpRequest"));
            return std::nullopt;
        }
        
        std::string protocol = url.substr(0, protocolEnd);
        if (protocol == "https") {
            config.protocol = NetworkProtocol::HTTPS;
        }
        
        size_t hostStart = protocolEnd + 3;
        size_t pathStart = url.find("/", hostStart);
        size_t portStart = url.find(":", hostStart);
        
        if (portStart != std::string::npos && (pathStart == std::string::npos || portStart < pathStart)) {
            config.host = url.substr(hostStart, portStart - hostStart);
            size_t portEnd = (pathStart != std::string::npos) ? pathStart : url.size();
            config.port = std::stoi(url.substr(portStart + 1, portEnd - portStart - 1));
        } else {
            config.host = (pathStart != std::string::npos) ? url.substr(hostStart, pathStart - hostStart) : url.substr(hostStart);
            config.port = (config.protocol == NetworkProtocol::HTTPS) ? 443 : 80;
        }
        
        // Crea una connessione
        auto connectionIdOpt = createConnection(config);
        if (!connectionIdOpt) {
            return std::nullopt;
        }
        
        std::string connectionId = *connectionIdOpt;
        
        // Connetti
        if (!connect(connectionId)) {
            closeConnection(connectionId);
            return std::nullopt;
        }
        
        // Costruisci la richiesta HTTP
        std::string path = (pathStart != std::string::npos) ? url.substr(pathStart) : "/";
        std::string requestStr = method + " " + path + " HTTP/1.1\r\n";
        requestStr += "Host: " + config.host + "\r\n";
        
        // Aggiungi gli header
        for (const auto& [key, value] : headers) {
            requestStr += key + ": " + value + "\r\n";
        }
        
        // Aggiungi il corpo se necessario
        if (std::holds_alternative<std::string>(body)) {
            const auto& bodyStr = std::get<std::string>(body);
            if (!bodyStr.empty()) {
                requestStr += "Content-Length: " + std::to_string(bodyStr.size()) + "\r\n\r\n";
                requestStr += bodyStr;
            } else {
                requestStr += "\r\n";
            }
        } else {
            const auto& bodyData = std::get<std::vector<uint8_t>>(body);
            if (!bodyData.empty()) {
                requestStr += "Content-Length: " + std::to_string(bodyData.size()) + "\r\n\r\n";
                requestStr.append(reinterpret_cast<const char*>(bodyData.data()), bodyData.size());
            } else {
                requestStr += "\r\n";
            }
        }
        
        // Invia la richiesta
        NetworkMessage requestMessage(requestStr);
        if (!send(connectionId, requestMessage)) {
            disconnect(connectionId);
            closeConnection(connectionId);
            return std::nullopt;
        }
        
        // Ricevi la risposta
        auto responseOpt = receive(connectionId, timeout);
        
        // Disconnetti e chiudi la connessione
        disconnect(connectionId);
        closeConnection(connectionId);
        
        if (!responseOpt) {
            setLastError(NetworkError(5, "Failed to receive response", "httpRequest"));
            return std::nullopt;
        }
        
        // Analizza la risposta HTTP
        std::string responseStr = responseOpt->getDataAsString();
        
        // Trova la fine della riga di stato
        size_t statusLineEnd = responseStr.find("\r\n");
        if (statusLineEnd == std::string::npos) {
            setLastError(NetworkError(6, "Invalid HTTP response", "httpRequest"));
            return std::nullopt;
        }
        
        // Estrai la versione HTTP e il codice di stato
        std::string statusLine = responseStr.substr(0, statusLineEnd);
        size_t httpVersionEnd = statusLine.find(" ");
        size_t statusCodeEnd = statusLine.find(" ", httpVersionEnd + 1);
        
        if (httpVersionEnd == std::string::npos || statusCodeEnd == std::string::npos) {
            setLastError(NetworkError(6, "Invalid HTTP status line", "httpRequest"));
            return std::nullopt;
        }
        
        std::string httpVersion = statusLine.substr(0, httpVersionEnd);
        int statusCode = std::stoi(statusLine.substr(httpVersionEnd + 1, statusCodeEnd - httpVersionEnd - 1));
        std::string statusText = statusLine.substr(statusCodeEnd + 1);
        
        // Trova l'inizio del corpo
        size_t headersEnd = responseStr.find("\r\n\r\n");
        if (headersEnd == std::string::npos) {
            setLastError(NetworkError(6, "Invalid HTTP response", "httpRequest"));
            return std::nullopt;
        }
        
        // Estrai gli header
        std::string headersStr = responseStr.substr(statusLineEnd + 2, headersEnd - statusLineEnd - 2);
        std::unordered_map<std::string, std::string> responseHeaders;
        
        size_t pos = 0;
        while (pos < headersStr.size()) {
            size_t lineEnd = headersStr.find("\r\n", pos);
            if (lineEnd == std::string::npos) {
                break;
            }
            
            std::string line = headersStr.substr(pos, lineEnd - pos);
            pos = lineEnd + 2;
            
            size_t colonPos = line.find(":");
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                
                // Rimuovi spazi iniziali e finali
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                responseHeaders[key] = value;
            }
        }
        
        // Estrai il corpo
        std::vector<uint8_t> responseBody(responseStr.begin() + headersEnd + 4, responseStr.end());
        
        // Costruisci la risposta
        std::unordered_map<std::string, std::variant<int, std::string, std::vector<uint8_t>>> response;
        response["status_code"] = statusCode;
        response["status_text"] = statusText;
        
        // Converti gli headers in una singola stringa JSON per compatibilità con la variante
        std::string headersJson = "{";
        bool first = true;
        for (const auto& [key, value] : responseHeaders) {
            if (!first) headersJson += ",";
            headersJson += "\"" + key + "\":\"" + value + "\"";
            first = false;
        }
        headersJson += "}";
        response["headers"] = headersJson;
        
        response["body"] = responseBody;
        
        return response;
    }

private:
    // Costruttore privato per pattern Singleton
    NetworkManager() : m_initialized(false), m_nextConnectionId(0) {}
    
    // Distruttore privato
    ~NetworkManager() {
        closeAllConnections();
    }
    
    // Disabilita copia e assegnazione
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
    
    /**
     * @brief Registra i gestori di protocollo predefiniti.
     */
    void registerDefaultProtocolHandlers() {
        // In un'implementazione reale, registrerebbe gestori per i protocolli supportati
        // Per esempio:
        // m_protocolHandlers[NetworkProtocol::TCP] = std::make_shared<TCPProtocolHandler>();
        // m_protocolHandlers[NetworkProtocol::HTTP] = std::make_shared<HTTPProtocolHandler>();
        // ecc.
    }
    
    /**
     * @brief Ottiene un gestore di protocollo.
     * @param protocol Protocollo
     * @return Puntatore al gestore, o nullptr se non disponibile
     */
    std::shared_ptr<IProtocolHandler> getProtocolHandler(NetworkProtocol protocol) {
        auto it = m_protocolHandlers.find(protocol);
        if (it != m_protocolHandlers.end()) {
            return it->second;
        }
        
        // In un'implementazione reale, potrebbe creare un gestore on-demand
        // Per ora, restituiamo nullptr
        return nullptr;
    }
    
    /**
     * @brief Genera un ID univoco per una connessione.
     * @return ID della connessione
     */
    std::string generateConnectionId() {
        return "conn_" + std::to_string(m_nextConnectionId++);
    }
    
    /**
     * @brief Imposta l'ultimo errore.
     * @param error Errore da impostare
     */
    void setLastError(const NetworkError& error) {
        m_lastError = error;
    }
    
    bool m_initialized;
    int m_nextConnectionId;
    mutable std::mutex m_mutex;
    std::optional<NetworkError> m_lastError;
    
    std::unordered_map<NetworkProtocol, std::shared_ptr<IProtocolHandler>> m_protocolHandlers;
    std::unordered_map<std::string, std::shared_ptr<IProtocolHandler>> m_connections;
};

} // namespace Extensions
} // namespace Core 