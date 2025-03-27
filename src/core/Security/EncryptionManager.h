#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <random>
#include <array>

namespace Core {
namespace Security {

/**
 * @brief Algoritmi di cifratura supportati.
 */
enum class EncryptionAlgorithm {
    AES_256_GCM,   ///< AES 256 in modalità GCM (Galois/Counter Mode)
    AES_256_CBC,   ///< AES 256 in modalità CBC (Cipher Block Chaining)
    CHACHA20_POLY1305, ///< ChaCha20 con autenticazione Poly1305
    RSA_2048,      ///< RSA 2048 bit
    RSA_4096,      ///< RSA 4096 bit
    NONE          ///< Nessun algoritmo, dati in chiaro
};

/**
 * @brief Metodi di derivazione delle chiavi supportati.
 */
enum class KeyDerivationMethod {
    PBKDF2,        ///< Password-Based Key Derivation Function 2
    ARGON2,        ///< Argon2 (vincitore della Password Hashing Competition)
    SCRYPT,        ///< scrypt (memory-hard)
    NONE           ///< Nessuna derivazione, chiave usata direttamente
};

/**
 * @brief Livelli di forza password.
 */
enum class PasswordStrength {
    VERY_WEAK,     ///< Password molto debole
    WEAK,          ///< Password debole
    MODERATE,      ///< Password moderata
    STRONG,        ///< Password forte
    VERY_STRONG    ///< Password molto forte
};

/**
 * @brief Risultato di un'operazione di cifratura/decifratura.
 */
struct CryptoResult {
    bool success;                  ///< Indica se l'operazione è riuscita
    std::string errorMessage;      ///< Messaggio di errore, vuoto se success è true
    std::vector<uint8_t> data;     ///< Dati risultanti dall'operazione
    
    /**
     * @brief Costruttore per risultato di successo.
     * @param resultData Dati risultanti dall'operazione
     */
    static CryptoResult Success(const std::vector<uint8_t>& resultData) {
        return { true, "", resultData };
    }
    
    /**
     * @brief Costruttore per risultato di errore.
     * @param message Messaggio di errore
     */
    static CryptoResult Error(const std::string& message) {
        return { false, message, {} };
    }
};

/**
 * @brief Interfaccia per i provider di cifratura.
 * 
 * Questa interfaccia definisce i metodi che devono essere implementati
 * da tutti i provider di cifratura supportati.
 */
class ICryptoProvider {
public:
    virtual ~ICryptoProvider() = default;
    
    /**
     * @brief Inizializza il provider.
     * @return true se l'inizializzazione è riuscita, false altrimenti
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Ottiene l'algoritmo di cifratura supportato dal provider.
     * @return Algoritmo di cifratura
     */
    virtual EncryptionAlgorithm getAlgorithm() const = 0;
    
    /**
     * @brief Genera una chiave casuale per l'algoritmo.
     * @return Chiave generata
     */
    virtual std::vector<uint8_t> generateKey() = 0;
    
    /**
     * @brief Genera un vettore di inizializzazione casuale.
     * @return Vettore di inizializzazione generato
     */
    virtual std::vector<uint8_t> generateIV() = 0;
    
    /**
     * @brief Cifra i dati.
     * @param plaintext Dati da cifrare
     * @param key Chiave di cifratura
     * @param iv Vettore di inizializzazione (opzionale per alcuni algoritmi)
     * @return Risultato dell'operazione
     */
    virtual CryptoResult encrypt(const std::vector<uint8_t>& plaintext, 
                                 const std::vector<uint8_t>& key,
                                 const std::vector<uint8_t>& iv = {}) = 0;
    
    /**
     * @brief Decifra i dati.
     * @param ciphertext Dati da decifrare
     * @param key Chiave di cifratura
     * @param iv Vettore di inizializzazione (opzionale per alcuni algoritmi)
     * @return Risultato dell'operazione
     */
    virtual CryptoResult decrypt(const std::vector<uint8_t>& ciphertext,
                                 const std::vector<uint8_t>& key,
                                 const std::vector<uint8_t>& iv = {}) = 0;
};

/**
 * @brief Provider di cifratura che non esegue alcuna operazione.
 * 
 * Utile per test o per quando la cifratura non è necessaria.
 */
class NoneCryptoProvider : public ICryptoProvider {
public:
    bool initialize() override { return true; }
    
    EncryptionAlgorithm getAlgorithm() const override { 
        return EncryptionAlgorithm::NONE; 
    }
    
    std::vector<uint8_t> generateKey() override { 
        return std::vector<uint8_t>(32, 0); 
    }
    
    std::vector<uint8_t> generateIV() override { 
        return std::vector<uint8_t>(16, 0); 
    }
    
    CryptoResult encrypt(const std::vector<uint8_t>& plaintext, 
                        const std::vector<uint8_t>&,
                        const std::vector<uint8_t>&) override {
        return CryptoResult::Success(plaintext);
    }
    
    CryptoResult decrypt(const std::vector<uint8_t>& ciphertext,
                        const std::vector<uint8_t>&,
                        const std::vector<uint8_t>&) override {
        return CryptoResult::Success(ciphertext);
    }
};

/**
 * @brief Funzioni di utilità per operazioni crittografiche.
 */
class CryptoUtils {
public:
    /**
     * @brief Genera un numero casuale sicuro.
     * @param min Valore minimo
     * @param max Valore massimo
     * @return Numero casuale generato
     */
    static int secureRandom(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }
    
    /**
     * @brief Genera una sequenza di byte casuali.
     * @param length Lunghezza della sequenza
     * @return Vettore di byte casuali
     */
    static std::vector<uint8_t> generateRandomBytes(size_t length) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint8_t> dist(0, 255);
        
        std::vector<uint8_t> result(length);
        for (size_t i = 0; i < length; ++i) {
            result[i] = dist(gen);
        }
        
        return result;
    }
    
    /**
     * @brief Genera una password casuale.
     * @param length Lunghezza della password
     * @param includeUppercase Se true, include lettere maiuscole
     * @param includeNumbers Se true, include numeri
     * @param includeSpecial Se true, include caratteri speciali
     * @return Password generata
     */
    static std::string generateRandomPassword(size_t length, 
                                             bool includeUppercase = true,
                                             bool includeNumbers = true,
                                             bool includeSpecial = true) {
        const std::string lowercase = "abcdefghijklmnopqrstuvwxyz";
        const std::string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const std::string numbers = "0123456789";
        const std::string special = "!@#$%^&*()_+-=[]{}|;:,.<>?";
        
        std::string charset = lowercase;
        if (includeUppercase) charset += uppercase;
        if (includeNumbers) charset += numbers;
        if (includeSpecial) charset += special;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, charset.length() - 1);
        
        std::string password;
        password.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            password += charset[dist(gen)];
        }
        
        return password;
    }
    
    /**
     * @brief Valuta la forza di una password.
     * @param password Password da valutare
     * @return Livello di forza della password
     */
    static PasswordStrength evaluatePasswordStrength(const std::string& password) {
        int score = 0;
        
        // Lunghezza
        if (password.length() >= 12) score += 2;
        else if (password.length() >= 8) score += 1;
        
        // Complessità
        bool hasLowercase = false;
        bool hasUppercase = false;
        bool hasDigit = false;
        bool hasSpecial = false;
        
        for (char c : password) {
            if (std::islower(c)) hasLowercase = true;
            else if (std::isupper(c)) hasUppercase = true;
            else if (std::isdigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        
        if (hasLowercase) score += 1;
        if (hasUppercase) score += 1;
        if (hasDigit) score += 1;
        if (hasSpecial) score += 1;
        
        // Varietà di caratteri
        std::array<int, 128> charCounts = {0};
        for (char c : password) {
            if (c >= 0 && c < 128) {
                charCounts[c]++;
            }
        }
        
        int uniqueChars = 0;
        for (int count : charCounts) {
            if (count > 0) uniqueChars++;
        }
        
        if (uniqueChars >= 8) score += 2;
        else if (uniqueChars >= 5) score += 1;
        
        // Valutazione finale
        if (score >= 7) return PasswordStrength::VERY_STRONG;
        if (score >= 5) return PasswordStrength::STRONG;
        if (score >= 3) return PasswordStrength::MODERATE;
        if (score >= 1) return PasswordStrength::WEAK;
        return PasswordStrength::VERY_WEAK;
    }
    
    /**
     * @brief Converte una stringa in un vettore di byte.
     * @param str Stringa da convertire
     * @return Vettore di byte
     */
    static std::vector<uint8_t> stringToBytes(const std::string& str) {
        return std::vector<uint8_t>(str.begin(), str.end());
    }
    
    /**
     * @brief Converte un vettore di byte in una stringa.
     * @param bytes Vettore di byte da convertire
     * @return Stringa
     */
    static std::string bytesToString(const std::vector<uint8_t>& bytes) {
        return std::string(bytes.begin(), bytes.end());
    }
    
    /**
     * @brief Converte una stringa in formato esadecimale.
     * @param bytes Vettore di byte da convertire
     * @return Stringa esadecimale
     */
    static std::string bytesToHex(const std::vector<uint8_t>& bytes) {
        static const char hexChars[] = "0123456789abcdef";
        std::string result;
        result.reserve(bytes.size() * 2);
        
        for (uint8_t b : bytes) {
            result.push_back(hexChars[b >> 4]);
            result.push_back(hexChars[b & 0x0F]);
        }
        
        return result;
    }
    
    /**
     * @brief Converte una stringa esadecimale in un vettore di byte.
     * @param hex Stringa esadecimale
     * @return Vettore di byte
     */
    static std::vector<uint8_t> hexToBytes(const std::string& hex) {
        std::vector<uint8_t> result;
        result.reserve(hex.length() / 2);
        
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteStr = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
            result.push_back(byte);
        }
        
        return result;
    }
};

/**
 * @brief Gestore principale per le operazioni di crittografia.
 * 
 * Questa classe fornisce un'interfaccia unificata per le operazioni
 * di cifratura, decifratura, gestione delle chiavi e altre funzionalità
 * di sicurezza.
 */
class EncryptionManager {
public:
    /**
     * @brief Ottiene l'istanza singleton del gestore.
     * @return Riferimento all'istanza singleton
     */
    static EncryptionManager& getInstance() {
        static EncryptionManager instance;
        return instance;
    }
    
    /**
     * @brief Inizializza il gestore.
     * @return true se l'inizializzazione è riuscita, false altrimenti
     */
    bool initialize() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Inizializziamo il provider predefinito
        m_defaultProvider = std::make_shared<NoneCryptoProvider>();
        if (!m_defaultProvider->initialize()) {
            m_defaultProvider.reset();
            return false;
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
     * @brief Registra un provider di cifratura.
     * @param provider Provider da registrare
     * @return true se la registrazione è riuscita, false altrimenti
     */
    bool registerProvider(std::shared_ptr<ICryptoProvider> provider) {
        if (!provider) return false;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!provider->initialize()) {
            return false;
        }
        
        auto algorithm = provider->getAlgorithm();
        m_providers[algorithm] = provider;
        
        return true;
    }
    
    /**
     * @brief Imposta l'algoritmo di cifratura predefinito.
     * @param algorithm Algoritmo da impostare come predefinito
     * @return true se l'impostazione è riuscita, false altrimenti
     */
    bool setDefaultAlgorithm(EncryptionAlgorithm algorithm) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_providers.find(algorithm);
        if (it == m_providers.end()) {
            return false;
        }
        
        m_defaultProvider = it->second;
        return true;
    }
    
    /**
     * @brief Ottiene l'algoritmo di cifratura predefinito.
     * @return Algoritmo predefinito
     */
    EncryptionAlgorithm getDefaultAlgorithm() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_defaultProvider) {
            return EncryptionAlgorithm::NONE;
        }
        
        return m_defaultProvider->getAlgorithm();
    }
    
    /**
     * @brief Genera una chiave per l'algoritmo specificato.
     * @param algorithm Algoritmo per cui generare la chiave
     * @return Chiave generata o vuota in caso di errore
     */
    std::vector<uint8_t> generateKey(EncryptionAlgorithm algorithm = EncryptionAlgorithm::NONE) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto provider = getProvider(algorithm);
        if (!provider) {
            return {};
        }
        
        return provider->generateKey();
    }
    
    /**
     * @brief Genera un vettore di inizializzazione per l'algoritmo specificato.
     * @param algorithm Algoritmo per cui generare l'IV
     * @return IV generato o vuoto in caso di errore
     */
    std::vector<uint8_t> generateIV(EncryptionAlgorithm algorithm = EncryptionAlgorithm::NONE) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto provider = getProvider(algorithm);
        if (!provider) {
            return {};
        }
        
        return provider->generateIV();
    }
    
    /**
     * @brief Cifra i dati.
     * @param plaintext Dati da cifrare
     * @param key Chiave di cifratura
     * @param iv Vettore di inizializzazione
     * @param algorithm Algoritmo da utilizzare
     * @return Risultato dell'operazione
     */
    CryptoResult encrypt(const std::vector<uint8_t>& plaintext,
                        const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv = {},
                        EncryptionAlgorithm algorithm = EncryptionAlgorithm::NONE) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto provider = getProvider(algorithm);
        if (!provider) {
            return CryptoResult::Error("Provider non disponibile per l'algoritmo specificato");
        }
        
        return provider->encrypt(plaintext, key, iv);
    }
    
    /**
     * @brief Cifra i dati di una stringa.
     * @param plaintext Stringa da cifrare
     * @param key Chiave di cifratura
     * @param iv Vettore di inizializzazione
     * @param algorithm Algoritmo da utilizzare
     * @return Risultato dell'operazione
     */
    CryptoResult encryptString(const std::string& plaintext,
                             const std::vector<uint8_t>& key,
                             const std::vector<uint8_t>& iv = {},
                             EncryptionAlgorithm algorithm = EncryptionAlgorithm::NONE) {
        return encrypt(CryptoUtils::stringToBytes(plaintext), key, iv, algorithm);
    }
    
    /**
     * @brief Decifra i dati.
     * @param ciphertext Dati da decifrare
     * @param key Chiave di cifratura
     * @param iv Vettore di inizializzazione
     * @param algorithm Algoritmo da utilizzare
     * @return Risultato dell'operazione
     */
    CryptoResult decrypt(const std::vector<uint8_t>& ciphertext,
                        const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& iv = {},
                        EncryptionAlgorithm algorithm = EncryptionAlgorithm::NONE) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto provider = getProvider(algorithm);
        if (!provider) {
            return CryptoResult::Error("Provider non disponibile per l'algoritmo specificato");
        }
        
        return provider->decrypt(ciphertext, key, iv);
    }
    
    /**
     * @brief Decifra i dati e restituisce una stringa.
     * @param ciphertext Dati da decifrare
     * @param key Chiave di cifratura
     * @param iv Vettore di inizializzazione
     * @param algorithm Algoritmo da utilizzare
     * @return Stringa decifrata o vuota in caso di errore
     */
    std::string decryptToString(const std::vector<uint8_t>& ciphertext,
                              const std::vector<uint8_t>& key,
                              const std::vector<uint8_t>& iv = {},
                              EncryptionAlgorithm algorithm = EncryptionAlgorithm::NONE) {
        auto result = decrypt(ciphertext, key, iv, algorithm);
        if (!result.success) {
            return "";
        }
        
        return CryptoUtils::bytesToString(result.data);
    }
    
    /**
     * @brief Ottiene il provider per l'algoritmo specificato.
     * 
     * Se non viene specificato un algoritmo o l'algoritmo non è disponibile,
     * viene restituito il provider predefinito.
     * 
     * @param algorithm Algoritmo richiesto
     * @return Provider o nullptr in caso di errore
     */
    std::shared_ptr<ICryptoProvider> getProvider(EncryptionAlgorithm algorithm = EncryptionAlgorithm::NONE) const {
        if (algorithm == EncryptionAlgorithm::NONE) {
            return m_defaultProvider;
        }
        
        auto it = m_providers.find(algorithm);
        if (it != m_providers.end()) {
            return it->second;
        }
        
        return m_defaultProvider;
    }
    
    /**
     * @brief Converte una password in una chiave utilizzando PBKDF2.
     * @param password Password da convertire
     * @param salt Sale per la derivazione
     * @param iterations Numero di iterazioni
     * @param keyLength Lunghezza della chiave in byte
     * @return Chiave derivata
     */
    std::vector<uint8_t> deriveKeyFromPassword(const std::string& password,
                                            const std::vector<uint8_t>& salt,
                                            int iterations = 10000,
                                            size_t keyLength = 32) {
        // Questa è una semplice implementazione di esempio.
        // In un'implementazione reale, si dovrebbe usare una libreria crittografica come OpenSSL o Botan.
        
        // Simuliamo la derivazione della chiave
        std::vector<uint8_t> result(keyLength);
        std::vector<uint8_t> passwordBytes = CryptoUtils::stringToBytes(password);
        
        // Semplice derivazione (non sicura, solo per dimostrazione)
        for (size_t i = 0; i < keyLength; ++i) {
            uint8_t byte = 0;
            for (size_t j = 0; j < passwordBytes.size(); ++j) {
                byte ^= passwordBytes[j];
            }
            for (size_t j = 0; j < salt.size(); ++j) {
                byte ^= salt[j % salt.size()];
            }
            byte ^= static_cast<uint8_t>(iterations % 256);
            byte ^= static_cast<uint8_t>(i);
            
            result[i] = byte;
        }
        
        return result;
    }
    
    /**
     * @brief Genera un sale casuale per la derivazione delle chiavi.
     * @param length Lunghezza del sale in byte
     * @return Sale generato
     */
    std::vector<uint8_t> generateSalt(size_t length = 16) {
        return CryptoUtils::generateRandomBytes(length);
    }
    
    /**
     * @brief Genera una password casuale.
     * @param length Lunghezza della password
     * @param includeUppercase Se true, include lettere maiuscole
     * @param includeNumbers Se true, include numeri
     * @param includeSpecial Se true, include caratteri speciali
     * @return Password generata
     */
    std::string generatePassword(size_t length = 16,
                               bool includeUppercase = true,
                               bool includeNumbers = true,
                               bool includeSpecial = true) {
        return CryptoUtils::generateRandomPassword(length, includeUppercase, includeNumbers, includeSpecial);
    }
    
    /**
     * @brief Valuta la forza di una password.
     * @param password Password da valutare
     * @return Livello di forza della password
     */
    PasswordStrength evaluatePasswordStrength(const std::string& password) {
        return CryptoUtils::evaluatePasswordStrength(password);
    }
    
private:
    // Costruttore privato per pattern Singleton
    EncryptionManager() : m_isInitialized(false) {}
    
    // Distruttore privato
    ~EncryptionManager() = default;
    
    // Disabilita copia e assegnazione
    EncryptionManager(const EncryptionManager&) = delete;
    EncryptionManager& operator=(const EncryptionManager&) = delete;
    
    bool m_isInitialized;
    std::shared_ptr<ICryptoProvider> m_defaultProvider;
    std::unordered_map<EncryptionAlgorithm, std::shared_ptr<ICryptoProvider>> m_providers;
    mutable std::mutex m_mutex;
};

/**
 * @brief Classe utilitaria per la protezione dei dati sensibili in memoria.
 * 
 * Questa classe permette di memorizzare dati sensibili (come password, chiavi, ecc.)
 * limitando il tempo in cui sono presenti in memoria in chiaro.
 */
template <typename T>
class SecureContainer {
public:
    /**
     * @brief Costruttore predefinito.
     */
    SecureContainer() : m_isLocked(true) {}
    
    /**
     * @brief Costruttore con dati iniziali.
     * @param data Dati iniziali
     * @param autoLock Se true, blocca automaticamente i dati dopo la creazione
     */
    SecureContainer(const T& data, bool autoLock = true) : m_data(data), m_isLocked(!autoLock) {
        if (autoLock) {
            lock();
        }
    }
    
    /**
     * @brief Distruttore.
     * 
     * Pulisce la memoria prima di distruggere l'oggetto.
     */
    ~SecureContainer() {
        clear();
    }
    
    /**
     * @brief Imposta i dati.
     * @param data Dati da memorizzare
     */
    void setData(const T& data) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data = data;
        m_isLocked = false;
    }
    
    /**
     * @brief Ottiene i dati.
     * 
     * Questo metodo fallisce se i dati sono bloccati.
     * 
     * @param data Riferimento dove memorizzare i dati
     * @return true se i dati sono stati ottenuti, false se erano bloccati
     */
    bool getData(T& data) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_isLocked) {
            return false;
        }
        
        data = m_data;
        return true;
    }
    
    /**
     * @brief Sblocca i dati.
     * @return true se i dati sono stati sbloccati, false se erano già sbloccati
     */
    bool unlock() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_isLocked) {
            return false;
        }
        
        m_isLocked = false;
        return true;
    }
    
    /**
     * @brief Blocca i dati.
     * @return true se i dati sono stati bloccati, false se erano già bloccati
     */
    bool lock() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_isLocked) {
            return false;
        }
        
        m_isLocked = true;
        return true;
    }
    
    /**
     * @brief Pulisce i dati.
     * 
     * Questo metodo sovrascrive la memoria utilizzata per i dati
     * prima di cancellarli, per evitare che rimangano in memoria.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Questo metodo dipende dal tipo T e potrebbe non funzionare per tutti i tipi
        // È un esempio di come si potrebbe implementare la pulizia sicura
        
        if constexpr (std::is_trivial_v<T>) {
            // Per tipi triviali (come int, float, ecc.) possiamo sovrascrivere direttamente la memoria
            volatile unsigned char* ptr = reinterpret_cast<volatile unsigned char*>(&m_data);
            size_t size = sizeof(T);
            
            // Sovrascriviamo con zeri
            for (size_t i = 0; i < size; ++i) {
                ptr[i] = 0;
            }
            
            // Sovrascriviamo con uno
            for (size_t i = 0; i < size; ++i) {
                ptr[i] = 0xFF;
            }
            
            // Sovrascriviamo nuovamente con zeri
            for (size_t i = 0; i < size; ++i) {
                ptr[i] = 0;
            }
        } else {
            // Per tipi non triviali (come std::string, std::vector, ecc.)
            // possiamo tentare di usare un operatore di assegnazione o un costruttore predefinito
            m_data = T();
        }
        
        m_isLocked = true;
    }
    
    /**
     * @brief Verifica se i dati sono bloccati.
     * @return true se i dati sono bloccati, false altrimenti
     */
    bool isLocked() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_isLocked;
    }
    
private:
    T m_data;
    bool m_isLocked;
    mutable std::mutex m_mutex;
};

// Alias per tipi comuni
using SecureString = SecureContainer<std::string>;
using SecureVector = SecureContainer<std::vector<uint8_t>>;

} // namespace Security
} // namespace Core 