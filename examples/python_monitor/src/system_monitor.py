#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Modulo per il monitoraggio delle risorse di sistema.
Fornisce funzionalità per monitorare CPU, memoria, disco e altre risorse.
"""

import logging
import time
import threading
from typing import Dict, List, Callable, Any, Tuple, Optional
from dataclasses import dataclass
from enum import Enum

from core_interface import CoreInterface, ResourceType

@dataclass
class SystemResources:
    """Classe per rappresentare le risorse di sistema."""
    # Percentuali di utilizzo
    cpu_usage: float
    memory_usage: float
    disk_usage: float
    
    # Valori assoluti in GB
    memory_used: float
    memory_total: float
    disk_used: float
    disk_total: float
    
    # Valori opzionali
    network_usage: Optional[float] = None
    gpu_usage: Optional[float] = None
    
    # Informazioni aggiuntive
    cpu_temp: Optional[float] = None
    uptime: str = "N/D"

class SystemMonitor:
    """Classe per il monitoraggio delle risorse di sistema."""
    
    def __init__(self, core: CoreInterface = None):
        """
        Inizializza il monitor di sistema.
        
        Args:
            core: Interfaccia con il Core (opzionale).
        """
        self.logger = logging.getLogger(__name__)
        
        # Crea un'interfaccia Core predefinita se non fornita
        self.core = core or CoreInterface()
        
        # Storico delle misurazioni (dizionario che mappa tipo_risorsa -> lista di tuple (timestamp, valore))
        self.history: Dict[str, List[Tuple[float, float]]] = {
            "cpu": [],
            "memory": [],
            "disk": [],
            "network": [],
            "gpu": []
        }
        
        # Limite della dimensione della storia (numero di campioni)
        self.history_size = 100
        
        # Lista di callback da notificare quando i dati cambiano
        self.callbacks: List[Callable] = []
        
        # Configura il monitoraggio
        self._configure()
        
        # Flag per indicare se il monitoraggio è attivo
        self.monitoring = False
        
        # Thread per il monitoraggio in background
        self.monitor_thread = None
        
        self.logger.info("Monitor di sistema inizializzato")
    
    def _configure(self) -> None:
        """Configura il monitor in base alle impostazioni del Core."""
        # Intervallo di aggiornamento (in secondi)
        self.update_interval = self.core.get_config("app.update_interval", 5)
        
        # Opzioni di monitoraggio
        self.monitor_cpu = self.core.get_config("monitoring.cpu", True)
        self.monitor_memory = self.core.get_config("monitoring.memory", True)
        self.monitor_disk = self.core.get_config("monitoring.disk", True)
        self.monitor_network = self.core.get_config("monitoring.network", False)
        self.monitor_gpu = self.core.get_config("monitoring.gpu", False)
        
        self.logger.debug(f"Monitor configurato: intervallo={self.update_interval}s")
    
    def start_monitoring(self) -> None:
        """Avvia il monitoraggio in background."""
        if self.monitoring:
            self.logger.warning("Il monitoraggio è già attivo")
            return
        
        self.monitoring = True
        self.monitor_thread = threading.Thread(target=self._monitoring_loop, daemon=True)
        self.monitor_thread.start()
        self.logger.info("Monitoraggio avviato")
    
    def stop_monitoring(self) -> None:
        """Ferma il monitoraggio in background."""
        if not self.monitoring:
            self.logger.warning("Il monitoraggio non è attivo")
            return
        
        self.monitoring = False
        if self.monitor_thread:
            self.monitor_thread.join(timeout=2.0)
        self.logger.info("Monitoraggio fermato")
    
    def register_callback(self, callback: Callable) -> None:
        """
        Registra una funzione di callback da chiamare quando i dati cambiano.
        
        Args:
            callback: Funzione da chiamare senza argomenti.
        """
        if callback not in self.callbacks:
            self.callbacks.append(callback)
            self.logger.debug("Callback registrata")
    
    def unregister_callback(self, callback: Callable) -> None:
        """
        Rimuove una funzione di callback.
        
        Args:
            callback: Funzione da rimuovere.
        """
        if callback in self.callbacks:
            self.callbacks.remove(callback)
            self.logger.debug("Callback rimossa")
    
    def _notify_callbacks(self) -> None:
        """Notifica tutte le callback registrate."""
        for callback in self.callbacks:
            try:
                callback()
            except Exception as e:
                self.logger.error(f"Errore durante la notifica della callback: {e}")
    
    def _monitoring_loop(self) -> None:
        """Loop di monitoraggio in background."""
        while self.monitoring:
            try:
                # Aggiorna le risorse
                self.update()
                
                # Notifica gli osservatori
                self._notify_callbacks()
                
                # Attendi l'intervallo configurato
                time.sleep(self.update_interval)
            except Exception as e:
                self.logger.error(f"Errore nel loop di monitoraggio: {e}")
                time.sleep(5)  # Attendi un po' di più in caso di errore
    
    def update(self) -> None:
        """Aggiorna le informazioni sulle risorse."""
        now = time.time()
        
        try:
            # Aggiorna CPU
            if self.monitor_cpu:
                cpu_usage = self.core.get_resource_value(ResourceType.CPU)
                self._update_history("cpu", now, cpu_usage)
            
            # Aggiorna memoria
            if self.monitor_memory:
                memory_usage = self.core.get_resource_value(ResourceType.MEMORY)
                self._update_history("memory", now, memory_usage)
            
            # Aggiorna disco
            if self.monitor_disk:
                disk_usage = self.core.get_resource_value(ResourceType.DISK)
                self._update_history("disk", now, disk_usage)
            
            # Aggiorna rete
            if self.monitor_network:
                network_usage = self.core.get_resource_value(ResourceType.NETWORK)
                self._update_history("network", now, network_usage)
            
            # Aggiorna GPU
            if self.monitor_gpu:
                gpu_usage = self.core.get_resource_value(ResourceType.GPU)
                self._update_history("gpu", now, gpu_usage)
            
            self.logger.debug("Risorse aggiornate")
        except Exception as e:
            self.logger.error(f"Errore durante l'aggiornamento delle risorse: {e}")
    
    def _update_history(self, resource_type: str, timestamp: float, value: float) -> None:
        """
        Aggiorna lo storico di una risorsa.
        
        Args:
            resource_type: Tipo di risorsa.
            timestamp: Timestamp della misurazione.
            value: Valore misurato.
        """
        # Aggiungi la nuova misurazione
        self.history[resource_type].append((timestamp, value))
        
        # Mantieni la dimensione massima
        if len(self.history[resource_type]) > self.history_size:
            self.history[resource_type].pop(0)
    
    def get_history(self, resource_type: str) -> List[Tuple[float, float]]:
        """
        Ottiene lo storico di una risorsa.
        
        Args:
            resource_type: Tipo di risorsa.
            
        Returns:
            Lista di tuple (timestamp, valore).
        """
        return self.history.get(resource_type, [])
    
    def get_system_resources(self) -> SystemResources:
        """
        Ottiene un'istantanea delle risorse di sistema.
        
        Returns:
            Oggetto SystemResources con i dati correnti.
        """
        # Ottieni i valori più recenti dallo storico
        cpu_value = self._get_latest_value("cpu")
        memory_value = self._get_latest_value("memory")
        disk_value = self._get_latest_value("disk")
        network_value = self._get_latest_value("network")
        gpu_value = self._get_latest_value("gpu")
        
        # Ottieni informazioni dettagliate sulle risorse
        memory_details = self.core.get_memory_details()
        disk_details = self.core.get_disk_details()
        
        # Ottieni temperature e uptime
        cpu_temp = self.core.get_cpu_temperature()
        uptime = self.core.get_system_uptime()
        
        # Crea e restituisci l'oggetto
        return SystemResources(
            cpu_usage=cpu_value,
            memory_usage=memory_value,
            disk_usage=disk_value,
            memory_used=memory_details.get("used", 0.0),
            memory_total=memory_details.get("total", 1.0),
            disk_used=disk_details.get("used", 0.0),
            disk_total=disk_details.get("total", 1.0),
            network_usage=network_value,
            gpu_usage=gpu_value,
            cpu_temp=cpu_temp,
            uptime=uptime
        )
    
    def _get_latest_value(self, resource_type: str) -> float:
        """
        Ottiene il valore più recente per una risorsa.
        
        Args:
            resource_type: Tipo di risorsa.
            
        Returns:
            Valore più recente o 0.0 se non disponibile.
        """
        history = self.history.get(resource_type, [])
        if not history:
            return 0.0
        return history[-1][1]  # Restituisce l'ultimo valore
    
    def reset_history(self) -> None:
        """Resetta lo storico di tutte le risorse."""
        for resource_type in self.history:
            self.history[resource_type] = []
        self.logger.info("Storico resettato")