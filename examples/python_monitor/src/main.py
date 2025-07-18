#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Applicazione principale per il monitoraggio delle risorse di sistema.
Fornisce funzionalità di monitoraggio, analisi e avvisi per le risorse del sistema.
"""

import sys
import os
import logging
import json
import time
from pathlib import Path

# Importazione dei moduli
from core_interface import CoreInterface, ResourceType
from system_monitor import SystemMonitor
from data_analyzer import DataAnalyzer
from alert_manager import AlertManager
from console_ui import ConsoleUI

def setup_logging(config):
    """
    Configura il sistema di logging.
    
    Args:
        config: Configurazione dell'applicazione.
    """
    log_level_str = config.get("log_level", "INFO").upper()
    log_level = getattr(logging, log_level_str, logging.INFO)
    
    log_dir = Path("logs")
    log_dir.mkdir(exist_ok=True)
    
    logging.basicConfig(
        level=log_level,
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        handlers=[
            logging.FileHandler(log_dir / "python_monitor.log"),
            logging.StreamHandler()
        ]
    )
    
    logging.info(f"Logging configurato al livello {log_level_str}")

def load_config():
    """
    Carica la configurazione dell'applicazione.
    
    Returns:
        Configurazione caricata dal file.
    """
    config_dir = Path("config")
    
    # Controlla se esiste la directory config
    if not config_dir.exists():
        config_dir.mkdir(exist_ok=True)
    
    config_file = config_dir / "default.json"
    
    # Se il file di configurazione non esiste, crea uno predefinito
    if not config_file.exists():
        default_config = {
            "app": {
                "name": "Python Resource Monitor",
                "version": "1.0.0",
                "log_level": "INFO",
                "update_interval": 5
            },
            "monitoring": {
                "cpu": True,
                "memory": True,
                "disk": True,
                "network": False,
                "gpu": False
            },
            "alerts": {
                "cpu_threshold": 80,
                "memory_threshold": 85,
                "disk_threshold": 90,
                "network_threshold": 80,
                "gpu_threshold": 75,
                "email_notifications": False,
                "email_to": ""
            },
            "ui": {
                "refresh_rate": 1.0,
                "show_graphs": True,
                "dark_mode": True
            }
        }
        
        with open(config_file, 'w') as f:
            json.dump(default_config, f, indent=4)
        
        logging.info(f"Creato file di configurazione predefinito: {config_file}")
    
    # Carica la configurazione
    with open(config_file, 'r') as f:
        config = json.load(f)
    
    logging.info(f"Configurazione caricata da {config_file}")
    return config

def main():
    """Punto di ingresso principale dell'applicazione."""
    try:
        # Carica la configurazione
        config = load_config()
        
        # Configura il logging
        setup_logging(config.get("app", {}))
        
        # Crea l'interfaccia al Core
        core = CoreInterface()
        core.set_config(config)
        
        # Avvia i componenti principali
        logging.info("Avvio dei componenti dell'applicazione...")
        
        # Crea il monitor di sistema
        system_monitor = SystemMonitor(core)
        
        # Crea l'analizzatore di dati
        data_analyzer = DataAnalyzer(core)
        
        # Crea il gestore degli allarmi
        alert_manager = AlertManager(core, system_monitor)
        
        # Avvia il monitoraggio in background
        system_monitor.start_monitoring()
        
        # Crea l'interfaccia utente
        ui = ConsoleUI(system_monitor, data_analyzer, alert_manager)
        
        # Avvia l'interfaccia utente
        ui.start()
        
    except KeyboardInterrupt:
        logging.info("Applicazione terminata dall'utente")
    except Exception as e:
        logging.error(f"Errore nell'applicazione: {e}", exc_info=True)
    finally:
        # Arresta il monitoraggio se è stato avviato
        if 'system_monitor' in locals():
            system_monitor.stop_monitoring()
        
        logging.info("Applicazione terminata")

if __name__ == "__main__":
    main() 