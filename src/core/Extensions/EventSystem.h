#pragma once

#include <unordered_map>
#include <functional>
#include <typeindex>
#include <vector>
#include <mutex>
#include <memory>
#include <any>
#include <string>

namespace Core {
namespace Extensions {

/**
 * @brief Sistema di eventi per la comunicazione inter-modulo.
 * 
 * Questa classe implementa un sistema di eventi basato sul pattern Pub/Sub
 * che permette la comunicazione tra moduli in modo disaccoppiato.
 */
class EventSystem {
public:
    /**
     * @brief Ottiene l'istanza singleton del sistema di eventi.
     * @return Riferimento all'istanza singleton
     */
    static EventSystem& getInstance() {
        static EventSystem instance;
        return instance;
    }

    /**
     * @brief Sottoscrive una callback a un tipo di evento.
     * 
     * @tparam EventType Tipo dell'evento
     * @param callback Funzione da chiamare quando l'evento viene pubblicato
     * @return ID della sottoscrizione, necessario per annullare la sottoscrizione
     */
    template<typename EventType>
    int subscribe(std::function<void(const EventType&)> callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto typeIndex = std::type_index(typeid(EventType));
        auto& subscribers = m_subscribers[typeIndex];
        
        int id = m_nextSubscriberId++;
        subscribers[id] = [callback](std::any event) {
            callback(std::any_cast<const EventType&>(event));
        };
        
        return id;
    }

    /**
     * @brief Annulla la sottoscrizione di una callback.
     * 
     * @tparam EventType Tipo dell'evento
     * @param subscriptionId ID della sottoscrizione da annullare
     * @return true se la sottoscrizione è stata annullata, false se non esisteva
     */
    template<typename EventType>
    bool unsubscribe(int subscriptionId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto typeIndex = std::type_index(typeid(EventType));
        auto it = m_subscribers.find(typeIndex);
        
        if (it != m_subscribers.end()) {
            auto& subscribers = it->second;
            auto subIt = subscribers.find(subscriptionId);
            
            if (subIt != subscribers.end()) {
                subscribers.erase(subIt);
                return true;
            }
        }
        
        return false;
    }

    /**
     * @brief Pubblica un evento a tutti i sottoscrittori.
     * 
     * @tparam EventType Tipo dell'evento
     * @param event Istanza dell'evento da pubblicare
     */
    template<typename EventType>
    void publish(const EventType& event) {
        std::unordered_map<int, std::function<void(std::any)>> subscribersCopy;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto typeIndex = std::type_index(typeid(EventType));
            auto it = m_subscribers.find(typeIndex);
            
            if (it != m_subscribers.end()) {
                subscribersCopy = it->second;
            }
        }
        
        for (const auto& [id, callback] : subscribersCopy) {
            callback(event);
        }
    }

private:
    // Costruttore privato per pattern Singleton
    EventSystem() : m_nextSubscriberId(0) {}
    
    // Distruttore privato
    ~EventSystem() = default;
    
    // Disabilita copia e assegnazione
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;

    std::mutex m_mutex;
    int m_nextSubscriberId;
    std::unordered_map<std::type_index, 
                      std::unordered_map<int, std::function<void(std::any)>>> m_subscribers;
};

/**
 * @brief Classe base per tutti gli eventi di sistema.
 */
struct BaseEvent {
    virtual ~BaseEvent() = default;
};

/**
 * @brief Evento emesso quando un modulo viene caricato.
 */
struct ModuleLoadedEvent : public BaseEvent {
    std::string moduleName;
    std::string version;
    
    ModuleLoadedEvent(const std::string& name, const std::string& ver)
        : moduleName(name), version(ver) {}
};

/**
 * @brief Evento emesso quando un modulo viene scaricato.
 */
struct ModuleUnloadedEvent : public BaseEvent {
    std::string moduleName;
    
    ModuleUnloadedEvent(const std::string& name) : moduleName(name) {}
};

/**
 * @brief Evento emesso quando la configurazione viene modificata.
 */
struct ConfigChangedEvent : public BaseEvent {
    std::string key;
    std::any newValue;
    std::any oldValue;
    
    ConfigChangedEvent(const std::string& k, std::any nv, std::any ov)
        : key(k), newValue(nv), oldValue(ov) {}
};

/**
 * @brief Evento emesso quando un errore critico si verifica.
 */
struct ErrorEvent : public BaseEvent {
    std::string message;
    std::string source;
    int errorCode;
    
    ErrorEvent(const std::string& msg, const std::string& src, int code)
        : message(msg), source(src), errorCode(code) {}
};

} // namespace Extensions
} // namespace Core 