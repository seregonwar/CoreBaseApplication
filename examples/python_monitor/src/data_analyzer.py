#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Modulo per l'analisi dei dati di monitoraggio.
Fornisce funzionalità per l'elaborazione e l'analisi dei dati raccolti dal sistema di monitoraggio.
"""

import logging
import statistics
import time
from typing import Dict, List, Any, Optional, Tuple
from datetime import datetime, timedelta

from core_interface import CoreInterface

class DataAnalyzer:
    """Classe per l'analisi dei dati di monitoraggio."""
    
    def __init__(self, core: CoreInterface):
        """
        Inizializza l'analizzatore di dati.
        
        Args:
            core: Interfaccia con il Core.
        """
        self.logger = logging.getLogger(__name__)
        self.core = core
        
        # Configura l'analizzatore in base alle impostazioni
        self._configure()
        
        self.logger.info("Analizzatore di dati inizializzato")
    
    def _configure(self) -> None:
        """Configura l'analizzatore in base alle impostazioni del Core."""
        # Periodo di analisi predefinito (in minuti)
        self.default_analysis_period = self.core.get_config("app.analysis_period", 5)
        
        # Soglie di allarme predefinite
        self.default_thresholds = {
            "cpu": self.core.get_config("alerts.cpu_threshold", 80),
            "memory": self.core.get_config("alerts.memory_threshold", 85),
            "disk": self.core.get_config("alerts.disk_threshold", 90),
            "network": self.core.get_config("alerts.network_threshold", 80),
            "gpu": self.core.get_config("alerts.gpu_threshold", 75)
        }
        
    def analyze_resource_usage(self, history: List[Tuple[float, float]], 
                              period_minutes: Optional[int] = None) -> Dict[str, Any]:
        """
        Analizza l'utilizzo di una risorsa nel periodo specificato.
        
        Args:
            history: Storico della risorsa (lista di tuple timestamp, valore).
            period_minutes: Periodo di analisi in minuti. Se None, usa il periodo predefinito.
            
        Returns:
            Dizionario con i risultati dell'analisi.
        """
        if not history:
            return {
                "min": 0.0,
                "max": 0.0,
                "avg": 0.0,
                "median": 0.0,
                "std_dev": 0.0,
                "current": 0.0,
                "samples": 0
            }
            
        # Determina il periodo di analisi
        period = period_minutes or self.default_analysis_period
        
        # Filtra lo storico in base al periodo
        cutoff_time = time.time() - (period * 60)
        filtered_history = [(ts, val) for ts, val in history if ts >= cutoff_time]
        
        # Estrai i valori
        values = [val for _, val in filtered_history]
        
        if not values:
            return {
                "min": 0.0,
                "max": 0.0,
                "avg": 0.0,
                "median": 0.0,
                "std_dev": 0.0,
                "current": 0.0,
                "samples": 0
            }
            
        # Calcola le statistiche
        result = {
            "min": min(values),
            "max": max(values),
            "avg": statistics.mean(values),
            "median": statistics.median(values),
            "std_dev": statistics.stdev(values) if len(values) > 1 else 0.0,
            "current": values[-1],
            "samples": len(values)
        }
        
        self.logger.debug(f"Analisi completata: {result}")
        return result
    
    def predict_resource_usage(self, history: List[Tuple[float, float]], 
                              minutes_ahead: int = 30) -> Dict[str, Any]:
        """
        Predice l'utilizzo futuro di una risorsa.
        
        Args:
            history: Storico della risorsa (lista di tuple timestamp, valore).
            minutes_ahead: Minuti nel futuro per la previsione.
            
        Returns:
            Dizionario con i risultati della previsione.
        """
        if len(history) < 2:
            return {
                "predicted": None,
                "confidence": 0.0,
                "trend": "stable"
            }
            
        # Estrai timestamp e valori
        timestamps = [ts for ts, _ in history]
        values = [val for _, val in history]
        
        # Calcola la variazione media
        diffs = [values[i] - values[i-1] for i in range(1, len(values))]
        avg_diff = statistics.mean(diffs)
        
        # Predici il valore futuro
        current_value = values[-1]
        predicted_value = current_value + (avg_diff * (minutes_ahead / 5))  # Assumendo campioni ogni 5 minuti
        
        # Limita il valore predetto tra 0 e 100
        predicted_value = max(0, min(100, predicted_value))
        
        # Determina il trend
        if abs(avg_diff) < 0.5:
            trend = "stable"
        elif avg_diff > 0:
            trend = "increasing"
        else:
            trend = "decreasing"
            
        # Calcola un indice di confidenza semplice
        # Più campioni = maggiore confidenza, minore deviazione standard = maggiore confidenza
        std_dev = statistics.stdev(values) if len(values) > 1 else float('inf')
        samples_factor = min(1.0, len(values) / 20)  # Max confidenza con 20+ campioni
        std_dev_factor = max(0.0, 1.0 - (std_dev / 20))  # Max confidenza con deviazione standard bassa
        
        confidence = samples_factor * std_dev_factor * 100
        
        result = {
            "predicted": predicted_value,
            "confidence": confidence,
            "trend": trend
        }
        
        self.logger.debug(f"Previsione completata: {result}")
        return result
    
    def _detect_anomalies_in_history(self, history: List[Tuple[float, float]], 
                       threshold_multiplier: float = 2.0) -> List[Tuple[float, float]]:
        """
        Rileva anomalie nello storico di una risorsa.
        
        Args:
            history: Storico della risorsa (lista di tuple timestamp, valore).
            threshold_multiplier: Moltiplicatore della deviazione standard per la soglia di anomalia.
            
        Returns:
            Lista di tuple (timestamp, valore) delle anomalie rilevate.
        """
        if len(history) < 10:  # Serve un minimo di campioni per avere un'analisi significativa
            return []
            
        # Estrai i valori
        values = [val for _, val in history]
        
        # Calcola media e deviazione standard
        mean = statistics.mean(values)
        std_dev = statistics.stdev(values)
        
        # Soglia per considerare un valore anomalo
        threshold = std_dev * threshold_multiplier
        
        # Trova le anomalie
        anomalies = []
        for ts, val in history:
            if abs(val - mean) > threshold:
                anomalies.append((ts, val))
                
        self.logger.debug(f"Rilevate {len(anomalies)} anomalie")
        return anomalies
    
    def detect_anomalies(self) -> List[str]:
        """
        Rileva anomalie nel sistema e restituisce una lista di messaggi descrittivi.
        
        Returns:
            Lista di stringhe che descrivono le anomalie rilevate.
        """
        self.logger.debug("Rilevamento anomalie in corso")
        
        anomalies = []
        
        # Ottiene gli storici delle risorse
        cpu_history = self.core.get_resource_history("cpu")
        memory_history = self.core.get_resource_history("memory")
        disk_history = self.core.get_resource_history("disk")
        
        # Rileva anomalie per ciascuna risorsa
        cpu_anomalies = self._detect_anomalies_in_history(cpu_history)
        if cpu_anomalies:
            anomalies.append(f"Anomalie CPU rilevate: {len(cpu_anomalies)} picchi di utilizzo")
        
        memory_anomalies = self._detect_anomalies_in_history(memory_history)
        if memory_anomalies:
            anomalies.append(f"Anomalie memoria rilevate: {len(memory_anomalies)} picchi di utilizzo")
        
        disk_anomalies = self._detect_anomalies_in_history(disk_history)
        if disk_anomalies:
            anomalies.append(f"Anomalie disco rilevate: {len(disk_anomalies)} picchi di utilizzo")
        
        return anomalies
    
    def generate_report(self, system_monitor) -> Dict[str, Any]:
        """
        Genera un report completo sul sistema.
        
        Args:
            system_monitor: Istanza del monitor di sistema.
            
        Returns:
            Dizionario con il report completo.
        """
        report = {
            "timestamp": datetime.now().isoformat(),
            "resources": {},
            "anomalies": {},
            "predictions": {}
        }
        
        # Analizza ogni tipo di risorsa
        for resource_type in ["cpu", "memory", "disk", "network", "gpu"]:
            history = system_monitor.get_history(resource_type)
            
            # Analisi dell'utilizzo
            report["resources"][resource_type] = self.analyze_resource_usage(history)
            
            # Previsione dell'utilizzo
            report["predictions"][resource_type] = self.predict_resource_usage(history)
            
            # Rilevamento anomalie
            anomalies = self._detect_anomalies_in_history(history)
            if anomalies:
                report["anomalies"][resource_type] = [
                    {"timestamp": datetime.fromtimestamp(ts).isoformat(), "value": val}
                    for ts, val in anomalies
                ]
            else:
                report["anomalies"][resource_type] = []
                
        self.logger.info("Report generato con successo")
        return report
    
    def get_resource_usage_statistics(self) -> Dict[str, Dict[str, float]]:
        """
        Ottiene le statistiche di utilizzo per tutte le risorse.
        
        Returns:
            Dizionario con le statistiche per ciascuna risorsa (cpu, memory, disk).
        """
        self.logger.debug("Recupero statistiche di utilizzo risorse")
        
        # Ottiene i dati memorizzati dalla core interface
        cpu_history = self.core.get_resource_history("cpu")
        memory_history = self.core.get_resource_history("memory")
        disk_history = self.core.get_resource_history("disk")
        
        # Analizza ciascuna risorsa
        cpu_stats = self.analyze_resource_usage(cpu_history)
        memory_stats = self.analyze_resource_usage(memory_history)
        disk_stats = self.analyze_resource_usage(disk_history)
        
        # Formato semplificato per l'UI
        return {
            "cpu": {
                "average": cpu_stats["avg"],
                "max": cpu_stats["max"],
                "min": cpu_stats["min"]
            },
            "memory": {
                "average": memory_stats["avg"],
                "max": memory_stats["max"],
                "min": memory_stats["min"]
            },
            "disk": {
                "average": disk_stats["avg"],
                "max": disk_stats["max"],
                "min": disk_stats["min"]
            }
        }
    
    def execute_analysis(self, code: str, context: Dict[str, Any] = None) -> Tuple[bool, Any]:
        """
        Esegue un'analisi personalizzata con codice Python.
        
        Args:
            code: Codice Python da eseguire.
            context: Contesto da passare al codice (dizionario di variabili).
            
        Returns:
            Tupla (successo, risultato).
        """
        self.logger.info("Esecuzione di un'analisi personalizzata")
        
        # Prepara il contesto
        exec_context = {
            "statistics": statistics,
            "datetime": datetime,
            "timedelta": timedelta
        }
        
        if context:
            exec_context.update(context)
            
        # Utilizza il Core per eseguire il codice
        return self.core.execute_python_code(code)
