# Guida all'Integrazione Python - CBA (Core Base Application)

## Panoramica

Il sistema CBA include un interprete Python completo integrato tramite pybind11, che consente l'esecuzione di script Python direttamente dall'applicazione C++ con pieno accesso alle funzionalità del core.

## Caratteristiche Principali

### 🐍 Interprete Python Embedded
- Interprete Python completo integrato nell'applicazione
- Gestione automatica dell'inizializzazione e finalizzazione
- Supporto per tutte le librerie Python standard
- Cattura di stdout/stderr per debugging

### 🔧 Gestione Avanzata delle Variabili
- Impostazione e lettura di variabili Python da C++
- Supporto per tipi: string, int, double, bool
- Persistenza delle variabili durante la sessione

### 📦 Gestione Moduli
- Importazione dinamica di moduli Python
- Supporto per alias dei moduli
- Tracciamento dei moduli caricati
- Gestione del Python path

### 🎯 Chiamata di Funzioni
- Esecuzione di funzioni Python globali
- Chiamata di funzioni specifiche di moduli
- Passaggio di argomenti multipli
- Cattura del valore di ritorno

### 💾 Gestione dello Stato
- Salvataggio e caricamento dello stato dell'interprete
- Persistenza delle variabili e moduli
- Reset completo dell'interprete

## Utilizzo tramite GUI

### Pannello "Informazioni Interprete"
- **Stato**: Mostra se l'interprete è inizializzato
- **Versione Python**: Versione dell'interprete in uso
- **Eseguibile**: Path dell'eseguibile Python
- **Pulsanti di controllo**:
  - `Inizializza Interprete`: Avvia l'interprete Python
  - `Reset Interprete`: Riavvia l'interprete pulendo lo stato
  - `Finalizza Interprete`: Chiude l'interprete

### Pannello "Esecuzione Script"
- **Editor di codice**: Area di testo per inserire script Python
- **Caricamento file**: Possibilità di caricare script da file
- **Modalità di esecuzione**:
  - `Esegui con Output`: Esegue lo script e cattura l'output
  - `Esegui Silenzioso`: Esegue lo script senza catturare l'output
- **Area output**: Visualizza i risultati dell'esecuzione

### Pannello "Gestione Variabili"
- **Impostazione variabili**: Nome, valore e tipo
- **Tipi supportati**: String, Int, Double, Bool
- **Lettura variabili**: Recupero del valore di variabili esistenti

### Pannello "Gestione Moduli"
- **Importazione moduli**: Nome del modulo e alias opzionale
- **Lista moduli caricati**: Visualizzazione dei moduli attualmente importati

### Pannello "Chiamata Funzioni"
- **Nome funzione**: Funzione da chiamare
- **Modulo**: Modulo contenente la funzione (opzionale)
- **Argomenti**: Lista di argomenti separati da virgola

### Pannello "Gestione Path Python"
- **Aggiunta path**: Aggiunge directory al sys.path di Python
- **Visualizzazione path**: Lista dei path attualmente configurati

### Pannello "Gestione Errori"
- **Ultimo errore**: Visualizza l'ultimo errore Python
- **Cancellazione errori**: Pulisce lo stato degli errori

### Pannello "Gestione Stato"
- **Salvataggio stato**: Salva variabili e moduli in un file
- **Caricamento stato**: Ripristina uno stato precedentemente salvato

## Utilizzo tramite API C++

### Inizializzazione
```cpp
// Inizializza l'interprete Python
bool success = coreAPI.initializePythonEngine();

// Verifica se è inizializzato
bool isInit = coreAPI.isPythonInitialized();

// Ottieni informazioni
std::string version = coreAPI.getPythonVersion();
std::string executable = coreAPI.getPythonExecutable();
```

### Esecuzione di Script
```cpp
// Esecuzione con cattura output
std::string result = coreAPI.executePythonString("print('Hello from Python!')");

// Esecuzione silenziosa
bool success = coreAPI.executePythonStringQuiet("x = 42");

// Esecuzione da file
std::string result = coreAPI.executePythonFile("script.py");
bool success = coreAPI.executePythonFileQuiet("script.py");
```

### Gestione Variabili
```cpp
// Impostazione variabili
coreAPI.setPythonVariable("name", "CBA System");
coreAPI.setPythonVariable("count", 42);
coreAPI.setPythonVariable("pi", 3.14159);
coreAPI.setPythonVariable("active", true);

// Lettura variabili
std::string value = coreAPI.getPythonVariable("name");
```

### Gestione Moduli
```cpp
// Importazione moduli
bool success = coreAPI.importPythonModule("math");
bool success = coreAPI.importPythonModuleAs("numpy", "np");

// Lista moduli caricati
std::vector<std::string> modules = coreAPI.getLoadedPythonModules();

// Gestione path
coreAPI.addToPythonPath("/custom/python/modules");
std::vector<std::string> paths = coreAPI.getPythonPath();
```

### Chiamata Funzioni
```cpp
// Funzione globale
std::vector<std::string> args = {"arg1", "arg2"};
std::string result = coreAPI.callPythonFunction("my_function", args);

// Funzione di modulo
std::string result = coreAPI.callPythonModuleFunction("math", "sqrt", {"16"});
```

### Gestione Errori
```cpp
// Ottieni ultimo errore
std::string error = coreAPI.getPythonLastError();

// Cancella errori
coreAPI.clearPythonError();
```

### Gestione dello Stato
```cpp
// Salva stato
bool success = coreAPI.savePythonState("session.pkl");

// Carica stato
bool success = coreAPI.loadPythonState("session.pkl");

// Reset completo
bool success = coreAPI.resetPythonEngine();
```

## Esempi Pratici

### Esempio 1: Calcoli Matematici
```python
# Script Python
import math

def calculate_area(radius):
    return math.pi * radius ** 2

radius = 5
area = calculate_area(radius)
print(f"Area del cerchio: {area}")
```

### Esempio 2: Elaborazione Dati
```python
# Script Python
data = [1, 2, 3, 4, 5]
processed = [x * 2 for x in data]
print(f"Dati elaborati: {processed}")

# Salva risultato in variabile
result = sum(processed)
```

### Esempio 3: Integrazione con Librerie Esterne
```python
# Script Python (richiede numpy)
import numpy as np

# Crea array
arr = np.array([1, 2, 3, 4, 5])
mean_val = np.mean(arr)
std_val = np.std(arr)

print(f"Media: {mean_val}, Deviazione standard: {std_val}")
```

## Best Practices

### 🔒 Sicurezza
- Validare sempre l'input degli script Python
- Limitare l'accesso a funzioni sensibili del sistema
- Utilizzare sandbox per script non fidati

### ⚡ Performance
- Inizializzare l'interprete una sola volta
- Riutilizzare variabili e moduli quando possibile
- Utilizzare l'esecuzione silenziosa per script senza output

### 🐛 Debugging
- Controllare sempre il valore di ritorno delle operazioni
- Utilizzare la gestione errori per diagnosticare problemi
- Salvare lo stato per debugging di sessioni complesse

### 📁 Organizzazione
- Organizzare script Python in directory dedicate
- Utilizzare moduli per funzionalità riutilizzabili
- Documentare script complessi

## Risoluzione Problemi

### Problemi Comuni

**Interprete non si inizializza**
- Verificare che Python sia installato correttamente
- Controllare le variabili d'ambiente PATH
- Verificare la compatibilità della versione Python

**Moduli non trovati**
- Aggiungere il path del modulo con `addToPythonPath()`
- Verificare che il modulo sia installato
- Controllare la sintassi del nome del modulo

**Errori di esecuzione**
- Controllare la sintassi Python
- Verificare che le variabili siano definite
- Utilizzare la gestione errori per dettagli

**Performance lente**
- Evitare di reinizializzare l'interprete frequentemente
- Utilizzare l'esecuzione silenziosa quando appropriato
- Ottimizzare gli script Python

## Limitazioni Attuali

- L'interprete è single-threaded
- Alcune librerie Python potrebbero non essere compatibili
- La gestione dello stato è limitata a variabili semplici
- Non supporta callback da Python a C++ (in sviluppo)

## Sviluppi Futuri

- Supporto per callback bidirezionali
- Integrazione con debugger Python
- Supporto per threading
- Plugin system basato su Python
- Interfaccia REST per controllo remoto

---

**Versione Documento**: 1.0  
**Ultimo Aggiornamento**: 2024  
**Autore**: CBA Development Team