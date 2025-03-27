#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>
#include <algorithm>

namespace Core {
namespace Patterns {

/**
 * @brief Implementazione del pattern Observer.
 * 
 * Questa classe implementa il pattern Observer in modo generico,
 * permettendo a oggetti di osservare ed essere notificati di cambiamenti in altri oggetti.
 * 
 * @tparam T Tipo di dato passato agli osservatori durante le notifiche
 */
template <typename T>
class Subject {
public:
    /**
     * @brief Costruttore.
     */
    Subject() : m_nextObserverId(0) {}
    
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~Subject() = default;
    
    /**
     * @brief Aggiunge un osservatore.
     * 
     * @param callback Funzione da chiamare quando i dati cambiano
     * @return ID dell'osservatore, necessario per rimuoverlo
     */
    int addObserver(std::function<void(const T&)> callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        int id = m_nextObserverId++;
        m_observers[id] = callback;
        return id;
    }
    
    /**
     * @brief Rimuove un osservatore.
     * 
     * @param observerId ID dell'osservatore da rimuovere
     * @return true se l'osservatore è stato rimosso, false se non esisteva
     */
    bool removeObserver(int observerId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_observers.find(observerId);
        if (it != m_observers.end()) {
            m_observers.erase(it);
            return true;
        }
        return false;
    }
    
    /**
     * @brief Notifica tutti gli osservatori di un cambiamento.
     * 
     * @param data Dati da passare agli osservatori
     */
    void notifyObservers(const T& data) {
        std::unordered_map<int, std::function<void(const T&)>> observersCopy;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            observersCopy = m_observers;
        }
        
        for (const auto& [id, callback] : observersCopy) {
            callback(data);
        }
    }
    
    /**
     * @brief Verifica se ci sono osservatori registrati.
     * 
     * @return true se ci sono osservatori, false altrimenti
     */
    bool hasObservers() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return !m_observers.empty();
    }
    
    /**
     * @brief Ottiene il numero di osservatori registrati.
     * 
     * @return Numero di osservatori
     */
    size_t observerCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_observers.size();
    }
    
    /**
     * @brief Rimuove tutti gli osservatori.
     */
    void clearObservers() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_observers.clear();
    }

private:
    mutable std::mutex m_mutex;
    int m_nextObserverId;
    std::unordered_map<int, std::function<void(const T&)>> m_observers;
};

/**
 * @brief Interfaccia per gli osservatori.
 * 
 * @tparam T Tipo di dato passato all'osservatore durante le notifiche
 */
template <typename T>
class IObserver {
public:
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~IObserver() = default;
    
    /**
     * @brief Metodo chiamato quando i dati osservati cambiano.
     * 
     * @param data Dati aggiornati
     */
    virtual void onUpdate(const T& data) = 0;
};

/**
 * @brief Implementazione del pattern Observer con una classe base per gli osservatori.
 * 
 * Questa classe implementa il pattern Observer usando una classe base per gli osservatori,
 * invece di funzioni di callback.
 * 
 * @tparam T Tipo di dato passato agli osservatori durante le notifiche
 */
template <typename T>
class ObservableSubject {
public:
    /**
     * @brief Costruttore.
     */
    ObservableSubject() {}
    
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~ObservableSubject() = default;
    
    /**
     * @brief Aggiunge un osservatore.
     * 
     * @param observer Puntatore all'osservatore da aggiungere
     */
    void addObserver(IObserver<T>* observer) {
        if (!observer) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        m_observers.push_back(observer);
    }
    
    /**
     * @brief Rimuove un osservatore.
     * 
     * @param observer Puntatore all'osservatore da rimuovere
     * @return true se l'osservatore è stato rimosso, false se non esisteva
     */
    bool removeObserver(IObserver<T>* observer) {
        if (!observer) return false;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find(m_observers.begin(), m_observers.end(), observer);
        if (it != m_observers.end()) {
            m_observers.erase(it);
            return true;
        }
        return false;
    }
    
    /**
     * @brief Notifica tutti gli osservatori di un cambiamento.
     * 
     * @param data Dati da passare agli osservatori
     */
    void notifyObservers(const T& data) {
        std::vector<IObserver<T>*> observersCopy;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            observersCopy = m_observers;
        }
        
        for (auto observer : observersCopy) {
            observer->onUpdate(data);
        }
    }
    
    /**
     * @brief Verifica se ci sono osservatori registrati.
     * 
     * @return true se ci sono osservatori, false altrimenti
     */
    bool hasObservers() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return !m_observers.empty();
    }
    
    /**
     * @brief Ottiene il numero di osservatori registrati.
     * 
     * @return Numero di osservatori
     */
    size_t observerCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_observers.size();
    }
    
    /**
     * @brief Rimuove tutti gli osservatori.
     */
    void clearObservers() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_observers.clear();
    }

private:
    mutable std::mutex m_mutex;
    std::vector<IObserver<T>*> m_observers;
};

/**
 * @brief Implementazione del pattern Observer con supporto per osservatori in thread diversi.
 * 
 * Questa classe implementa il pattern Observer con la possibilità di specificare
 * se un osservatore deve essere notificato in modo sincrono o asincrono.
 * 
 * @tparam T Tipo di dato passato agli osservatori durante le notifiche
 */
template <typename T>
class ThreadSafeSubject {
public:
    /**
     * @brief Costruttore.
     */
    ThreadSafeSubject() : m_nextObserverId(0) {}
    
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~ThreadSafeSubject() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& task : m_asyncTasks) {
            if (task.second.valid()) {
                task.second.wait();
            }
        }
    }
    
    /**
     * @brief Aggiunge un osservatore sincrono.
     * 
     * @param callback Funzione da chiamare quando i dati cambiano
     * @return ID dell'osservatore, necessario per rimuoverlo
     */
    int addObserver(std::function<void(const T&)> callback) {
        return addObserver(callback, false);
    }
    
    /**
     * @brief Aggiunge un osservatore.
     * 
     * @param callback Funzione da chiamare quando i dati cambiano
     * @param isAsync Se true, la callback sarà eseguita in un thread separato
     * @return ID dell'osservatore, necessario per rimuoverlo
     */
    int addObserver(std::function<void(const T&)> callback, bool isAsync) {
        std::lock_guard<std::mutex> lock(m_mutex);
        int id = m_nextObserverId++;
        m_observers[id] = std::make_pair(callback, isAsync);
        return id;
    }
    
    /**
     * @brief Rimuove un osservatore.
     * 
     * @param observerId ID dell'osservatore da rimuovere
     * @return true se l'osservatore è stato rimosso, false se non esisteva
     */
    bool removeObserver(int observerId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Se c'è un task asincrono in esecuzione per questo osservatore, attendiamo che finisca
        auto taskIt = m_asyncTasks.find(observerId);
        if (taskIt != m_asyncTasks.end() && taskIt->second.valid()) {
            taskIt->second.wait();
            m_asyncTasks.erase(taskIt);
        }
        
        auto it = m_observers.find(observerId);
        if (it != m_observers.end()) {
            m_observers.erase(it);
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief Notifica tutti gli osservatori di un cambiamento.
     * 
     * @param data Dati da passare agli osservatori
     */
    void notifyObservers(const T& data) {
        std::unordered_map<int, std::pair<std::function<void(const T&)>, bool>> observersCopy;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            observersCopy = m_observers;
            
            // Pulizia dei task completati
            for (auto it = m_asyncTasks.begin(); it != m_asyncTasks.end();) {
                if (it->second.valid() && 
                    it->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    it = m_asyncTasks.erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        for (const auto& [id, observer] : observersCopy) {
            const auto& [callback, isAsync] = observer;
            
            if (isAsync) {
                std::lock_guard<std::mutex> lock(m_mutex);
                // Crea una copia dei dati per il thread asincrono
                T dataCopy = data;
                auto future = std::async(std::launch::async, [callback, dataCopy]() {
                    callback(dataCopy);
                });
                m_asyncTasks[id] = std::move(future);
            } else {
                callback(data);
            }
        }
    }
    
    /**
     * @brief Attende il completamento di tutti i task asincroni.
     */
    void waitForAsyncTasks() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& task : m_asyncTasks) {
            if (task.second.valid()) {
                task.second.wait();
            }
        }
        m_asyncTasks.clear();
    }

private:
    mutable std::mutex m_mutex;
    int m_nextObserverId;
    std::unordered_map<int, std::pair<std::function<void(const T&)>, bool>> m_observers;
    std::unordered_map<int, std::future<void>> m_asyncTasks;
};

} // namespace Patterns
} // namespace Core 