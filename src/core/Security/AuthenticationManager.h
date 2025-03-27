#pragma once

#include "EncryptionManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
#include <optional>
#include <algorithm>

namespace Core {
namespace Security {

/**
 * @brief Livelli di privilegio.
 */
enum class PrivilegeLevel {
    GUEST,      ///< Accesso limitato, solo lettura per alcune risorse
    USER,       ///< Accesso base, lettura e scrittura per risorse proprie
    POWER_USER, ///< Accesso esteso, lettura e scrittura per risorse condivise
    ADMIN,      ///< Accesso amministrativo, gestione utenti e risorse
    SYSTEM      ///< Accesso completo, operazioni di sistema
};

/**
 * @brief Tipi di autenticazione supportati.
 */
enum class AuthenticationType {
    PASSWORD,       ///< Autenticazione tramite password
    TOKEN,          ///< Autenticazione tramite token
    CERTIFICATE,    ///< Autenticazione tramite certificato
    BIOMETRIC,      ///< Autenticazione biometrica
    TWO_FACTOR,     ///< Autenticazione a due fattori
    EXTERNAL        ///< Autenticazione delegata a un sistema esterno
};

/**
 * @brief Risultato di un'operazione di autenticazione.
 */
struct AuthResult {
    bool success;                  ///< Indica se l'autenticazione è riuscita
    std::string message;           ///< Messaggio informativo o di errore
    std::string sessionToken;      ///< Token di sessione, valido se success è true
    PrivilegeLevel level;          ///< Livello di privilegio dell'utente autenticato
    
    /**
     * @brief Costruttore per risultato di successo.
     * @param token Token di sessione
     * @param userLevel Livello di privilegio
     * @param infoMessage Messaggio informativo
     */
    static AuthResult Success(const std::string& token, PrivilegeLevel userLevel, const std::string& infoMessage = "") {
        return { true, infoMessage, token, userLevel };
    }
    
    /**
     * @brief Costruttore per risultato di errore.
     * @param errorMessage Messaggio di errore
     */
    static AuthResult Error(const std::string& errorMessage) {
        return { false, errorMessage, "", PrivilegeLevel::GUEST };
    }
};

/**
 * @brief Informazioni su un utente.
 */
struct UserInfo {
    std::string username;                  ///< Nome utente
    std::string fullName;                  ///< Nome completo
    std::string email;                     ///< Indirizzo email
    PrivilegeLevel level;                  ///< Livello di privilegio
    std::vector<AuthenticationType> authTypes; ///< Tipi di autenticazione supportati
    std::chrono::system_clock::time_point lastLogin; ///< Data e ora dell'ultimo accesso
    bool isActive;                         ///< Indica se l'utente è attivo
    bool isLocked;                         ///< Indica se l'utente è bloccato
    int failedLoginAttempts;               ///< Numero di tentativi di accesso falliti
    
    /**
     * @brief Costruttore.
     * @param user Nome utente
     * @param name Nome completo
     * @param mail Indirizzo email
     * @param privLevel Livello di privilegio
     */
    UserInfo(const std::string& user = "",
            const std::string& name = "",
            const std::string& mail = "",
            PrivilegeLevel privLevel = PrivilegeLevel::GUEST)
        : username(user), fullName(name), email(mail), level(privLevel),
          isActive(true), isLocked(false), failedLoginAttempts(0) {}
};

/**
 * @brief Informazioni su una sessione.
 */
struct SessionInfo {
    std::string token;                                 ///< Token di sessione
    std::string username;                              ///< Nome utente
    PrivilegeLevel level;                              ///< Livello di privilegio
    std::chrono::system_clock::time_point creationTime; ///< Data e ora di creazione
    std::chrono::system_clock::time_point lastActivity; ///< Data e ora dell'ultima attività
    std::chrono::seconds timeout;                      ///< Timeout di inattività
    bool isValid;                                      ///< Indica se la sessione è valida
    
    /**
     * @brief Verifica se la sessione è scaduta.
     * @return true se la sessione è scaduta, false altrimenti
     */
    bool isExpired() const {
        auto now = std::chrono::system_clock::now();
        return now - lastActivity > timeout;
    }
    
    /**
     * @brief Aggiorna l'orario dell'ultima attività.
     */
    void updateLastActivity() {
        lastActivity = std::chrono::system_clock::now();
    }
};

/**
 * @brief Interfaccia per i provider di autenticazione.
 * 
 * Questa interfaccia definisce i metodi che devono essere implementati
 * da tutti i provider di autenticazione supportati.
 */
class IAuthProvider {
public:
    virtual ~IAuthProvider() = default;
    
    /**
     * @brief Inizializza il provider.
     * @return true se l'inizializzazione è riuscita, false altrimenti
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Ottiene il tipo di autenticazione supportato dal provider.
     * @return Tipo di autenticazione
     */
    virtual AuthenticationType getType() const = 0;
    
    /**
     * @brief Verifica le credenziali di un utente.
     * @param username Nome utente
     * @param credentials Credenziali (password, token, ecc.)
     * @return true se le credenziali sono valide, false altrimenti
     */
    virtual bool verifyCredentials(const std::string& username, const std::string& credentials) = 0;
    
    /**
     * @brief Imposta le credenziali di un utente.
     * @param username Nome utente
     * @param credentials Nuove credenziali
     * @return true se l'operazione è riuscita, false altrimenti
     */
    virtual bool setCredentials(const std::string& username, const std::string& credentials) = 0;
    
    /**
     * @brief Verifica se un utente esiste.
     * @param username Nome utente
     * @return true se l'utente esiste, false altrimenti
     */
    virtual bool userExists(const std::string& username) = 0;
};

/**
 * @brief Provider di autenticazione tramite password.
 */
class PasswordAuthProvider : public IAuthProvider {
public:
    /**
     * @brief Costruttore.
     * @param encryptionManager Gestore della crittografia
     */
    PasswordAuthProvider(EncryptionManager& encryptionManager)
        : m_encryptionManager(encryptionManager) {}
    
    bool initialize() override {
        // Verifichiamo che il gestore della crittografia sia inizializzato
        return m_encryptionManager.isInitialized();
    }
    
    AuthenticationType getType() const override {
        return AuthenticationType::PASSWORD;
    }
    
    bool verifyCredentials(const std::string& username, const std::string& password) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_hashedPasswords.find(username);
        if (it == m_hashedPasswords.end()) {
            return false;
        }
        
        const auto& storedHash = it->second.first;
        const auto& storedSalt = it->second.second;
        
        // Derivo la chiave dalla password e dal sale
        auto derivedKey = m_encryptionManager.deriveKeyFromPassword(password, storedSalt);
        
        // Converto la chiave derivata in formato esadecimale
        auto derivedHash = CryptoUtils::bytesToHex(derivedKey);
        
        // Verifico se corrisponde all'hash memorizzato
        return derivedHash == storedHash;
    }
    
    bool setCredentials(const std::string& username, const std::string& password) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Valuto la forza della password
        auto strength = m_encryptionManager.evaluatePasswordStrength(password);
        if (strength == PasswordStrength::VERY_WEAK || strength == PasswordStrength::WEAK) {
            return false; // Password troppo debole
        }
        
        // Genero un sale casuale
        auto salt = m_encryptionManager.generateSalt();
        
        // Derivo la chiave dalla password e dal sale
        auto derivedKey = m_encryptionManager.deriveKeyFromPassword(password, salt);
        
        // Converto la chiave derivata in formato esadecimale
        auto derivedHash = CryptoUtils::bytesToHex(derivedKey);
        
        // Memorizzo l'hash e il sale
        m_hashedPasswords[username] = std::make_pair(derivedHash, salt);
        
        return true;
    }
    
    bool userExists(const std::string& username) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_hashedPasswords.find(username) != m_hashedPasswords.end();
    }
    
private:
    EncryptionManager& m_encryptionManager;
    std::unordered_map<std::string, std::pair<std::string, std::vector<uint8_t>>> m_hashedPasswords;
    std::mutex m_mutex;
};

/**
 * @brief Gestore principale per le operazioni di autenticazione.
 * 
 * Questa classe fornisce un'interfaccia unificata per le operazioni
 * di autenticazione, gestione degli utenti e delle sessioni.
 */
class AuthenticationManager {
public:
    /**
     * @brief Ottiene l'istanza singleton del gestore.
     * @return Riferimento all'istanza singleton
     */
    static AuthenticationManager& getInstance() {
        static AuthenticationManager instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il gestore.
     * @param encryptionManager Gestore della crittografia
     * @return true se l'inizializzazione è riuscita, false altrimenti
     */
    bool initialize(EncryptionManager& encryptionManager) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_isInitialized) return true;
        
        m_encryptionManager = &encryptionManager;
        
        // Verifichiamo che il gestore della crittografia sia inizializzato
        if (!m_encryptionManager->isInitialized()) {
            return false;
        }
        
        // Registriamo il provider di autenticazione tramite password
        auto passwordProvider = std::make_shared<PasswordAuthProvider>(encryptionManager);
        if (!passwordProvider->initialize()) {
            return false;
        }
        
        registerAuthProvider(passwordProvider);
        
        // Imposto il timeout di sessione predefinito a 30 minuti
        m_sessionTimeout = std::chrono::minutes(30);
        
        // Creo un utente amministratore se non esiste
        if (!userExists("admin")) {
            UserInfo adminUser("admin", "Administrator", "admin@example.com", PrivilegeLevel::ADMIN);
            adminUser.authTypes.push_back(AuthenticationType::PASSWORD);
            createUser(adminUser, "admin123");
        }
        
        m_isInitialized = true;
        return true;
    }
    
    /**
     * @brief Verifica se il gestore è inizializzato.
     * @return true se il gestore è inizializzato, false altrimenti
     */
    bool isInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_isInitialized;
    }
    
    /**
     * @brief Registra un provider di autenticazione.
     * @param provider Provider da registrare
     * @return true se la registrazione è riuscita, false altrimenti
     */
    bool registerAuthProvider(std::shared_ptr<IAuthProvider> provider) {
        if (!provider) return false;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!provider->initialize()) {
            return false;
        }
        
        auto type = provider->getType();
        m_authProviders[type] = provider;
        
        return true;
    }
    
    /**
     * @brief Crea un nuovo utente.
     * @param userInfo Informazioni sull'utente
     * @param initialPassword Password iniziale (opzionale)
     * @return true se la creazione è riuscita, false altrimenti
     */
    bool createUser(const UserInfo& userInfo, const std::string& initialPassword = "") {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        // Verifico se l'utente esiste già
        if (m_users.find(userInfo.username) != m_users.end()) {
            return false;
        }
        
        // Creo l'utente
        m_users[userInfo.username] = userInfo;
        
        // Se è stata fornita una password iniziale, la imposto
        if (!initialPassword.empty() && 
            std::find(userInfo.authTypes.begin(), userInfo.authTypes.end(), AuthenticationType::PASSWORD) != userInfo.authTypes.end()) {
            auto passwordProvider = getAuthProvider(AuthenticationType::PASSWORD);
            if (passwordProvider) {
                if (!passwordProvider->setCredentials(userInfo.username, initialPassword)) {
                    // Se non riesco a impostare la password, elimino l'utente
                    m_users.erase(userInfo.username);
                    return false;
                }
            }
        }
        
        return true;
    }
    
    /**
     * @brief Aggiorna le informazioni di un utente.
     * @param username Nome utente
     * @param userInfo Nuove informazioni
     * @return true se l'aggiornamento è riuscito, false altrimenti
     */
    bool updateUser(const std::string& username, const UserInfo& userInfo) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        // Verifico se l'utente esiste
        auto it = m_users.find(username);
        if (it == m_users.end()) {
            return false;
        }
        
        // Aggiorno l'utente
        it->second = userInfo;
        
        return true;
    }
    
    /**
     * @brief Elimina un utente.
     * @param username Nome utente
     * @return true se l'eliminazione è riuscita, false altrimenti
     */
    bool deleteUser(const std::string& username) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        // Verifico se l'utente esiste
        auto it = m_users.find(username);
        if (it == m_users.end()) {
            return false;
        }
        
        // Elimino tutte le sessioni dell'utente
        auto sessionIt = m_sessions.begin();
        while (sessionIt != m_sessions.end()) {
            if (sessionIt->second.username == username) {
                sessionIt = m_sessions.erase(sessionIt);
            } else {
                ++sessionIt;
            }
        }
        
        // Elimino l'utente
        m_users.erase(it);
        
        return true;
    }
    
    /**
     * @brief Verifica se un utente esiste.
     * @param username Nome utente
     * @return true se l'utente esiste, false altrimenti
     */
    bool userExists(const std::string& username) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        return m_users.find(username) != m_users.end();
    }
    
    /**
     * @brief Ottiene le informazioni su un utente.
     * @param username Nome utente
     * @return Informazioni sull'utente o nullopt se l'utente non esiste
     */
    std::optional<UserInfo> getUserInfo(const std::string& username) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return std::nullopt;
        
        auto it = m_users.find(username);
        if (it == m_users.end()) {
            return std::nullopt;
        }
        
        return it->second;
    }
    
    /**
     * @brief Ottiene l'elenco di tutti gli utenti.
     * @return Elenco degli utenti
     */
    std::vector<UserInfo> getAllUsers() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return {};
        
        std::vector<UserInfo> users;
        users.reserve(m_users.size());
        
        for (const auto& pair : m_users) {
            users.push_back(pair.second);
        }
        
        return users;
    }
    
    /**
     * @brief Imposta la password di un utente.
     * @param username Nome utente
     * @param password Nuova password
     * @return true se l'operazione è riuscita, false altrimenti
     */
    bool setPassword(const std::string& username, const std::string& password) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        // Verifico se l'utente esiste
        auto it = m_users.find(username);
        if (it == m_users.end()) {
            return false;
        }
        
        // Verifico se l'utente supporta l'autenticazione tramite password
        auto& authTypes = it->second.authTypes;
        if (std::find(authTypes.begin(), authTypes.end(), AuthenticationType::PASSWORD) == authTypes.end()) {
            return false;
        }
        
        // Ottengo il provider di autenticazione tramite password
        auto passwordProvider = getAuthProvider(AuthenticationType::PASSWORD);
        if (!passwordProvider) {
            return false;
        }
        
        // Imposto la password
        return passwordProvider->setCredentials(username, password);
    }
    
    /**
     * @brief Autentica un utente.
     * @param username Nome utente
     * @param credentials Credenziali (password, token, ecc.)
     * @param authType Tipo di autenticazione
     * @return Risultato dell'autenticazione
     */
    AuthResult authenticate(const std::string& username, const std::string& credentials, AuthenticationType authType) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) {
            return AuthResult::Error("Il gestore dell'autenticazione non è inizializzato");
        }
        
        // Verifico se l'utente esiste
        auto userIt = m_users.find(username);
        if (userIt == m_users.end()) {
            return AuthResult::Error("Utente non trovato");
        }
        
        // Verifico se l'utente è attivo
        if (!userIt->second.isActive) {
            return AuthResult::Error("L'utente non è attivo");
        }
        
        // Verifico se l'utente è bloccato
        if (userIt->second.isLocked) {
            return AuthResult::Error("L'utente è bloccato");
        }
        
        // Verifico se l'utente supporta il tipo di autenticazione richiesto
        auto& authTypes = userIt->second.authTypes;
        if (std::find(authTypes.begin(), authTypes.end(), authType) == authTypes.end()) {
            return AuthResult::Error("Il tipo di autenticazione richiesto non è supportato per questo utente");
        }
        
        // Ottengo il provider di autenticazione
        auto provider = getAuthProvider(authType);
        if (!provider) {
            return AuthResult::Error("Provider di autenticazione non disponibile");
        }
        
        // Verifico le credenziali
        bool isAuthenticated = provider->verifyCredentials(username, credentials);
        if (!isAuthenticated) {
            // Incremento il contatore dei tentativi falliti
            userIt->second.failedLoginAttempts++;
            
            // Blocco l'utente dopo 5 tentativi falliti
            if (userIt->second.failedLoginAttempts >= 5) {
                userIt->second.isLocked = true;
                return AuthResult::Error("Troppi tentativi falliti, l'utente è stato bloccato");
            }
            
            return AuthResult::Error("Credenziali non valide");
        }
        
        // Resetto il contatore dei tentativi falliti
        userIt->second.failedLoginAttempts = 0;
        
        // Aggiorno la data dell'ultimo accesso
        userIt->second.lastLogin = std::chrono::system_clock::now();
        
        // Creo una nuova sessione
        auto sessionToken = generateSessionToken();
        SessionInfo session;
        session.token = sessionToken;
        session.username = username;
        session.level = userIt->second.level;
        session.creationTime = std::chrono::system_clock::now();
        session.lastActivity = session.creationTime;
        session.timeout = m_sessionTimeout;
        session.isValid = true;
        
        m_sessions[sessionToken] = session;
        
        return AuthResult::Success(sessionToken, userIt->second.level, "Autenticazione riuscita");
    }
    
    /**
     * @brief Verifica se una sessione è valida.
     * @param sessionToken Token di sessione
     * @return true se la sessione è valida, false altrimenti
     */
    bool isSessionValid(const std::string& sessionToken) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        auto it = m_sessions.find(sessionToken);
        if (it == m_sessions.end()) {
            return false;
        }
        
        if (!it->second.isValid || it->second.isExpired()) {
            // Elimina la sessione scaduta
            m_sessions.erase(it);
            return false;
        }
        
        // Aggiorna l'orario dell'ultima attività
        it->second.updateLastActivity();
        
        return true;
    }
    
    /**
     * @brief Ottiene le informazioni su una sessione.
     * @param sessionToken Token di sessione
     * @return Informazioni sulla sessione o nullopt se la sessione non esiste o non è valida
     */
    std::optional<SessionInfo> getSessionInfo(const std::string& sessionToken) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return std::nullopt;
        
        auto it = m_sessions.find(sessionToken);
        if (it == m_sessions.end()) {
            return std::nullopt;
        }
        
        if (!it->second.isValid || it->second.isExpired()) {
            // Elimina la sessione scaduta
            m_sessions.erase(it);
            return std::nullopt;
        }
        
        // Aggiorna l'orario dell'ultima attività
        it->second.updateLastActivity();
        
        return it->second;
    }
    
    /**
     * @brief Chiude una sessione.
     * @param sessionToken Token di sessione
     * @return true se la sessione è stata chiusa, false altrimenti
     */
    bool closeSession(const std::string& sessionToken) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        auto it = m_sessions.find(sessionToken);
        if (it == m_sessions.end()) {
            return false;
        }
        
        m_sessions.erase(it);
        return true;
    }
    
    /**
     * @brief Chiude tutte le sessioni di un utente.
     * @param username Nome utente
     * @return Numero di sessioni chiuse
     */
    int closeUserSessions(const std::string& username) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return 0;
        
        int count = 0;
        auto it = m_sessions.begin();
        while (it != m_sessions.end()) {
            if (it->second.username == username) {
                it = m_sessions.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        
        return count;
    }
    
    /**
     * @brief Imposta il timeout di inattività per le sessioni.
     * @param timeout Timeout in secondi
     */
    void setSessionTimeout(std::chrono::seconds timeout) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessionTimeout = timeout;
    }
    
    /**
     * @brief Ottiene il timeout di inattività per le sessioni.
     * @return Timeout in secondi
     */
    std::chrono::seconds getSessionTimeout() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_sessionTimeout;
    }
    
    /**
     * @brief Verifica se un utente ha un certo livello di privilegio.
     * @param username Nome utente
     * @param level Livello di privilegio richiesto
     * @return true se l'utente ha almeno il livello richiesto, false altrimenti
     */
    bool hasPrivilege(const std::string& username, PrivilegeLevel level) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        auto it = m_users.find(username);
        if (it == m_users.end()) {
            return false;
        }
        
        return it->second.level >= level;
    }
    
    /**
     * @brief Verifica se una sessione ha un certo livello di privilegio.
     * @param sessionToken Token di sessione
     * @param level Livello di privilegio richiesto
     * @return true se la sessione ha almeno il livello richiesto, false altrimenti
     */
    bool hasSessionPrivilege(const std::string& sessionToken, PrivilegeLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isInitialized) return false;
        
        auto it = m_sessions.find(sessionToken);
        if (it == m_sessions.end()) {
            return false;
        }
        
        if (!it->second.isValid || it->second.isExpired()) {
            // Elimina la sessione scaduta
            m_sessions.erase(it);
            return false;
        }
        
        // Aggiorna l'orario dell'ultima attività
        it->second.updateLastActivity();
        
        return it->second.level >= level;
    }
    
private:
    // Costruttore privato per pattern Singleton
    AuthenticationManager() : m_isInitialized(false), m_encryptionManager(nullptr) {}
    
    // Distruttore privato
    ~AuthenticationManager() = default;
    
    // Disabilita copia e assegnazione
    AuthenticationManager(const AuthenticationManager&) = delete;
    AuthenticationManager& operator=(const AuthenticationManager&) = delete;
    
    /**
     * @brief Ottiene un provider di autenticazione.
     * @param type Tipo di autenticazione
     * @return Provider richiesto o nullptr se non disponibile
     */
    std::shared_ptr<IAuthProvider> getAuthProvider(AuthenticationType type) const {
        auto it = m_authProviders.find(type);
        if (it != m_authProviders.end()) {
            return it->second;
        }
        
        return nullptr;
    }
    
    /**
     * @brief Genera un token di sessione.
     * @return Token generato
     */
    std::string generateSessionToken() const {
        // Genero 32 byte casuali
        auto bytes = m_encryptionManager->generateKey();
        
        // Converto in formato esadecimale
        return CryptoUtils::bytesToHex(bytes);
    }
    
    bool m_isInitialized;
    std::chrono::seconds m_sessionTimeout;
    EncryptionManager* m_encryptionManager;
    
    std::unordered_map<AuthenticationType, std::shared_ptr<IAuthProvider>> m_authProviders;
    std::unordered_map<std::string, UserInfo> m_users;
    std::unordered_map<std::string, SessionInfo> m_sessions;
    
    mutable std::mutex m_mutex;
};

} // namespace Security
} // namespace Core 