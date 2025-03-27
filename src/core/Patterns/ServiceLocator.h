#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <string>
#include <mutex>

namespace Core {
namespace Patterns {

/**
 * @brief Implementazione del pattern Service Locator per la dependency injection.
 * 
 * Questa classe fornisce un meccanismo centralizzato per registrare e accedere
 * a servizi condivisi nell'applicazione, facilitando la dependency injection e
 * migliorando la testabilità del codice.
 */
class ServiceLocator {
public:
    /**
     * @brief Registra un servizio.
     * 
     * @tparam T Tipo del servizio
     * @param service Puntatore condiviso al servizio
     * @param name Nome opzionale del servizio (utile per registrare multiple implementazioni dello stesso tipo)
     */
    template<typename T>
    static void registerService(std::shared_ptr<T> service, const std::string& name = "") {
        std::lock_guard<std::mutex> lock(getMutex());
        
        auto key = getKey<T>(name);
        getServices()[key] = service;
    }
    
    /**
     * @brief Ottiene un servizio registrato.
     * 
     * @tparam T Tipo del servizio
     * @param name Nome opzionale del servizio
     * @return Puntatore condiviso al servizio
     * @throws std::runtime_error Se il servizio non è registrato
     */
    template<typename T>
    static std::shared_ptr<T> getService(const std::string& name = "") {
        std::lock_guard<std::mutex> lock(getMutex());
        
        auto key = getKey<T>(name);
        auto& services = getServices();
        
        auto it = services.find(key);
        if (it != services.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        
        throw std::runtime_error("Service not registered: " + std::string(typeid(T).name()) + 
                                (name.empty() ? "" : " (name: " + name + ")"));
    }
    
    /**
     * @brief Tenta di ottenere un servizio registrato.
     * 
     * @tparam T Tipo del servizio
     * @param name Nome opzionale del servizio
     * @return Puntatore condiviso al servizio, o nullptr se non registrato
     */
    template<typename T>
    static std::shared_ptr<T> tryGetService(const std::string& name = "") {
        std::lock_guard<std::mutex> lock(getMutex());
        
        auto key = getKey<T>(name);
        auto& services = getServices();
        
        auto it = services.find(key);
        if (it != services.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        
        return nullptr;
    }
    
    /**
     * @brief Verifica se un servizio è registrato.
     * 
     * @tparam T Tipo del servizio
     * @param name Nome opzionale del servizio
     * @return true se il servizio è registrato, false altrimenti
     */
    template<typename T>
    static bool isServiceRegistered(const std::string& name = "") {
        std::lock_guard<std::mutex> lock(getMutex());
        
        auto key = getKey<T>(name);
        auto& services = getServices();
        
        return services.find(key) != services.end();
    }
    
    /**
     * @brief Rimuove un servizio registrato.
     * 
     * @tparam T Tipo del servizio
     * @param name Nome opzionale del servizio
     * @return true se il servizio è stato rimosso, false se non era registrato
     */
    template<typename T>
    static bool unregisterService(const std::string& name = "") {
        std::lock_guard<std::mutex> lock(getMutex());
        
        auto key = getKey<T>(name);
        auto& services = getServices();
        
        auto it = services.find(key);
        if (it != services.end()) {
            services.erase(it);
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief Rimuove tutti i servizi registrati.
     */
    static void reset() {
        std::lock_guard<std::mutex> lock(getMutex());
        getServices().clear();
    }

private:
    // Costruttore privato per evitare istanziazione
    ServiceLocator() = default;
    
    // Distruttore privato
    ~ServiceLocator() = default;
    
    /**
     * @brief Ottiene la chiave per un tipo di servizio e un nome opzionale.
     * 
     * @tparam T Tipo del servizio
     * @param name Nome opzionale del servizio
     * @return Chiave per il servizio
     */
    template<typename T>
    static std::string getKey(const std::string& name) {
        std::string key = typeid(T).name();
        if (!name.empty()) {
            key += ":" + name;
        }
        return key;
    }
    
    /**
     * @brief Ottiene il riferimento alla mappa dei servizi.
     * 
     * @return Riferimento alla mappa dei servizi
     */
    static std::unordered_map<std::string, std::shared_ptr<void>>& getServices() {
        static std::unordered_map<std::string, std::shared_ptr<void>> s_services;
        return s_services;
    }
    
    /**
     * @brief Ottiene il mutex per la sincronizzazione.
     * 
     * @return Riferimento al mutex
     */
    static std::mutex& getMutex() {
        static std::mutex s_mutex;
        return s_mutex;
    }
};

/**
 * @brief Interfaccia base per i servizi registrabili nel ServiceLocator.
 * 
 * Questa classe fornisce un'interfaccia comune per i servizi che possono
 * essere registrati nel ServiceLocator, facilitando la creazione di mock
 * per i test.
 */
class IService {
public:
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~IService() = default;
};

/**
 * @brief Helper per registrare servizi nel ServiceLocator.
 * 
 * Questa classe fornisce un meccanismo per registrare automaticamente
 * un servizio nel ServiceLocator alla sua creazione e rimuoverlo
 * alla sua distruzione.
 * 
 * @tparam T Tipo del servizio
 */
template<typename T>
class ServiceRegistration {
public:
    /**
     * @brief Costruttore.
     * 
     * @param service Puntatore condiviso al servizio
     * @param name Nome opzionale del servizio
     */
    ServiceRegistration(std::shared_ptr<T> service, const std::string& name = "")
        : m_service(service), m_name(name) {
        ServiceLocator::registerService<T>(m_service, m_name);
    }
    
    /**
     * @brief Distruttore.
     */
    ~ServiceRegistration() {
        ServiceLocator::unregisterService<T>(m_name);
    }

private:
    std::shared_ptr<T> m_service;
    std::string m_name;
};

/**
 * @brief Factory per creare e registrare servizi.
 * 
 * Questa classe fornisce un meccanismo per creare e registrare servizi
 * in modo centralizzato, consentendo di sostituire le implementazioni
 * con mock per i test.
 */
class ServiceFactory {
public:
    /**
     * @brief Crea e registra un servizio.
     * 
     * @tparam T Tipo del servizio
     * @tparam Args Tipi dei parametri per il costruttore
     * @param name Nome opzionale del servizio
     * @param args Parametri per il costruttore
     * @return Puntatore condiviso al servizio
     */
    template<typename T, typename... Args>
    static std::shared_ptr<T> createAndRegister(const std::string& name, Args&&... args) {
        auto service = std::make_shared<T>(std::forward<Args>(args)...);
        ServiceLocator::registerService<T>(service, name);
        return service;
    }
    
    /**
     * @brief Crea e registra un servizio con nome predefinito.
     * 
     * @tparam T Tipo del servizio
     * @tparam Args Tipi dei parametri per il costruttore
     * @param args Parametri per il costruttore
     * @return Puntatore condiviso al servizio
     */
    template<typename T, typename... Args>
    static std::shared_ptr<T> createAndRegister(Args&&... args) {
        return createAndRegister<T>("", std::forward<Args>(args)...);
    }
};

/**
 * @brief Provider di servizi per classi che dipendono da servizi.
 * 
 * Questa classe fornisce un meccanismo per accedere ai servizi registrati
 * nel ServiceLocator, semplificando la dependency injection.
 */
class ServiceProvider {
public:
    /**
     * @brief Ottiene un servizio registrato.
     * 
     * @tparam T Tipo del servizio
     * @param name Nome opzionale del servizio
     * @return Puntatore condiviso al servizio
     */
    template<typename T>
    static std::shared_ptr<T> get(const std::string& name = "") {
        return ServiceLocator::getService<T>(name);
    }
    
    /**
     * @brief Tenta di ottenere un servizio registrato.
     * 
     * @tparam T Tipo del servizio
     * @param name Nome opzionale del servizio
     * @return Puntatore condiviso al servizio, o nullptr se non registrato
     */
    template<typename T>
    static std::shared_ptr<T> tryGet(const std::string& name = "") {
        return ServiceLocator::tryGetService<T>(name);
    }
};

} // namespace Patterns
} // namespace Core 