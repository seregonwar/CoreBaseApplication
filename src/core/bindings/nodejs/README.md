# CoreBase Node.js Bindings

Node.js bindings per la libreria CoreBase, che fornisce accesso alle funzionalità di sistema, gestione della configurazione, monitoraggio e networking attraverso un'interfaccia JavaScript/TypeScript.

## Caratteristiche

- **ErrorHandler**: Gestione centralizzata degli errori e logging
- **ConfigManager**: Gestione della configurazione con supporto JSON
- **SystemMonitor**: Monitoraggio delle risorse di sistema (CPU, memoria, disco, rete)
- **NetworkManager**: Gestione delle connessioni di rete (HTTP, WebSocket, TCP/UDP)
- **Cross-platform**: Supporto per Windows, macOS, Linux, iOS e Android
- **TypeScript**: Definizioni di tipo complete per un'esperienza di sviluppo ottimale

## Installazione

### Prerequisiti

- Node.js 16.x o superiore
- npm o yarn
- Python 3.x (per node-gyp)
- Compilatore C++ (Visual Studio su Windows, Xcode su macOS, GCC su Linux)
- CMake 3.15 o superiore

### Installazione delle dipendenze

```bash
npm install
```

### Compilazione del modulo nativo

```bash
npm run build
```

## Utilizzo

### Importazione

```typescript
import { 
  CoreBaseManager, 
  createCoreBase, 
  getCoreBase,
  initializeCoreBase,
  shutdownCoreBase 
} from '@corebase/nodejs';

// Oppure importazione CommonJS
const { CoreBaseManager } = require('@corebase/nodejs');
```

### Utilizzo di base

```typescript
import { createCoreBase } from '@corebase/nodejs';

// Crea una nuova istanza
const corebase = createCoreBase();

// Inizializza il sistema
if (corebase.initialize()) {
  console.log('CoreBase inizializzato con successo');
  
  // Utilizza i componenti
  const platformInfo = corebase.getPlatformInfo();
  console.log('Piattaforma:', platformInfo.platform);
  
  // Gestione degli errori
  corebase.errorHandler.logInfo('Applicazione avviata');
  
  // Configurazione
  corebase.configManager.setValue('app.name', 'MyApp');
  const appName = corebase.configManager.getValue('app.name');
  
  // Monitoraggio del sistema
  corebase.systemMonitor.startMonitoring();
  const cpuInfo = corebase.systemMonitor.getCpuInfo();
  console.log('Utilizzo CPU:', cpuInfo.usage + '%');
  
  // Networking
  const response = corebase.networkManager.httpGet('https://api.example.com/data');
  if (response.success) {
    console.log('Dati ricevuti:', response.body);
  }
  
  // Cleanup
  corebase.shutdown();
} else {
  console.error('Errore durante l\'inizializzazione di CoreBase');
}
```

### Utilizzo del singleton

```typescript
import { initializeCoreBase, getCoreBase, shutdownCoreBase } from '@corebase/nodejs';

// Inizializza il singleton
if (initializeCoreBase()) {
  // Ottieni l'istanza singleton
  const corebase = getCoreBase();
  
  // Utilizza i componenti
  corebase.errorHandler.logInfo('Utilizzo del singleton');
  
  // Cleanup automatico alla fine dell'applicazione
  process.on('exit', () => {
    shutdownCoreBase();
  });
}
```

## API Reference

### CoreBaseManager

#### Metodi principali

- `initialize(): boolean` - Inizializza il sistema CoreBase
- `shutdown(): void` - Arresta il sistema CoreBase
- `isInitialized(): boolean` - Verifica se il sistema è inizializzato
- `getPlatformInfo(): PlatformInfo` - Ottiene informazioni sulla piattaforma
- `getVersion(): string` - Ottiene la versione di CoreBase

#### Componenti

- `errorHandler: ErrorHandler` - Gestione degli errori
- `configManager: ConfigManager` - Gestione della configurazione
- `systemMonitor: SystemMonitor` - Monitoraggio del sistema
- `networkManager: NetworkManager` - Gestione della rete

### ErrorHandler

```typescript
// Configurazione del livello di log
corebase.errorHandler.setLogLevel(corebase.errorHandler.getLogLevels().INFO);

// Logging
corebase.errorHandler.logError('Messaggio di errore');
corebase.errorHandler.logWarning('Messaggio di avviso');
corebase.errorHandler.logInfo('Messaggio informativo');
corebase.errorHandler.logDebug('Messaggio di debug');

// Gestione errori
corebase.errorHandler.handleError('Errore critico');
corebase.errorHandler.handleException('Eccezione non gestita');
```

### ConfigManager

```typescript
// Caricamento e salvataggio
corebase.configManager.loadConfig('./config.json');
corebase.configManager.saveConfig('./config.json');

// Gestione valori
corebase.configManager.setValue('database.host', 'localhost');
const host = corebase.configManager.getValue('database.host');

// Verifica esistenza
if (corebase.configManager.hasKey('database.port')) {
  const port = corebase.configManager.getValue('database.port');
}

// Rimozione chiavi
corebase.configManager.removeKey('temp.setting');

// Ottenere tutte le chiavi
const allKeys = corebase.configManager.getAllKeys();

// Validazione
if (corebase.configManager.validateConfig()) {
  console.log('Configurazione valida');
}
```

### SystemMonitor

```typescript
// Avvio monitoraggio
corebase.systemMonitor.startMonitoring();

// Informazioni CPU
const cpuInfo = corebase.systemMonitor.getCpuInfo();
console.log(`CPU: ${cpuInfo.usage}% (${cpuInfo.cores} cores)`);

// Informazioni memoria
const memInfo = corebase.systemMonitor.getMemoryInfo();
console.log(`Memoria: ${memInfo.usagePercentage}%`);

// Informazioni disco
const diskInfo = corebase.systemMonitor.getDiskInfo();
diskInfo.forEach(disk => {
  console.log(`Disco ${disk.diskName}: ${disk.usagePercentage}%`);
});

// Informazioni rete
const netInfo = corebase.systemMonitor.getNetworkInfo();
netInfo.forEach(iface => {
  console.log(`${iface.interfaceName}: ${iface.bytesReceived} bytes ricevuti`);
});

// Configurazione intervallo
corebase.systemMonitor.setMonitoringInterval(5000); // 5 secondi

// Soglie di allarme
corebase.systemMonitor.setThresholds({
  'cpu.usage': 80,
  'memory.usage': 90,
  'disk.usage': 95
});

// Ottenere allarmi
const alerts = corebase.systemMonitor.getAlerts();
alerts.forEach(alert => {
  console.log(`Allarme: ${alert.message}`);
});
```

### NetworkManager

```typescript
// Richieste HTTP
const response = corebase.networkManager.httpGet('https://api.example.com/users');
if (response.success) {
  const users = JSON.parse(response.body);
  console.log('Utenti:', users);
}

// POST con dati
const postResponse = corebase.networkManager.httpPost(
  'https://api.example.com/users',
  JSON.stringify({ name: 'John', email: 'john@example.com' }),
  { 'Content-Type': 'application/json' }
);

// WebSocket
const socketId = corebase.networkManager.createWebSocket('ws://localhost:8080');
if (corebase.networkManager.connectWebSocket(socketId)) {
  corebase.networkManager.sendWebSocketMessage(socketId, 'Hello WebSocket!');
}

// TCP Server
const serverId = corebase.networkManager.createTcpServer(3000);
console.log('Server TCP creato con ID:', serverId);

// Configurazione
corebase.networkManager.setTimeout(10000); // 10 secondi
corebase.networkManager.setUserAgent('MyApp/1.0');

// Statistiche
const stats = corebase.networkManager.getConnectionStats();
console.log(`Connessioni attive: ${stats.activeConnections}`);
```

## Sviluppo

### Script disponibili

- `npm run build` - Compila il modulo nativo e TypeScript
- `npm run build:native` - Compila solo il modulo nativo
- `npm run build:ts` - Compila solo TypeScript
- `npm test` - Esegue i test
- `npm run test:watch` - Esegue i test in modalità watch
- `npm run test:coverage` - Esegue i test con coverage
- `npm run lint` - Esegue ESLint
- `npm run lint:fix` - Corregge automaticamente i problemi di linting
- `npm run format` - Formatta il codice con Prettier
- `npm run clean` - Pulisce i file di build

### Struttura del progetto

```
src/
├── native/           # Codice C++ per il modulo nativo
│   ├── corebase_addon.cpp
│   ├── error_handler_wrapper.h/cpp
│   ├── config_manager_wrapper.h/cpp
│   ├── system_monitor_wrapper.h/cpp
│   └── network_manager_wrapper.h/cpp
├── types/            # Definizioni TypeScript
│   └── index.ts
└── index.ts          # Entry point principale

tests/                # Test Jest
├── corebase.test.ts
├── setup.ts
├── globalSetup.ts
└── globalTeardown.ts

build/                # File compilati
└── Release/
    └── corebase_addon.node
```

### Compilazione cross-platform

Il progetto supporta la compilazione per diverse piattaforme:

```bash
# Windows
npm run build -- --target_platform=windows

# macOS
npm run build -- --target_platform=macos

# Linux
npm run build -- --target_platform=linux

# iOS (richiede Xcode)
npm run build -- --target_platform=ios

# Android (richiede NDK)
npm run build -- --target_platform=android
```

## Test

### Esecuzione dei test

```bash
# Test completi
npm test

# Test con coverage
npm run test:coverage

# Test in modalità watch
npm run test:watch

# Test specifici
npm test -- --testNamePattern="ErrorHandler"
```

### Struttura dei test

I test sono organizzati per componente e coprono:

- Inizializzazione e shutdown
- Funzionalità di base di ogni componente
- Gestione degli errori
- Integrazione tra componenti
- Compatibilità cross-platform

## Troubleshooting

### Errori di compilazione

1. **node-gyp non trovato**:
   ```bash
   npm install -g node-gyp
   ```

2. **Errori di compilazione C++**:
   - Verificare che il compilatore C++ sia installato
   - Su Windows: installare Visual Studio Build Tools
   - Su macOS: installare Xcode Command Line Tools
   - Su Linux: installare build-essential

3. **CMake non trovato**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install cmake
   
   # macOS
   brew install cmake
   
   # Windows
   # Scaricare da https://cmake.org/download/
   ```

### Errori di runtime

1. **Modulo nativo non trovato**:
   - Verificare che `npm run build` sia stato eseguito
   - Controllare che il file `build/Release/corebase_addon.node` esista

2. **Errori di inizializzazione**:
   - Verificare i permessi del sistema
   - Controllare i log per dettagli specifici

## Licenza

Questo progetto è rilasciato sotto licenza MIT. Vedere il file LICENSE per i dettagli.

## Contributi

I contributi sono benvenuti! Per favore:

1. Fork del repository
2. Crea un branch per la tua feature
3. Aggiungi test per le nuove funzionalità
4. Assicurati che tutti i test passino
5. Invia una pull request

## Supporto

Per supporto e domande:

- Apri un issue su GitHub
- Consulta la documentazione dell'API
- Controlla gli esempi nella directory `examples/`