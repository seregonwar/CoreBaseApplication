#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>

// Forward declarations
class ErrorHandler;

/**
 * @brief Enum che definisce i tipi di IPC supportati
 */
enum class IPCType {
    SHARED_MEMORY,
    NAMED_PIPE,
    SOCKET,
    MESSAGE_QUEUE
};

/**
 * @brief Struct che rappresenta le informazioni di un canale IPC
 */
struct IPCChannelInfo {
    std::string name;
    IPCType type;
    std::string description;
    bool isActive;
    void* handle;
    size_t size;
};

/**
 * @brief Enum che definisce il ruolo in una comunicazione IPC
 */
enum class IPCRole {
    SERVER,
    CLIENT
};

/**
 * @brief Callback per la gestione dei messaggi IPC
 */
using IPCMessageCallback = std::function<void(const std::string& channelName, const void* data, size_t dataSize)>;

/**
 * @brief Classe per la gestione della comunicazione tra processi
 */
class IPCManager {
public:
    /**
     * @brief Costruttore dell'IPCManager
     * @param errorHandler Riferimento all'ErrorHandler
     */
    IPCManager(ErrorHandler& errorHandler);
    
    /**
     * @brief Distruttore dell'IPCManager
     */
    ~IPCManager();
    
    /**
     * @brief Inizializza l'IPCManager
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize();
    
    /**
     * @brief Crea un'area di memoria condivisa
     * @param name Nome della memoria condivisa
     * @param size Dimensione della memoria in bytes
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createSharedMemory(const std::string& name, size_t size);
    
    /**
     * @brief Ottiene un puntatore alla memoria condivisa
     * @param name Nome della memoria condivisa
     * @return Puntatore alla memoria condivisa, nullptr se non esiste
     */
    void* getSharedMemory(const std::string& name);
    
    /**
     * @brief Rilascia una memoria condivisa
     * @param name Nome della memoria condivisa
     * @return true se il rilascio è avvenuto con successo, false altrimenti
     */
    bool releaseSharedMemory(const std::string& name);
    
    /**
     * @brief Crea un named pipe
     * @param name Nome del pipe
     * @param role Ruolo (server o client)
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createNamedPipe(const std::string& name, IPCRole role);
    
    /**
     * @brief Scrive dati su un named pipe
     * @param name Nome del pipe
     * @param data Dati da scrivere
     * @param dataSize Dimensione dei dati
     * @return true se la scrittura è avvenuta con successo, false altrimenti
     */
    bool writeToNamedPipe(const std::string& name, const void* data, size_t dataSize);
    
    /**
     * @brief Legge dati da un named pipe
     * @param name Nome del pipe
     * @param buffer Buffer in cui leggere i dati
     * @param bufferSize Dimensione del buffer
     * @param bytesRead Riferimento in cui salvare il numero di bytes letti
     * @return true se la lettura è avvenuta con successo, false altrimenti
     */
    bool readFromNamedPipe(const std::string& name, void* buffer, size_t bufferSize, size_t& bytesRead);
    
    /**
     * @brief Chiude un named pipe
     * @param name Nome del pipe
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool closeNamedPipe(const std::string& name);
    
    /**
     * @brief Crea un socket
     * @param name Nome del socket
     * @param role Ruolo (server o client)
     * @param host Hostname o indirizzo IP (solo per client)
     * @param port Porta (solo per client)
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createSocket(const std::string& name, IPCRole role, const std::string& host = "", int port = 0);
    
    /**
     * @brief Scrive dati su un socket
     * @param name Nome del socket
     * @param data Dati da scrivere
     * @param dataSize Dimensione dei dati
     * @return true se la scrittura è avvenuta con successo, false altrimenti
     */
    bool writeToSocket(const std::string& name, const void* data, size_t dataSize);
    
    /**
     * @brief Legge dati da un socket
     * @param name Nome del socket
     * @param buffer Buffer in cui leggere i dati
     * @param bufferSize Dimensione del buffer
     * @param bytesRead Riferimento in cui salvare il numero di bytes letti
     * @return true se la lettura è avvenuta con successo, false altrimenti
     */
    bool readFromSocket(const std::string& name, void* buffer, size_t bufferSize, size_t& bytesRead);
    
    /**
     * @brief Chiude un socket
     * @param name Nome del socket
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool closeSocket(const std::string& name);
    
    /**
     * @brief Crea una coda di messaggi
     * @param name Nome della coda
     * @return true se la creazione è avvenuta con successo, false altrimenti
     */
    bool createMessageQueue(const std::string& name);
    
    /**
     * @brief Invia un messaggio a una coda
     * @param name Nome della coda
     * @param data Dati da inviare
     * @param dataSize Dimensione dei dati
     * @return true se l'invio è avvenuto con successo, false altrimenti
     */
    bool sendMessage(const std::string& name, const void* data, size_t dataSize);
    
    /**
     * @brief Riceve un messaggio da una coda
     * @param name Nome della coda
     * @param buffer Buffer in cui leggere i dati
     * @param bufferSize Dimensione del buffer
     * @param bytesRead Riferimento in cui salvare il numero di bytes letti
     * @return true se la ricezione è avvenuta con successo, false altrimenti
     */
    bool receiveMessage(const std::string& name, void* buffer, size_t bufferSize, size_t& bytesRead);
    
    /**
     * @brief Chiude una coda di messaggi
     * @param name Nome della coda
     * @return true se la chiusura è avvenuta con successo, false altrimenti
     */
    bool closeMessageQueue(const std::string& name);
    
    /**
     * @brief Registra una callback per i messaggi ricevuti
     * @param name Nome del canale IPC
     * @param callback Funzione da chiamare quando arriva un messaggio
     * @return ID della callback registrata
     */
    int registerMessageCallback(const std::string& name, IPCMessageCallback callback);
    
    /**
     * @brief Rimuove una callback registrata
     * @param callbackId ID della callback da rimuovere
     * @return true se la callback è stata rimossa, false se non esisteva
     */
    bool unregisterMessageCallback(int callbackId);
    
private:
    ErrorHandler& m_errorHandler;
    std::mutex m_mutex;
    std::unordered_map<std::string, IPCChannelInfo> m_channels;
    std::unordered_map<int, std::pair<std::string, IPCMessageCallback>> m_callbacks;
    int m_nextCallbackId;
    
    /**
     * @brief Verifica se un canale IPC esiste
     * @param name Nome del canale
     * @param type Tipo di canale
     * @return true se il canale esiste, false altrimenti
     */
    bool channelExists(const std::string& name, IPCType type) const;
};
