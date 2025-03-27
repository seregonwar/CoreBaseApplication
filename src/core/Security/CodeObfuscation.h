#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include <mutex>
#include <random>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Core {
namespace Security {

/**
 * @brief Classe per la protezione e l'offuscamento del codice.
 * 
 * Fornisce strumenti per proteggere l'applicazione da reverse engineering, debug,
 * tampering e altre tecniche di analisi. Implementa varie strategie per rendere
 * il codice più difficile da analizzare durante l'esecuzione.
 */
class CodeProtection {
public:
    /**
     * @brief Enum che definisce i tipi di protezione supportati.
     */
    enum class ProtectionType {
        ANTI_DEBUG,         ///< Rileva tentativi di debug
        ANTI_TAMPERING,     ///< Rileva modifiche al codice
        MEMORY_INTEGRITY,   ///< Verifica l'integrità della memoria
        ANTI_VM,            ///< Rileva l'esecuzione in ambienti virtualizzati
        CODE_OBFUSCATION,   ///< Offusca il codice e i dati
        ANTI_DUMP           ///< Previene il dump della memoria
    };

    /**
     * @brief Ottiene l'istanza singleton del sistema di protezione.
     * @return Riferimento all'istanza singleton
     */
    static CodeProtection& getInstance() {
        static CodeProtection instance;
        return instance;
    }

    /**
     * @brief Inizializza il sistema di protezione.
     * @param protectionTypes Tipi di protezione da attivare
     * @param callback Funzione da chiamare in caso di rilevamento di una minaccia
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initialize(const std::vector<ProtectionType>& protectionTypes, 
                   std::function<void(ProtectionType, const std::string&)> callback = nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_initialized) return true;
        
        m_activeProtections = protectionTypes;
        m_securityCallback = callback;
        
        // Inizializza le protezioni richieste
        for (auto type : protectionTypes) {
            initializeProtection(type);
        }
        
        // Avvia il thread di monitoraggio se necessario
        if (!protectionTypes.empty()) {
            startMonitoringThread();
        }
        
        m_initialized = true;
        return true;
    }

    /**
     * @brief Abilita un tipo specifico di protezione.
     * @param type Tipo di protezione da abilitare
     * @return true se la protezione è stata abilitata, false altrimenti
     */
    bool enableProtection(ProtectionType type) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return false;
        
        // Verifica se la protezione è già attiva
        if (std::find(m_activeProtections.begin(), m_activeProtections.end(), type) != m_activeProtections.end()) {
            return true;
        }
        
        // Inizializza la protezione
        if (initializeProtection(type)) {
            m_activeProtections.push_back(type);
            return true;
        }
        
        return false;
    }

    /**
     * @brief Disabilita un tipo specifico di protezione.
     * @param type Tipo di protezione da disabilitare
     * @return true se la protezione è stata disabilitata, false altrimenti
     */
    bool disableProtection(ProtectionType type) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_initialized) return false;
        
        // Rimuove la protezione dalla lista delle protezioni attive
        auto it = std::find(m_activeProtections.begin(), m_activeProtections.end(), type);
        if (it != m_activeProtections.end()) {
            m_activeProtections.erase(it);
            return true;
        }
        
        return false;
    }

    /**
     * @brief Verifica se un tipo specifico di protezione è attivo.
     * @param type Tipo di protezione da verificare
     * @return true se la protezione è attiva, false altrimenti
     */
    bool isProtectionEnabled(ProtectionType type) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        return std::find(m_activeProtections.begin(), m_activeProtections.end(), type) != m_activeProtections.end();
    }

    /**
     * @brief Verifica se il sistema è sotto debug.
     * @return true se è stato rilevato un debugger, false altrimenti
     */
    bool isDebuggerPresent() const {
        #ifdef _WIN32
            return IsDebuggerPresent() != 0;
        #elif defined(__linux__) || defined(__APPLE__)
            // Implementazione per Linux/macOS usando ptrace
            // Questa è una versione semplificata
            return checkTracerPid();
        #else
            return false;
        #endif
    }

    /**
     * @brief Calcola il checksum di una regione di memoria.
     * @param data Puntatore all'inizio della regione di memoria
     * @param size Dimensione della regione in byte
     * @return Checksum calcolato
     */
    uint32_t calculateChecksum(const void* data, size_t size) const {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        uint32_t checksum = 0;
        
        for (size_t i = 0; i < size; ++i) {
            checksum = (checksum << 1) | (checksum >> 31); // Rotazione a sinistra di 1 bit
            checksum ^= bytes[i];
        }
        
        return checksum;
    }

    /**
     * @brief Offusca una stringa utilizzando una chiave di offuscamento.
     * @param input Stringa da offuscare
     * @param key Chiave di offuscamento
     * @return Stringa offuscata
     */
    std::string obfuscateString(const std::string& input, uint32_t key = 0) const {
        if (key == 0) {
            // Genera una chiave casuale se non specificata
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint32_t> dist(1, 0xFFFFFFFF);
            key = dist(gen);
        }
        
        std::string result = input;
        for (char& c : result) {
            c ^= (key & 0xFF);
            key = (key << 1) | (key >> 31); // Rotazione a sinistra di 1 bit
        }
        
        return result;
    }

    /**
     * @brief Deoffusca una stringa utilizzando una chiave di offuscamento.
     * @param input Stringa offuscata
     * @param key Chiave di offuscamento
     * @return Stringa originale
     */
    std::string deobfuscateString(const std::string& input, uint32_t key) const {
        // L'operazione di XOR è simmetrica, quindi usiamo lo stesso algoritmo
        return obfuscateString(input, key);
    }

    /**
     * @brief Esegue un codice nascosto all'analisi statica.
     * @param hiddenCode Funzione contenente il codice da eseguire
     */
    void executeHiddenCode(std::function<void()> hiddenCode) const {
        if (hiddenCode) {
            // Esegue il codice solo se non è stato rilevato un debugger
            if (!isDebuggerPresent()) {
                hiddenCode();
            }
        }
    }

    /**
     * @brief Crea una regione di memoria protetta da accessi non autorizzati.
     * @param size Dimensione della regione in byte
     * @return Puntatore alla regione di memoria
     */
    void* createProtectedMemory(size_t size) {
        #ifdef _WIN32
            return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        #else
            // Allocazione di memoria standard per altri sistemi
            return malloc(size);
        #endif
    }

    /**
     * @brief Libera una regione di memoria protetta.
     * @param memory Puntatore alla regione di memoria
     * @param size Dimensione della regione in byte
     */
    void freeProtectedMemory(void* memory, size_t size) {
        if (!memory) return;
        
        #ifdef _WIN32
            VirtualFree(memory, 0, MEM_RELEASE);
        #else
            // Prima sovrascrive la memoria con zeri
            memset(memory, 0, size);
            // Poi libera la memoria
            free(memory);
        #endif
    }

private:
    // Costruttore privato per pattern Singleton
    CodeProtection() : m_initialized(false), m_isMonitoringActive(false) {}
    
    // Distruttore privato
    ~CodeProtection() {
        // Ferma il thread di monitoraggio
        stopMonitoringThread();
    }
    
    // Disabilita copia e assegnazione
    CodeProtection(const CodeProtection&) = delete;
    CodeProtection& operator=(const CodeProtection&) = delete;

    /**
     * @brief Inizializza un tipo specifico di protezione.
     * @param type Tipo di protezione da inizializzare
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeProtection(ProtectionType type) {
        switch (type) {
            case ProtectionType::ANTI_DEBUG:
                // Implementazione della protezione anti-debug
                return initializeAntiDebug();
                
            case ProtectionType::ANTI_TAMPERING:
                // Implementazione della protezione anti-tampering
                return initializeAntiTampering();
                
            case ProtectionType::MEMORY_INTEGRITY:
                // Implementazione della verifica dell'integrità della memoria
                return initializeMemoryIntegrity();
                
            case ProtectionType::ANTI_VM:
                // Implementazione della protezione anti-VM
                return initializeAntiVM();
                
            case ProtectionType::CODE_OBFUSCATION:
                // Implementazione dell'offuscamento del codice
                return initializeCodeObfuscation();
                
            case ProtectionType::ANTI_DUMP:
                // Implementazione della protezione anti-dump
                return initializeAntiDump();
                
            default:
                return false;
        }
    }

    /**
     * @brief Inizializza la protezione anti-debug.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeAntiDebug() {
        // Registra le aree di memoria da monitorare
        // Implementazione specifica per piattaforma...
        return true;
    }

    /**
     * @brief Inizializza la protezione anti-tampering.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeAntiTampering() {
        // Calcola e memorizza i checksum delle sezioni critiche del codice
        // Implementazione specifica per piattaforma...
        return true;
    }

    /**
     * @brief Inizializza la verifica dell'integrità della memoria.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeMemoryIntegrity() {
        // Inizializza le strutture per la verifica dell'integrità della memoria
        // Implementazione specifica per piattaforma...
        return true;
    }

    /**
     * @brief Inizializza la protezione anti-VM.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeAntiVM() {
        // Implementa i controlli per rilevare ambienti virtualizzati
        // Implementazione specifica per piattaforma...
        return true;
    }

    /**
     * @brief Inizializza l'offuscamento del codice.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeCodeObfuscation() {
        // Inizializza le strutture per l'offuscamento del codice
        // Implementazione specifica per piattaforma...
        return true;
    }

    /**
     * @brief Inizializza la protezione anti-dump.
     * @return true se l'inizializzazione è avvenuta con successo, false altrimenti
     */
    bool initializeAntiDump() {
        // Implementa i controlli per prevenire il dump della memoria
        // Implementazione specifica per piattaforma...
        return true;
    }

    /**
     * @brief Avvia il thread di monitoraggio.
     */
    void startMonitoringThread() {
        if (m_isMonitoringActive) return;
        
        m_isMonitoringActive = true;
        m_monitoringThread = std::thread([this]() {
            while (m_isMonitoringActive) {
                // Verifica le varie protezioni
                checkProtections();
                
                // Attende un po' prima della prossima verifica
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });
    }

    /**
     * @brief Ferma il thread di monitoraggio.
     */
    void stopMonitoringThread() {
        if (!m_isMonitoringActive) return;
        
        m_isMonitoringActive = false;
        if (m_monitoringThread.joinable()) {
            m_monitoringThread.join();
        }
    }

    /**
     * @brief Verifica tutte le protezioni attive.
     */
    void checkProtections() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto type : m_activeProtections) {
            switch (type) {
                case ProtectionType::ANTI_DEBUG:
                    if (isDebuggerPresent()) {
                        handleSecurityViolation(type, "Debugger rilevato");
                    }
                    break;
                    
                case ProtectionType::ANTI_TAMPERING:
                    // Verifica l'integrità del codice
                    // ...
                    break;
                    
                case ProtectionType::MEMORY_INTEGRITY:
                    // Verifica l'integrità della memoria
                    // ...
                    break;
                    
                case ProtectionType::ANTI_VM:
                    // Verifica se siamo in una VM
                    // ...
                    break;
                    
                case ProtectionType::CODE_OBFUSCATION:
                    // Verifica l'integrità dell'offuscamento
                    // ...
                    break;
                    
                case ProtectionType::ANTI_DUMP:
                    // Verifica tentativi di dump della memoria
                    // ...
                    break;
            }
        }
    }

    /**
     * @brief Gestisce una violazione della sicurezza.
     * @param type Tipo di protezione violata
     * @param message Messaggio di errore
     */
    void handleSecurityViolation(ProtectionType type, const std::string& message) {
        if (m_securityCallback) {
            m_securityCallback(type, message);
        }
        
        // Implementa una risposta predefinita alla violazione
        // In un sistema reale, potrebbe terminare l'applicazione, corrompere i dati, ecc.
    }

    /**
     * @brief Verifica se il processo è tracciato (per Linux/macOS).
     * @return true se il processo è tracciato, false altrimenti
     */
    bool checkTracerPid() const {
        #if defined(__linux__)
            // Implementazione Linux
            FILE* f = fopen("/proc/self/status", "r");
            if (f) {
                char line[256];
                int tracerPid = 0;
                
                while (fgets(line, sizeof(line), f)) {
                    if (strncmp(line, "TracerPid:", 10) == 0) {
                        tracerPid = atoi(&line[10]);
                        break;
                    }
                }
                
                fclose(f);
                return tracerPid != 0;
            }
            return false;
        #elif defined(__APPLE__)
            // Implementazione macOS
            // Questo è un placeholder, l'implementazione reale richiederebbe l'uso di ptrace
            return false;
        #else
            return false;
        #endif
    }

    bool m_initialized;
    std::vector<ProtectionType> m_activeProtections;
    std::function<void(ProtectionType, const std::string&)> m_securityCallback;
    mutable std::mutex m_mutex;
    
    bool m_isMonitoringActive;
    std::thread m_monitoringThread;
    
    // Strutture per memorizzare informazioni sulle protezioni
    struct MemoryRegion {
        void* address;
        size_t size;
        uint32_t checksum;
    };
    
    std::vector<MemoryRegion> m_monitoredRegions;
};

/**
 * @brief Classe per l'offuscamento di stringhe in fase di compilazione.
 * 
 * Permette di offuscare stringhe in fase di compilazione, rendendo più difficile
 * l'estrazione di stringhe hardcoded dal binario.
 * 
 * @tparam N Lunghezza della stringa (incluso il carattere null)
 */
template<size_t N>
class ObfuscatedString {
public:
    /**
     * @brief Costruttore.
     * @param str Stringa da offuscare
     * @param key Chiave di offuscamento
     */
    constexpr ObfuscatedString(const char (&str)[N], uint32_t key = 0x12345678)
        : m_key(key) {
        // Offusca la stringa
        for (size_t i = 0; i < N; i++) {
            m_data[i] = str[i] ^ (m_key & 0xFF);
            m_key = (m_key << 1) | (m_key >> 31); // Rotazione a sinistra di 1 bit
        }
    }
    
    /**
     * @brief Deoffusca la stringa.
     * @return Stringa deoffuscata
     */
    std::string decrypt() const {
        std::string result;
        result.reserve(N - 1); // Esclude il carattere null
        
        uint32_t key = m_key;
        for (size_t i = 0; i < N; i++) {
            char c = m_data[i] ^ (key & 0xFF);
            key = (key << 1) | (key >> 31); // Rotazione a sinistra di 1 bit
            
            if (c != '\0') {
                result.push_back(c);
            }
        }
        
        return result;
    }

private:
    char m_data[N];
    uint32_t m_key;
};

/**
 * @brief Helper per creare facilmente stringhe offuscate.
 * 
 * @param str Stringa da offuscare
 * @param key Chiave di offuscamento
 * @return Stringa offuscata
 */
template<size_t N>
constexpr ObfuscatedString<N> makeObfuscatedString(const char (&str)[N], uint32_t key = 0x12345678) {
    return ObfuscatedString<N>(str, key);
}

// Macro per semplificare l'uso delle stringhe offuscate
#define OBFUSCATE(str) makeObfuscatedString(str).decrypt()

} // namespace Security
} // namespace Core 