#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <optional>
#include <chrono>

namespace Core {
namespace Patterns {

/**
 * @brief Interfaccia base per tutti i comandi.
 * 
 * Questa classe definisce l'interfaccia che tutti i comandi devono implementare.
 * Il pattern Command permette di incapsulare una richiesta come oggetto,
 * consentendo di parametrizzare client con diverse richieste, accodare o loggare le richieste,
 * e supportare operazioni annullabili.
 */
class ICommand {
public:
    /**
     * @brief Distruttore virtuale.
     */
    virtual ~ICommand() = default;
    
    /**
     * @brief Esegue il comando.
     * 
     * @return true se l'esecuzione è avvenuta con successo, false altrimenti
     */
    virtual bool execute() = 0;
    
    /**
     * @brief Annulla il comando, ripristinando lo stato precedente.
     * 
     * @return true se l'annullamento è avvenuto con successo, false altrimenti
     */
    virtual bool undo() = 0;
    
    /**
     * @brief Ottiene il nome del comando.
     * 
     * @return Nome del comando
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Verifica se il comando è annullabile.
     * 
     * @return true se il comando è annullabile, false altrimenti
     */
    virtual bool isUndoable() const = 0;
    
    /**
     * @brief Ottiene una descrizione del comando.
     * 
     * @return Descrizione del comando
     */
    virtual std::string getDescription() const = 0;
    
    /**
     * @brief Ottiene il timestamp di esecuzione del comando.
     * 
     * @return Timestamp di esecuzione, o nullopt se il comando non è ancora stato eseguito
     */
    virtual std::optional<std::chrono::system_clock::time_point> getExecutionTimestamp() const = 0;
};

/**
 * @brief Implementazione di base per i comandi.
 * 
 * Questa classe fornisce un'implementazione di base per i comandi.
 */
class BaseCommand : public ICommand {
public:
    /**
     * @brief Costruttore.
     * 
     * @param name Nome del comando
     * @param description Descrizione del comando
     * @param isUndoable Se il comando è annullabile
     */
    BaseCommand(const std::string& name, const std::string& description, bool isUndoable = true)
        : m_name(name), m_description(description), m_isUndoable(isUndoable) {}
    
    /**
     * @brief Ottiene il nome del comando.
     * 
     * @return Nome del comando
     */
    std::string getName() const override {
        return m_name;
    }
    
    /**
     * @brief Verifica se il comando è annullabile.
     * 
     * @return true se il comando è annullabile, false altrimenti
     */
    bool isUndoable() const override {
        return m_isUndoable;
    }
    
    /**
     * @brief Ottiene una descrizione del comando.
     * 
     * @return Descrizione del comando
     */
    std::string getDescription() const override {
        return m_description;
    }
    
    /**
     * @brief Ottiene il timestamp di esecuzione del comando.
     * 
     * @return Timestamp di esecuzione, o nullopt se il comando non è ancora stato eseguito
     */
    std::optional<std::chrono::system_clock::time_point> getExecutionTimestamp() const override {
        return m_executionTimestamp;
    }

protected:
    std::string m_name;
    std::string m_description;
    bool m_isUndoable;
    std::optional<std::chrono::system_clock::time_point> m_executionTimestamp;
};

/**
 * @brief Comando che esegue una funzione lambda.
 * 
 * Questa classe permette di creare comandi a partire da funzioni lambda,
 * senza dover definire una nuova classe per ogni tipo di comando.
 */
class LambdaCommand : public BaseCommand {
public:
    /**
     * @brief Costruttore.
     * 
     * @param name Nome del comando
     * @param description Descrizione del comando
     * @param executeFunc Funzione da eseguire
     * @param undoFunc Funzione per annullare l'esecuzione
     */
    LambdaCommand(const std::string& name, 
                 const std::string& description,
                 std::function<bool()> executeFunc,
                 std::function<bool()> undoFunc = nullptr)
        : BaseCommand(name, description, undoFunc != nullptr),
          m_executeFunc(executeFunc),
          m_undoFunc(undoFunc) {}
    
    /**
     * @brief Esegue il comando.
     * 
     * @return true se l'esecuzione è avvenuta con successo, false altrimenti
     */
    bool execute() override {
        if (m_executeFunc) {
            bool result = m_executeFunc();
            if (result) {
                m_executionTimestamp = std::chrono::system_clock::now();
            }
            return result;
        }
        return false;
    }
    
    /**
     * @brief Annulla il comando, ripristinando lo stato precedente.
     * 
     * @return true se l'annullamento è avvenuto con successo, false altrimenti
     */
    bool undo() override {
        if (m_undoFunc && m_executionTimestamp) {
            return m_undoFunc();
        }
        return false;
    }

private:
    std::function<bool()> m_executeFunc;
    std::function<bool()> m_undoFunc;
};

/**
 * @brief Gestore dei comandi con supporto per undo/redo.
 * 
 * Questa classe gestisce l'esecuzione, l'annullamento e il ripristino di comandi,
 * mantenendo una cronologia delle operazioni.
 */
class CommandManager {
public:
    /**
     * @brief Costruttore.
     * 
     * @param maxHistory Numero massimo di comandi da mantenere nella cronologia
     */
    CommandManager(size_t maxHistory = 100)
        : m_maxHistory(maxHistory), m_currentIndex(0) {}
    
    /**
     * @brief Esegue un comando e lo aggiunge alla cronologia.
     * 
     * @param command Comando da eseguire
     * @return true se l'esecuzione è avvenuta con successo, false altrimenti
     */
    bool executeCommand(std::unique_ptr<ICommand> command) {
        if (!command) return false;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Se abbiamo comandi annullati nella cronologia, li rimuoviamo
        if (m_currentIndex < m_commandHistory.size()) {
            m_commandHistory.resize(m_currentIndex);
        }
        
        // Eseguiamo il comando
        bool result = command->execute();
        
        if (result) {
            // Aggiungiamo il comando alla cronologia
            m_commandHistory.push_back(std::move(command));
            m_currentIndex = m_commandHistory.size();
            
            // Se la cronologia supera la dimensione massima, rimuoviamo i comandi più vecchi
            if (m_commandHistory.size() > m_maxHistory) {
                m_commandHistory.erase(m_commandHistory.begin());
                m_currentIndex--;
            }
        }
        
        return result;
    }
    
    /**
     * @brief Annulla l'ultimo comando eseguito.
     * 
     * @return true se l'annullamento è avvenuto con successo, false altrimenti
     */
    bool undoLastCommand() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_currentIndex > 0 && !m_commandHistory.empty()) {
            m_currentIndex--;
            
            auto& command = m_commandHistory[m_currentIndex];
            if (command->isUndoable()) {
                return command->undo();
            }
        }
        
        return false;
    }
    
    /**
     * @brief Ripristina l'ultimo comando annullato.
     * 
     * @return true se il ripristino è avvenuto con successo, false altrimenti
     */
    bool redoCommand() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_currentIndex < m_commandHistory.size()) {
            auto& command = m_commandHistory[m_currentIndex];
            bool result = command->execute();
            
            if (result) {
                m_currentIndex++;
            }
            
            return result;
        }
        
        return false;
    }
    
    /**
     * @brief Verifica se è possibile annullare un comando.
     * 
     * @return true se è possibile annullare un comando, false altrimenti
     */
    bool canUndo() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_currentIndex > 0 && !m_commandHistory.empty();
    }
    
    /**
     * @brief Verifica se è possibile ripristinare un comando.
     * 
     * @return true se è possibile ripristinare un comando, false altrimenti
     */
    bool canRedo() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_currentIndex < m_commandHistory.size();
    }
    
    /**
     * @brief Ottiene l'indice del comando corrente.
     * 
     * @return Indice del comando corrente
     */
    size_t getCurrentIndex() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_currentIndex;
    }
    
    /**
     * @brief Ottiene il numero di comandi nella cronologia.
     * 
     * @return Numero di comandi nella cronologia
     */
    size_t getHistorySize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_commandHistory.size();
    }
    
    /**
     * @brief Ottiene la cronologia dei comandi.
     * 
     * @return Vettore contenente i nomi dei comandi nella cronologia
     */
    std::vector<std::string> getCommandHistory() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::string> history;
        for (const auto& command : m_commandHistory) {
            history.push_back(command->getName());
        }
        
        return history;
    }
    
    /**
     * @brief Svuota la cronologia dei comandi.
     */
    void clearHistory() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_commandHistory.clear();
        m_currentIndex = 0;
    }

private:
    size_t m_maxHistory;
    size_t m_currentIndex;
    std::vector<std::unique_ptr<ICommand>> m_commandHistory;
    mutable std::mutex m_mutex;
};

/**
 * @brief Comando che raggruppa più comandi in un'unica operazione.
 * 
 * Questa classe permette di eseguire più comandi come se fossero un unico comando,
 * con supporto per annullamento e ripristino atomici.
 */
class MacroCommand : public BaseCommand {
public:
    /**
     * @brief Costruttore.
     * 
     * @param name Nome del comando
     * @param description Descrizione del comando
     */
    MacroCommand(const std::string& name, const std::string& description)
        : BaseCommand(name, description, true) {}
    
    /**
     * @brief Aggiunge un comando al macro comando.
     * 
     * @param command Comando da aggiungere
     */
    void addCommand(std::unique_ptr<ICommand> command) {
        if (command) {
            m_commands.push_back(std::move(command));
        }
    }
    
    /**
     * @brief Esegue tutti i comandi nel macro comando.
     * 
     * @return true se tutti i comandi sono stati eseguiti con successo, false altrimenti
     */
    bool execute() override {
        bool allSuccess = true;
        
        for (auto& command : m_commands) {
            if (!command->execute()) {
                allSuccess = false;
                break;
            }
        }
        
        if (allSuccess) {
            m_executionTimestamp = std::chrono::system_clock::now();
        } else {
            // Se un comando fallisce, annulliamo quelli già eseguiti
            for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
                if ((*it)->getExecutionTimestamp()) {
                    (*it)->undo();
                }
            }
        }
        
        return allSuccess;
    }
    
    /**
     * @brief Annulla tutti i comandi nel macro comando in ordine inverso.
     * 
     * @return true se tutti i comandi sono stati annullati con successo, false altrimenti
     */
    bool undo() override {
        if (!m_executionTimestamp) return false;
        
        bool allSuccess = true;
        
        for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
            if (!(*it)->undo()) {
                allSuccess = false;
                break;
            }
        }
        
        if (allSuccess) {
            m_executionTimestamp.reset();
        }
        
        return allSuccess;
    }
    
    /**
     * @brief Verifica se il macro comando è annullabile.
     * 
     * @return true se tutti i comandi sono annullabili, false altrimenti
     */
    bool isUndoable() const override {
        for (const auto& command : m_commands) {
            if (!command->isUndoable()) {
                return false;
            }
        }
        
        return !m_commands.empty();
    }
    
    /**
     * @brief Ottiene il numero di comandi nel macro comando.
     * 
     * @return Numero di comandi
     */
    size_t commandCount() const {
        return m_commands.size();
    }

private:
    std::vector<std::unique_ptr<ICommand>> m_commands;
};

} // namespace Patterns
} // namespace Core 