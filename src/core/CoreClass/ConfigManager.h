#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

/**
 * @brief Classe per la gestione delle configurazioni dell'applicazione
 */
class ConfigManager {
public:
    using ConfigValue = std::variant<std::string, int, double, bool, std::vector<std::string>>;
    
    /**
     * @brief Costruttore del ConfigManager
     */
    ConfigManager();
    
    /**
     * @brief Distruttore del ConfigManager
     */
    ~ConfigManager();
    
    /**
     * @brief Carica una configurazione da file
     * @param filePath Percorso del file di configurazione
     * @return true se il caricamento è avvenuto con successo, false altrimenti
     */
    bool load(const std::string& filePath);
    
    /**
     * @brief Salva la configurazione corrente su file
     * @param filePath Percorso del file (se vuoto, usa l'ultimo file caricato)
     * @return true se il salvataggio è avvenuto con successo, false altrimenti
     */
    bool save(const std::string& filePath = "");
    
    /**
     * @brief Ottiene il valore di una chiave di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param defaultValue Valore di default se la chiave non esiste
     * @return Valore della configurazione
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue = T()) const;
    
    /**
     * @brief Verifica se una chiave di configurazione esiste
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @return true se la chiave esiste, false altrimenti
     */
    bool exists(const std::string& key) const;
    
    /**
     * @brief Imposta il valore di una chiave di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @param value Valore da impostare
     */
    template<typename T>
    void set(const std::string& key, const T& value);
    
    /**
     * @brief Rimuove una chiave di configurazione
     * @param key Chiave di configurazione (formato: "sezione.chiave")
     * @return true se la chiave è stata rimossa, false se non esisteva
     */
    bool remove(const std::string& key);
    
    /**
     * @brief Cancella tutte le configurazioni
     */
    void clear();
    
private:
    std::unordered_map<std::string, ConfigValue> m_config;
    std::string m_lastLoadedFile;
    
    /**
     * @brief Analizza una chiave nel formato "sezione.chiave"
     * @param key Chiave da analizzare
     * @param section Riferimento alla sezione estratta
     * @param name Riferimento al nome estratto
     * @return true se la chiave è valida, false altrimenti
     */
    bool parseKey(const std::string& key, std::string& section, std::string& name) const;
};
