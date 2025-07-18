#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
CorePythonAPI - Interfaccia Python per CoreBaseApplication

Questo modulo fornisce un'interfaccia Python per accedere alle funzionalità
del CoreBaseApplication. Semplifica l'uso delle API C++ attraverso una
classe wrapper che espone le principali funzionalità.
"""

import json
import ctypes
import os
import sys
from enum import Enum
from typing import List, Dict, Any, Callable, Union, Optional, Tuple
import pybind11
from pybind11 import get_cmake_dir

# Questo file è un placeholder. L'implementazione reale dei bindings C++ avverrà in un modulo compilato (es: python_bindings.so/dll)
# Esempio di come sarà l'interfaccia Python:

class ErrorHandler:
    def initialize(self): ...
    def shutdown(self): ...
    def setLogLevel(self, level: int): ...
    def getLogLevel(self) -> int: ...
    def log(self, level: int, message: str): ...
    def debug(self, message: str): ...
    def info(self, message: str): ...
    def warning(self, message: str): ...
    def error(self, message: str): ...
    def critical(self, message: str): ...
    def handleError(self, message: str, file: str, line: int, function: str): ...

# La vera implementazione sarà generata da pybind11 in fase di build C++


class LogLevel(Enum):
    """Livelli di log supportati"""
    DEBUG = 0
    INFO = 1
    WARNING = 2
    ERROR = 3
    FATAL = 4


class ModuleType(Enum):
    """Tipi di moduli supportati"""
    CPP = 0
    PYTHON = 1
    JAVA = 2


class IPCType(Enum):
    """Tipi di comunicazione IPC supportati"""
    SHARED_MEMORY = 0
    NAMED_PIPE = 1
    SOCKET = 2
    MESSAGE_QUEUE = 3


class IPCRole(Enum):
    """Ruolo in una comunicazione IPC"""
    SERVER = 0
    CLIENT = 1


class SystemResources:
    """Classe che rappresenta le risorse di sistema"""
    
    def __init__(self, data: Dict[str, Any] = None):
        """
        Inizializza le risorse di sistema
        
        :param data: Dizionario con i dati delle risorse (opzionale)
        """
        self.cpu_usage_percent = 0
        self.available_memory_bytes = 0
        self.total_memory_bytes = 0
        self.available_disk_bytes = 0
        self.total_disk_bytes = 0
        self.network_usage_percent = 0
        self.gpu_usage_percent = 0
        
        if data:
            self.from_dict(data)
    
    def from_dict(self, data: Dict[str, Any]) -> None:
        """
        Popola i dati dalle risorse di sistema da un dizionario
        
        :param data: Dizionario con i dati delle risorse
        """
        self.cpu_usage_percent = data.get('cpuUsagePercent', 0)
        self.available_memory_bytes = data.get('availableMemoryBytes', 0)
        self.total_memory_bytes = data.get('totalMemoryBytes', 0)
        self.available_disk_bytes = data.get('availableDiskBytes', 0)
        self.total_disk_bytes = data.get('totalDiskBytes', 0)
        self.network_usage_percent = data.get('networkUsagePercent', 0)
        self.gpu_usage_percent = data.get('gpuUsagePercent', 0)
    
    def to_dict(self) -> Dict[str, Any]:
        """
        Converte le risorse di sistema in un dizionario
        
        :return: Dizionario con i dati delle risorse
        """
        return {
            'cpuUsagePercent': self.cpu_usage_percent,
            'availableMemoryBytes': self.available_memory_bytes,
            'totalMemoryBytes': self.total_memory_bytes,
            'availableDiskBytes': self.available_disk_bytes,
            'totalDiskBytes': self.total_disk_bytes,
            'networkUsagePercent': self.network_usage_percent,
            'gpuUsagePercent': self.gpu_usage_percent
        }
    
    def __str__(self) -> str:
        """
        Rappresentazione leggibile delle risorse di sistema
        
        :return: Stringa formattata con le risorse
        """
        return (
            f"CPU: {self.cpu_usage_percent}% | "
            f"Memory: {self.available_memory_bytes / (1024**2):.1f}MB / {self.total_memory_bytes / (1024**2):.1f}MB | "
            f"Disk: {self.available_disk_bytes / (1024**3):.1f}GB / {self.total_disk_bytes / (1024**3):.1f}GB | "
            f"Network: {self.network_usage_percent}% | "
            f"GPU: {self.gpu_usage_percent}%"
        )


class ModuleInfo:
    """Classe che rappresenta le informazioni di un modulo"""
    
    def __init__(self, data: Dict[str, Any] = None):
        """
        Inizializza le informazioni del modulo
        
        :param data: Dizionario con i dati del modulo (opzionale)
        """
        self.name = ""
        self.version = ""
        self.author = ""
        self.description = ""
        self.type = ModuleType.CPP
        self.dependencies = []
        self.is_loaded = False
        
        if data:
            self.from_dict(data)
    
    def from_dict(self, data: Dict[str, Any]) -> None:
        """
        Popola i dati del modulo da un dizionario
        
        :param data: Dizionario con i dati del modulo
        """
        self.name = data.get('name', '')
        self.version = data.get('version', '')
        self.author = data.get('author', '')
        self.description = data.get('description', '')
        
        module_type = data.get('type', 0)
        if isinstance(module_type, int):
            self.type = ModuleType(module_type)
        
        self.dependencies = data.get('dependencies', [])
        self.is_loaded = data.get('isLoaded', False)
    
    def to_dict(self) -> Dict[str, Any]:
        """
        Converte le informazioni del modulo in un dizionario
        
        :return: Dizionario con i dati del modulo
        """
        return {
            'name': self.name,
            'version': self.version,
            'author': self.author,
            'description': self.description,
            'type': self.type.value,
            'dependencies': self.dependencies,
            'isLoaded': self.is_loaded
        }
    
    def __str__(self) -> str:
        """
        Rappresentazione leggibile delle informazioni del modulo
        
        :return: Stringa formattata con le informazioni
        """
        return (
            f"Module: {self.name} (v{self.version})\n"
            f"Author: {self.author}\n"
            f"Type: {self.type.name}\n"
            f"Description: {self.description}\n"
            f"Dependencies: {', '.join(self.dependencies)}\n"
            f"Loaded: {'Yes' if self.is_loaded else 'No'}"
        )


class CoreAPIException(Exception):
    """Eccezione base per gli errori di CoreAPI"""
    pass


class CoreInitializationError(CoreAPIException):
    """Eccezione per errori di inizializzazione"""
    pass


class ModuleLoadError(CoreAPIException):
    """Eccezione per errori di caricamento moduli"""
    pass


class ConfigError(CoreAPIException):
    """Eccezione per errori di configurazione"""
    pass


class IPCError(CoreAPIException):
    """Eccezione per errori di comunicazione IPC"""
    pass


class CoreAPI:
    """Interfaccia Python per CoreBaseApplication"""
    
    def __init__(self, lib_path: str = None, config_path: str = "config.json"):
        """
        Inizializza l'interfaccia Python per CoreAPI
        
        :param lib_path: Percorso della libreria C++ (opzionale)
        :param config_path: Percorso del file di configurazione (opzionale)
        :raises CoreInitializationError: Se l'inizializzazione fallisce
        """
        # Carica la libreria C++
        if lib_path is None:
            # Prova a trovare la libreria in posizioni standard
            if sys.platform == "win32":
                lib_names = ["CoreAPI.dll", "libCoreAPI.dll"]
            elif sys.platform == "darwin":
                lib_names = ["libCoreAPI.dylib"]
            else:
                lib_names = ["libCoreAPI.so"]
            
            found = False
            for name in lib_names:
                try:
                    self._lib = ctypes.CDLL(name)
                    found = True
                    break
                except OSError:
                    continue
            
            if not found:
                raise CoreInitializationError("Impossibile trovare la libreria CoreAPI")
        else:
            try:
                self._lib = ctypes.CDLL(lib_path)
            except OSError as e:
                raise CoreInitializationError(f"Impossibile caricare la libreria: {e}")
        
        # Configurazione dei prototipi delle funzioni C++
        self._configure_lib_functions()
        
        # Inizializza CoreAPI
        self._handle = self._create_api(config_path.encode('utf-8'))
        if not self._handle:
            raise CoreInitializationError("Impossibile creare l'istanza di CoreAPI")
        
        # Lista di callback registrate per evitare il garbage collection
        self._callbacks = {}
    
    def _configure_lib_functions(self) -> None:
        """
        Configura i prototipi delle funzioni della libreria C++
        """
        # Funzioni di creazione/distruzione dell'API
        self._lib.CreateCoreAPI.argtypes = [ctypes.c_char_p]
        self._lib.CreateCoreAPI.restype = ctypes.c_void_p
        
        self._lib.DestroyCoreAPI.argtypes = [ctypes.c_void_p]
        self._lib.DestroyCoreAPI.restype = None
        
        # Inizializzazione e shutdown
        self._lib.Initialize.argtypes = [ctypes.c_void_p]
        self._lib.Initialize.restype = ctypes.c_bool
        
        self._lib.Shutdown.argtypes = [ctypes.c_void_p]
        self._lib.Shutdown.restype = None
        
        # Configurazione
        self._lib.LoadConfig.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.LoadConfig.restype = ctypes.c_bool
        
        self._lib.SaveConfig.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.SaveConfig.restype = ctypes.c_bool
        
        self._lib.GetConfigString.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]
        self._lib.GetConfigString.restype = ctypes.c_char_p
        
        self._lib.GetConfigInt.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int]
        self._lib.GetConfigInt.restype = ctypes.c_int
        
        self._lib.GetConfigDouble.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double]
        self._lib.GetConfigDouble.restype = ctypes.c_double
        
        self._lib.GetConfigBool.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_bool]
        self._lib.GetConfigBool.restype = ctypes.c_bool
        
        self._lib.SetConfigString.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]
        self._lib.SetConfigString.restype = None
        
        self._lib.SetConfigInt.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int]
        self._lib.SetConfigInt.restype = None
        
        self._lib.SetConfigDouble.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double]
        self._lib.SetConfigDouble.restype = None
        
        self._lib.SetConfigBool.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_bool]
        self._lib.SetConfigBool.restype = None
        
        # Gestione moduli
        self._lib.LoadModule.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.LoadModule.restype = ctypes.c_bool
        
        self._lib.UnloadModule.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.UnloadModule.restype = ctypes.c_bool
        
        self._lib.IsModuleLoaded.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.IsModuleLoaded.restype = ctypes.c_bool
        
        self._lib.GetModuleInfo.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.GetModuleInfo.restype = ctypes.c_char_p
        
        self._lib.GetLoadedModules.argtypes = [ctypes.c_void_p]
        self._lib.GetLoadedModules.restype = ctypes.c_char_p
        
        # Logging
        self._lib.Log.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_char_p, 
                                 ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p]
        self._lib.Log.restype = None
        
        self._lib.LogError.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.LogError.restype = None
        
        self._lib.LogWarning.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.LogWarning.restype = None
        
        self._lib.LogInfo.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.LogInfo.restype = None
        
        self._lib.LogDebug.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.LogDebug.restype = None
        
        self._lib.SetLogLevel.argtypes = [ctypes.c_void_p, ctypes.c_int]
        self._lib.SetLogLevel.restype = None
        
        # Risorse
        self._lib.GetSystemResources.argtypes = [ctypes.c_void_p]
        self._lib.GetSystemResources.restype = ctypes.c_char_p
        
        self._lib.GetAvailableMemory.argtypes = [ctypes.c_void_p]
        self._lib.GetAvailableMemory.restype = ctypes.c_ulonglong
        
        self._lib.GetCpuUsage.argtypes = [ctypes.c_void_p]
        self._lib.GetCpuUsage.restype = ctypes.c_int
        
        # IPC
        self._lib.CreateIPCChannel.argtypes = [ctypes.c_void_p, ctypes.c_char_p, 
                                             ctypes.c_int, ctypes.c_int, ctypes.c_char_p]
        self._lib.CreateIPCChannel.restype = ctypes.c_bool
        
        self._lib.SendData.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_size_t]
        self._lib.SendData.restype = ctypes.c_bool
        
        self._lib.ReceiveData.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p, 
                                        ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)]
        self._lib.ReceiveData.restype = ctypes.c_bool
        
        self._lib.CloseIPCChannel.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.CloseIPCChannel.restype = ctypes.c_bool
        
        # Funzioni Python specifiche
        self._lib.ExecutePythonFunction.argtypes = [ctypes.c_void_p, ctypes.c_char_p, 
                                                  ctypes.c_char_p, ctypes.c_char_p]
        self._lib.ExecutePythonFunction.restype = ctypes.c_bool
        
        self._lib.ExecutePythonCode.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.ExecutePythonCode.restype = ctypes.c_bool
    
    def _create_api(self, config_path: bytes) -> ctypes.c_void_p:
        """
        Crea l'istanza di CoreAPI
        
        :param config_path: Percorso del file di configurazione
        :return: Handle all'istanza di CoreAPI
        """
        return self._lib.CreateCoreAPI(config_path)
    
    def __del__(self) -> None:
        """
        Distruttore che pulisce le risorse
        """
        if hasattr(self, '_handle') and self._handle:
            self._lib.DestroyCoreAPI(self._handle)
            self._handle = None
    
    def initialize(self) -> bool:
        """
        Inizializza il Core e tutti i suoi componenti
        
        :return: True se l'inizializzazione è avvenuta con successo, False altrimenti
        """
        return self._lib.Initialize(self._handle)
    
    def shutdown(self) -> None:
        """
        Arresta il Core e libera le risorse
        """
        self._lib.Shutdown(self._handle)
    
    # Gestione configurazioni
    def load_config(self, file_path: str) -> bool:
        """
        Carica una configurazione da file
        
        :param file_path: Percorso del file di configurazione
        :return: True se il caricamento è avvenuto con successo, False altrimenti
        """
        return self._lib.LoadConfig(self._handle, file_path.encode('utf-8'))
    
    def save_config(self, file_path: str = "") -> bool:
        """
        Salva la configurazione corrente su file
        
        :param file_path: Percorso del file (se vuoto, usa l'ultimo file caricato)
        :return: True se il salvataggio è avvenuto con successo, False altrimenti
        """
        return self._lib.SaveConfig(self._handle, file_path.encode('utf-8'))
    
    def get_config_string(self, key: str, default_value: str = "") -> str:
        """
        Ottiene un valore di configurazione come stringa
        
        :param key: Chiave di configurazione (formato: "sezione.chiave")
        :param default_value: Valore di default se la chiave non esiste
        :return: Valore della configurazione
        """
        result = self._lib.GetConfigString(self._handle, key.encode('utf-8'), default_value.encode('utf-8'))
        return result.decode('utf-8') if result else default_value
    
    def get_config_int(self, key: str, default_value: int = 0) -> int:
        """
        Ottiene un valore di configurazione come intero
        
        :param key: Chiave di configurazione (formato: "sezione.chiave")
        :param default_value: Valore di default se la chiave non esiste
        :return: Valore della configurazione
        """
        return self._lib.GetConfigInt(self._handle, key.encode('utf-8'), default_value)
    
    def get_config_double(self, key: str, default_value: float = 0.0) -> float:
        """
        Ottiene un valore di configurazione come double
        
        :param key: Chiave di configurazione (formato: "sezione.chiave")
        :param default_value: Valore di default se la chiave non esiste
        :return: Valore della configurazione
        """
        return self._lib.GetConfigDouble(self._handle, key.encode('utf-8'), default_value)
    
    def get_config_bool(self, key: str, default_value: bool = False) -> bool:
        """
        Ottiene un valore di configurazione come boolean
        
        :param key: Chiave di configurazione (formato: "sezione.chiave")
        :param default_value: Valore di default se la chiave non esiste
        :return: Valore della configurazione
        """
        return self._lib.GetConfigBool(self._handle, key.encode('utf-8'), default_value)
    
    def set_config(self, key: str, value: Union[str, int, float, bool]) -> None:
        """
        Imposta un valore di configurazione
        
        :param key: Chiave di configurazione (formato: "sezione.chiave")
        :param value: Valore da impostare
        """
        key_bytes = key.encode('utf-8')
        
        if isinstance(value, str):
            self._lib.SetConfigString(self._handle, key_bytes, value.encode('utf-8'))
        elif isinstance(value, int):
            self._lib.SetConfigInt(self._handle, key_bytes, value)
        elif isinstance(value, float):
            self._lib.SetConfigDouble(self._handle, key_bytes, value)
        elif isinstance(value, bool):
            self._lib.SetConfigBool(self._handle, key_bytes, value)
        else:
            raise TypeError(f"Tipo non supportato: {type(value)}")
    
    # Gestione moduli
    def load_module(self, module_path: str) -> bool:
        """
        Carica un modulo
        
        :param module_path: Percorso del modulo da caricare
        :return: True se il caricamento è avvenuto con successo, False altrimenti
        :raises ModuleLoadError: Se il caricamento fallisce
        """
        result = self._lib.LoadModule(self._handle, module_path.encode('utf-8'))
        if not result:
            raise ModuleLoadError(f"Impossibile caricare il modulo: {module_path}")
        return result
    
    def unload_module(self, module_name: str) -> bool:
        """
        Scarica un modulo
        
        :param module_name: Nome del modulo da scaricare
        :return: True se lo scaricamento è avvenuto con successo, False altrimenti
        """
        return self._lib.UnloadModule(self._handle, module_name.encode('utf-8'))
    
    def is_module_loaded(self, module_name: str) -> bool:
        """
        Verifica se un modulo è caricato
        
        :param module_name: Nome del modulo
        :return: True se il modulo è caricato, False altrimenti
        """
        return self._lib.IsModuleLoaded(self._handle, module_name.encode('utf-8'))
    
    def get_module_info(self, module_name: str) -> ModuleInfo:
        """
        Ottiene le informazioni di un modulo
        
        :param module_name: Nome del modulo
        :return: Informazioni sul modulo
        """
        result = self._lib.GetModuleInfo(self._handle, module_name.encode('utf-8'))
        if not result:
            return ModuleInfo()
        
        json_str = result.decode('utf-8')
        data = json.loads(json_str)
        return ModuleInfo(data)
    
    def get_loaded_modules(self) -> List[str]:
        """
        Ottiene tutti i moduli caricati
        
        :return: Lista contenente i nomi dei moduli caricati
        """
        result = self._lib.GetLoadedModules(self._handle)
        if not result:
            return []
        
        json_str = result.decode('utf-8')
        return json.loads(json_str)
    
    # Logging e gestione errori
    def log(self, level: LogLevel, message: str, source_file: str = "", 
          line_number: int = 0, function_name: str = "") -> None:
        """
        Registra un messaggio di log
        
        :param level: Livello del log
        :param message: Messaggio da registrare
        :param source_file: Nome del file sorgente (opzionale)
        :param line_number: Numero di riga nel file sorgente (opzionale)
        :param function_name: Nome della funzione (opzionale)
        """
        self._lib.Log(
            self._handle,
            level.value,
            message.encode('utf-8'),
            source_file.encode('utf-8'),
            line_number,
            function_name.encode('utf-8')
        )
    
    def log_error(self, message: str) -> None:
        """
        Registra un errore
        
        :param message: Messaggio di errore
        """
        self._lib.LogError(self._handle, message.encode('utf-8'))
    
    def log_warning(self, message: str) -> None:
        """
        Registra un avviso
        
        :param message: Messaggio di avviso
        """
        self._lib.LogWarning(self._handle, message.encode('utf-8'))
    
    def log_info(self, message: str) -> None:
        """
        Registra un messaggio informativo
        
        :param message: Messaggio informativo
        """
        self._lib.LogInfo(self._handle, message.encode('utf-8'))
    
    def log_debug(self, message: str) -> None:
        """
        Registra un messaggio di debug
        
        :param message: Messaggio di debug
        """
        self._lib.LogDebug(self._handle, message.encode('utf-8'))
    
    def set_log_level(self, level: LogLevel) -> None:
        """
        Imposta il livello minimo di log da registrare
        
        :param level: Livello minimo di log
        """
        self._lib.SetLogLevel(self._handle, level.value)
    
    # Gestione risorse
    def get_system_resources(self) -> SystemResources:
        """
        Ottiene le informazioni sulle risorse di sistema
        
        :return: Oggetto contenente informazioni sulle risorse
        """
        result = self._lib.GetSystemResources(self._handle)
        if not result:
            return SystemResources()
        
        json_str = result.decode('utf-8')
        data = json.loads(json_str)
        return SystemResources(data)
    
    def get_available_memory(self) -> int:
        """
        Ottiene la quantità di memoria disponibile
        
        :return: Memoria disponibile in bytes
        """
        return self._lib.GetAvailableMemory(self._handle)
    
    def get_cpu_usage(self) -> int:
        """
        Ottiene l'utilizzo della CPU
        
        :return: Percentuale di utilizzo della CPU (0-100)
        """
        return self._lib.GetCpuUsage(self._handle)
    
    # IPC
    def create_ipc_channel(self, name: str, channel_type: IPCType,
                         role: IPCRole = IPCRole.SERVER,
                         params: str = "") -> bool:
        """
        Crea un canale di comunicazione IPC
        
        :param name: Nome del canale
        :param channel_type: Tipo di canale
        :param role: Ruolo (server o client)
        :param params: Parametri specifici per il tipo di canale (es. host:port per socket)
        :return: True se la creazione è avvenuta con successo, False altrimenti
        :raises IPCError: Se la creazione fallisce
        """
        result = self._lib.CreateIPCChannel(
            self._handle,
            name.encode('utf-8'),
            channel_type.value,
            role.value,
            params.encode('utf-8')
        )
        
        if not result:
            raise IPCError(f"Impossibile creare il canale IPC: {name}")
        
        return result
    
    def send_data(self, channel_name: str, data: bytes) -> bool:
        """
        Invia dati su un canale IPC
        
        :param channel_name: Nome del canale
        :param data: Dati da inviare
        :return: True se l'invio è avvenuto con successo, False altrimenti
        """
        buf = ctypes.create_string_buffer(data)
        return self._lib.SendData(
            self._handle,
            channel_name.encode('utf-8'),
            ctypes.byref(buf),
            len(data)
        )
    
    def receive_data(self, channel_name: str, buffer_size: int = 1024) -> Tuple[bool, bytes, int]:
        """
        Riceve dati da un canale IPC
        
        :param channel_name: Nome del canale
        :param buffer_size: Dimensione del buffer
        :return: Tupla (successo, dati, bytes_letti)
        """
        buf = ctypes.create_string_buffer(buffer_size)
        bytes_read = ctypes.c_size_t(0)
        
        success = self._lib.ReceiveData(
            self._handle,
            channel_name.encode('utf-8'),
            ctypes.byref(buf),
            buffer_size,
            ctypes.byref(bytes_read)
        )
        
        if success:
            return success, buf.raw[:bytes_read.value], bytes_read.value
        else:
            return success, b"", 0
    
    def close_ipc_channel(self, channel_name: str) -> bool:
        """
        Chiude un canale IPC
        
        :param channel_name: Nome del canale
        :return: True se la chiusura è avvenuta con successo, False altrimenti
        """
        return self._lib.CloseIPCChannel(self._handle, channel_name.encode('utf-8'))
    
    # Funzioni Python specifiche
    def execute_function(self, module_name: str, func_name: str, *args) -> Any:
        """
        Esegue una funzione Python
        
        :param module_name: Nome del modulo
        :param func_name: Nome della funzione
        :param args: Argomenti da passare alla funzione
        :return: Risultato della funzione
        """
        args_json = json.dumps(args)
        
        result = self._lib.ExecutePythonFunction(
            self._handle,
            module_name.encode('utf-8'),
            func_name.encode('utf-8'),
            args_json.encode('utf-8')
        )
        
        return result
    
    def execute_code(self, code: str) -> bool:
        """
        Esegue un codice Python
        
        :param code: Codice Python da eseguire
        :return: True se l'esecuzione è avvenuta con successo, False altrimenti
        """
        return self._lib.ExecutePythonCode(self._handle, code.encode('utf-8'))


# Esempio di utilizzo
if __name__ == "__main__":
    try:
        # Inizializza l'API
        core = CoreAPI()
        if not core.initialize():
            print("Impossibile inizializzare CoreAPI")
            sys.exit(1)
        
        # Esempio di configurazione
        core.set_config("app.name", "TestPythonApp")
        core.set_config("app.version", "1.0.0")
        
        # Stampa informazioni di configurazione
        print(f"App Name: {core.get_config_string('app.name')}")
        print(f"App Version: {core.get_config_string('app.version')}")
        
        # Esempio di log
        core.log_info("Applicazione avviata con successo")
        
        # Ottieni informazioni sulle risorse
        resources = core.get_system_resources()
        print(f"Risorse di sistema: {resources}")
        
        # Crea un canale IPC
        try:
            core.create_ipc_channel("test_channel", IPCType.SHARED_MEMORY, params="1024")
            print("Canale IPC creato con successo")
            
            # Invia dati
            data = b"Hello from Python!"
            if core.send_data("test_channel", data):
                print("Dati inviati con successo")
            
            # Chiudi il canale
            core.close_ipc_channel("test_channel")
            
        except IPCError as e:
            print(f"Errore IPC: {e}")
        
        # Esegui codice Python
        core.execute_code("print('Codice Python eseguito tramite API')")
        
        # Chiudi l'API
        core.shutdown()
        print("CoreAPI chiuso con successo")
        
    except CoreAPIException as e:
        print(f"Errore CoreAPI: {e}")
        sys.exit(1) 