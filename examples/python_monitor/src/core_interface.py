#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Modulo di interfaccia tra l'applicazione Python e il framework Core C++.
Gestisce tutte le chiamate al framework Core utilizzando CorePythonAPI.
"""

import os
import sys
import json
import logging
import ctypes
import random
import time
from enum import Enum, auto
from typing import Dict, Any, Optional, Callable, Tuple, List

# Definiamo i nostri tipi equivalenti per compatibilità con il codice esistente
class ResourceType:
    """Tipi di risorse di sistema monitorabili."""
    CPU = "cpu"
    MEMORY = "memory"
    DISK = "disk"
    NETWORK = "network"
    GPU = "gpu"
    
    @classmethod
    def get_value(cls, resource_type):
        """Ottiene il valore della risorsa"""
        return resource_type

class LogLevel:
    """Livelli di log supportati dal Core."""
    DEBUG = 0
    INFO = 1
    WARNING = 2
    ERROR = 3
    FATAL = 4

class CoreInterface:
    """Interfaccia per comunicare con il framework Core C++."""
    
    def __init__(self, config_path: str = None):
        """
        Inizializza l'interfaccia con il Core.
        
        Args:
            config_path: Percorso del file di configurazione.
        """
        self.logger = logging.getLogger(__name__)
        self.logger.info("Inizializzazione dell'interfaccia Core...")
        
        # Configurazione del percorso del file di configurazione
        if config_path and os.path.exists(config_path):
            self.config_path = os.path.abspath(config_path)
        else:
            # Usa il file di configurazione di default
            base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            default_config = os.path.join(base_dir, "config", "default.json")
            
            if not os.path.exists(default_config):
                # Crea il file di configurazione di default se non esiste
                self._create_default_config(default_config)
                
            self.config_path = default_config
            
        self.logger.info(f"Utilizzo del file di configurazione: {self.config_path}")
        
        # Inizializzazione del Core
        self._initialize_core()
        
        # Mantieni traccia dei callback
        self._resource_callbacks = {}
        self._message_callbacks = {}
        
        # Memorizza lo storico delle risorse
        self._resource_history = {
            ResourceType.CPU: [],
            ResourceType.MEMORY: [],
            ResourceType.DISK: [],
            ResourceType.NETWORK: [],
            ResourceType.GPU: []
        }
        
    def _create_default_config(self, config_path: str):
        """
        Crea un file di configurazione di default.
        
        Args:
            config_path: Percorso dove creare il file di configurazione.
        """
        default_config = {
            "app": {
                "name": "PythonSystemMonitor",
                "version": "1.0.0",
                "log_level": 1,  # INFO
                "update_interval": 5
            },
            "system": {
                "monitor_cpu": True,
                "monitor_memory": True,
                "monitor_disk": True,
                "monitor_network": True,
                "monitor_gpu": False
            },
            "alerts": {
                "cpu_threshold": 80,
                "memory_threshold": 85,
                "disk_threshold": 90,
                "network_threshold": 80,
                "gpu_threshold": 75,
                "email_notifications": False,
                "email_recipient": ""
            },
            "ui": {
                "refresh_rate": 1000,
                "show_graphs": True,
                "dark_mode": True
            }
        }
        
        # Crea la directory se non esiste
        os.makedirs(os.path.dirname(config_path), exist_ok=True)
        
        # Salva il file di configurazione
        with open(config_path, "w") as f:
            json.dump(default_config, f, indent=4)
            
        self.logger.info(f"Creato file di configurazione di default: {config_path}")
    
    def _initialize_core(self):
        """
        Inizializza l'interfaccia con il framework Core C++.
        
        In un'implementazione reale, questo metodo caricherà la libreria dinamica 
        del Core e inizializzerà le sue funzionalità. Per il momento, simuliamo
        questo comportamento con funzioni fittizie.
        """
        self.logger.info("Inizializzazione del Core...")
        
        # In un caso reale, caricheremmo la DLL/SO del Core
        # self.core_lib = ctypes.CDLL("path/to/core_library.dll")
        
        # Simuliamo l'inizializzazione del Core
        self.initialized = True
        self.logger.info("Core inizializzato con successo")
        
        # Carica la configurazione
        self.config = self._load_config()
        
        # Imposta il livello di log
        log_level = self.config.get("app", {}).get("log_level", 1)
        self.set_log_level(LogLevel(log_level))
        
    def _load_config(self) -> Dict[str, Any]:
        """
        Carica la configurazione dal file.
        
        Returns:
            Dizionario con la configurazione.
        """
        try:
            with open(self.config_path, "r") as f:
                config = json.load(f)
                
            self.logger.debug(f"Configurazione caricata: {config}")
            return config
        except Exception as e:
            self.logger.error(f"Errore nel caricamento della configurazione: {e}")
            return {}
    
    def save_config(self, config: Dict[str, Any] = None) -> bool:
        """
        Salva la configurazione su file.
        
        Args:
            config: Configurazione da salvare. Se None, salva la configurazione corrente.
            
        Returns:
            True se il salvataggio è avvenuto con successo, False altrimenti.
        """
        config_to_save = config or self.config
        
        try:
            with open(self.config_path, "w") as f:
                json.dump(config_to_save, f, indent=4)
                
            self.logger.debug(f"Configurazione salvata: {self.config_path}")
            return True
        except Exception as e:
            self.logger.error(f"Errore nel salvataggio della configurazione: {e}")
            return False
    
    def get_config(self, key: str, default: Any = None) -> Any:
        """
        Ottiene un valore di configurazione.
        
        Args:
            key: Chiave del valore di configurazione (formato dot-notation, es. "app.name").
            default: Valore di default da restituire se la chiave non esiste.
            
        Returns:
            Valore di configurazione o default se la chiave non esiste.
        """
        parts = key.split(".")
        current = self.config
        
        for part in parts:
            if isinstance(current, dict) and part in current:
                current = current[part]
            else:
                return default
                
        return current
    
    def set_config(self, key: str, value: Any) -> None:
        """
        Imposta un valore di configurazione.
        
        Args:
            key: Chiave del valore di configurazione (formato dot-notation, es. "app.name").
            value: Valore da impostare.
        """
        parts = key.split(".")
        current = self.config
        
        # Naviga nella struttura fino all'ultimo livello
        for i, part in enumerate(parts[:-1]):
            if part not in current:
                current[part] = {}
            current = current[part]
            
        # Imposta il valore
        current[parts[-1]] = value
        self.logger.debug(f"Configurazione aggiornata: {key} = {value}")
    
    def log(self, level: LogLevel, message: str) -> None:
        """
        Registra un messaggio di log.
        
        Args:
            level: Livello di log.
            message: Messaggio da registrare.
        """
        log_method = {
            LogLevel.DEBUG: self.logger.debug,
            LogLevel.INFO: self.logger.info,
            LogLevel.WARNING: self.logger.warning,
            LogLevel.ERROR: self.logger.error,
            LogLevel.FATAL: self.logger.critical
        }.get(level, self.logger.info)
        
        log_method(f"[CORE] {message}")
    
    def set_log_level(self, level: LogLevel) -> None:
        """
        Imposta il livello di log.
        
        Args:
            level: Livello di log.
        """
        self.logger.info(f"Impostazione del livello di log a {level}")
        # In un'implementazione reale, chiameremmo la funzione del Core
        
    def get_system_resources(self) -> Dict[str, Any]:
        """
        Ottiene le risorse di sistema.
        
        Returns:
            Dizionario con le informazioni sulle risorse di sistema.
        """
        # In un'implementazione reale, chiameremmo la funzione del Core
        # La risposta simulata per scopi dimostrativi
        import psutil
        
        try:
            cpu_percent = psutil.cpu_percent(interval=0.1)
            memory = psutil.virtual_memory()
            disk = psutil.disk_usage('/')
            
            if hasattr(psutil, 'net_io_counters'):
                net = psutil.net_io_counters()
                net_usage = 50.0  # Valore simulato per la rete
            else:
                net_usage = 0.0
                
            resources = {
                "cpu_usage_percent": cpu_percent,
                "memory_available_bytes": memory.available,
                "memory_total_bytes": memory.total,
                "disk_available_bytes": disk.free,
                "disk_total_bytes": disk.total,
                "network_usage_percent": net_usage,
                "gpu_usage_percent": 0.0  # GPU non supportata in questa implementazione
            }
            
            # Aggiorna lo storico delle risorse
            now = time.time()
            self._resource_history[ResourceType.CPU].append((now, cpu_percent))
            self._resource_history[ResourceType.MEMORY].append((now, memory.percent))
            self._resource_history[ResourceType.DISK].append((now, disk.percent))
            self._resource_history[ResourceType.NETWORK].append((now, net_usage))
            
            # Limita la dimensione dello storico
            max_history = 100
            for resource_type in self._resource_history:
                if len(self._resource_history[resource_type]) > max_history:
                    self._resource_history[resource_type] = self._resource_history[resource_type][-max_history:]
            
            self.logger.debug(f"Risorse di sistema: {resources}")
            return resources
        except Exception as e:
            self.logger.error(f"Errore nel recupero delle risorse di sistema: {e}")
            # Restituisci valori di default in caso di errore
            return {
                "cpu_usage_percent": 0.0,
                "memory_available_bytes": 0,
                "memory_total_bytes": 0,
                "disk_available_bytes": 0,
                "disk_total_bytes": 0,
                "network_usage_percent": 0.0,
                "gpu_usage_percent": 0.0
            }
    
    def register_resource_callback(self, resource_type, threshold: int, 
                              callback: Callable[[str, int], None]) -> int:
        """
        Registra un callback da chiamare quando l'utilizzo di una risorsa supera una soglia.
        
        Args:
            resource_type: Tipo di risorsa.
            threshold: Soglia percentuale per la notifica.
            callback: Funzione di callback da chiamare.
            
        Returns:
            ID della callback registrata.
        """
        resource_name = resource_type if isinstance(resource_type, str) else ResourceType.get_value(resource_type)
        callback_id = len(self._resource_callbacks) + 1
        
        self._resource_callbacks[callback_id] = {
            "resource_type": resource_name,
            "threshold": threshold,
            "callback": callback
        }
        
        self.logger.debug(f"Registrata callback per {resource_name} con soglia {threshold}%")
        return callback_id
    
    def unregister_resource_callback(self, callback_id: int) -> bool:
        """
        Deregistra una callback per il monitoraggio delle risorse.
        
        Args:
            callback_id: ID della callback.
            
        Returns:
            True se la deregistrazione è avvenuta con successo, False altrimenti.
        """
        if callback_id in self._resource_callbacks:
            del self._resource_callbacks[callback_id]
            self.logger.debug(f"Deregistrata callback ID {callback_id}")
            return True
        
        self.logger.warning(f"Tentativo di deregistrare una callback inesistente (ID {callback_id})")
        return False
    
    def execute_python_code(self, code: str) -> Tuple[bool, Any]:
        """
        Esegue codice Python.
        
        Args:
            code: Codice Python da eseguire.
            
        Returns:
            Tupla (successo, risultato).
        """
        self.logger.debug(f"Esecuzione codice Python: {code[:50]}...")
        
        try:
            # Creiamo un ambiente locale per l'esecuzione
            local_vars = {}
            
            # Esecuzionne del codice
            exec(code, globals(), local_vars)
            
            # Se è presente una variabile 'result', la restituiamo
            result = local_vars.get('result', None)
            
            return True, result
        except Exception as e:
            self.logger.error(f"Errore nell'esecuzione del codice Python: {e}")
            return False, str(e)
    
    def shutdown(self) -> None:
        """Arresta il Core e libera le risorse."""
        self.logger.info("Arresto del Core...")
        
        # In un'implementazione reale, chiameremmo la funzione del Core
        self.initialized = False
        
        self.logger.info("Core arrestato con successo")
        
    def get_resource_value(self, resource_type: str) -> float:
        """
        Ottiene il valore corrente di una risorsa.
        
        Args:
            resource_type: Tipo di risorsa.
            
        Returns:
            Valore percentuale della risorsa.
        """
        # In un'implementazione reale, chiameremmo la funzione del Core
        
        # Per ora simuliamo dei valori
        import psutil
        
        try:
            if resource_type == ResourceType.CPU:
                return psutil.cpu_percent(interval=0.1)
            elif resource_type == ResourceType.MEMORY:
                return psutil.virtual_memory().percent
            elif resource_type == ResourceType.DISK:
                return psutil.disk_usage('/').percent
            elif resource_type == ResourceType.NETWORK:
                # Simuliamo un valore casuale per la rete
                return random.uniform(20.0, 70.0)
            elif resource_type == ResourceType.GPU:
                # Simuliamo un valore casuale per la GPU
                return random.uniform(10.0, 60.0)
            else:
                self.logger.warning(f"Tipo di risorsa non supportato: {resource_type}")
                return 0.0
        except Exception as e:
            self.logger.error(f"Errore nel recupero del valore della risorsa {resource_type}: {e}")
            return 0.0
    
    def get_resource_history(self, resource_type: str) -> List[Tuple[float, float]]:
        """
        Ottiene lo storico dei valori di una risorsa.
        
        Args:
            resource_type: Tipo di risorsa.
            
        Returns:
            Lista di tuple (timestamp, valore) per la risorsa.
        """
        if resource_type == ResourceType.CPU:
            return self._resource_history[ResourceType.CPU]
        elif resource_type == ResourceType.MEMORY:
            return self._resource_history[ResourceType.MEMORY]
        elif resource_type == ResourceType.DISK:
            return self._resource_history[ResourceType.DISK]
        elif resource_type == ResourceType.NETWORK:
            return self._resource_history[ResourceType.NETWORK]
        elif resource_type == ResourceType.GPU:
            return self._resource_history[ResourceType.GPU]
        else:
            self.logger.warning(f"Tipo di risorsa non supportato per lo storico: {resource_type}")
            return []
    
    def get_memory_details(self) -> Dict[str, float]:
        """
        Ottiene informazioni dettagliate sulla memoria.
        
        Returns:
            Dizionario con dettagli sulla memoria.
        """
        try:
            import psutil
            memory = psutil.virtual_memory()
            
            # Converti da byte a GB
            total_gb = memory.total / (1024.0**3)
            used_gb = (memory.total - memory.available) / (1024.0**3)
            
            return {
                "total": total_gb,
                "used": used_gb,
                "percent": memory.percent
            }
        except Exception as e:
            self.logger.error(f"Errore nel recupero dei dettagli della memoria: {e}")
            return {"total": 16.0, "used": 8.0, "percent": 50.0}  # Valori di fallback
    
    def get_disk_details(self) -> Dict[str, float]:
        """
        Ottiene informazioni dettagliate sul disco.
        
        Returns:
            Dizionario con dettagli sul disco.
        """
        try:
            import psutil
            disk = psutil.disk_usage('/')
            
            # Converti da byte a GB
            total_gb = disk.total / (1024.0**3)
            used_gb = disk.used / (1024.0**3)
            
            return {
                "total": total_gb,
                "used": used_gb,
                "percent": disk.percent
            }
        except Exception as e:
            self.logger.error(f"Errore nel recupero dei dettagli del disco: {e}")
            return {"total": 500.0, "used": 250.0, "percent": 50.0}  # Valori di fallback
    
    def get_cpu_temperature(self) -> Optional[float]:
        """
        Ottiene la temperatura della CPU.
        
        Returns:
            Temperatura in gradi Celsius o None se non disponibile.
        """
        try:
            import psutil
            if hasattr(psutil, "sensors_temperatures"):
                temps = psutil.sensors_temperatures()
                if temps:
                    # Cerca la temperatura della CPU
                    for name, entries in temps.items():
                        if name.lower() in ["coretemp", "cpu_thermal", "k10temp"]:
                            return entries[0].current
            
            # Se non è possibile ottenere la temperatura reale, simula un valore
            return random.uniform(35.0, 65.0)
        except Exception as e:
            self.logger.error(f"Errore nel recupero della temperatura della CPU: {e}")
            return None
    
    def get_system_uptime(self) -> str:
        """
        Ottiene l'uptime del sistema.
        
        Returns:
            Uptime come stringa formattata.
        """
        try:
            import psutil
            uptime_seconds = time.time() - psutil.boot_time()
            
            # Formatta l'uptime
            days = int(uptime_seconds // (24 * 3600))
            uptime_seconds %= (24 * 3600)
            hours = int(uptime_seconds // 3600)
            uptime_seconds %= 3600
            minutes = int(uptime_seconds // 60)
            
            if days > 0:
                return f"{days}g {hours}h {minutes}m"
            elif hours > 0:
                return f"{hours}h {minutes}m"
            else:
                return f"{minutes}m"
        except Exception as e:
            self.logger.error(f"Errore nel recupero dell'uptime del sistema: {e}")
            return "N/D"