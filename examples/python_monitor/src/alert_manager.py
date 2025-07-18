#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Modulo per la gestione degli allarmi e delle notifiche.
Monitora le risorse e genera allarmi quando vengono superate le soglie.
"""

import logging
import time
import threading
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from datetime import datetime
from typing import Dict, List, Callable, Any, Optional
from dataclasses import dataclass

from core_interface import CoreInterface
from system_monitor import SystemMonitor

@dataclass
class Alert:
    """Classe per rappresentare un allarme."""
    timestamp: datetime
    resource_type: str
    value: float
    threshold: float
    message: str
    is_critical: bool = False
    
    def __post_init__(self):
        """Inizializzazione dopo la creazione dell'istanza."""
        # Calcola la criticità in base alla distanza dalla soglia
        overrun_percentage = (self.value - self.threshold) / self.threshold * 100
        self.is_critical = overrun_percentage > 20  # Critico se supera la soglia del 20%

class AlertManager:
    """Classe per la gestione degli allarmi."""
    
    def __init__(self, core: CoreInterface, system_monitor: SystemMonitor):
        """
        Inizializza il gestore degli allarmi.
        
        Args:
            core: Interfaccia con il Core.
            system_monitor: Monitor di sistema.
        """
        self.logger = logging.getLogger(__name__)
        self.core = core
        self.system_monitor = system_monitor
        
        # Lista di allarmi attivi
        self.active_alerts: List[Alert] = []
        
        # Soglie di allarme
        self.thresholds = {
            "cpu": 80.0,  # Percentuale
            "memory": 85.0,  # Percentuale
            "disk": 90.0,  # Percentuale
            "network": 80.0,  # Percentuale
            "gpu": 75.0  # Percentuale
        }
        
        # Lista di callback per le notifiche
        self.callbacks: List[Callable[[Alert], None]] = []
        
        # Registro degli allarmi inviati per evitare duplicati
        self.sent_alerts: Dict[str, float] = {}
        
        # Periodo di cooldown per gli allarmi (in secondi)
        self.alert_cooldown = 300  # 5 minuti
        
        # Crea un thread separato per il controllo periodico
        self.check_thread = None
        self.running = False
        
        # Configura il gestore
        self._configure()
        
        # Registra il gestore come osservatore del monitor
        self.system_monitor.register_callback(self._check_resources)
        
        self.logger.info("Gestore degli allarmi inizializzato")
        
    def _configure(self) -> None:
        """Configura il gestore in base alle impostazioni del Core."""
        # Carica le soglie di allarme dalla configurazione
        self.thresholds["cpu"] = self.core.get_config("alerts.cpu_threshold", 80.0)
        self.thresholds["memory"] = self.core.get_config("alerts.memory_threshold", 85.0)
        self.thresholds["disk"] = self.core.get_config("alerts.disk_threshold", 90.0)
        self.thresholds["network"] = self.core.get_config("alerts.network_threshold", 80.0)
        self.thresholds["gpu"] = self.core.get_config("alerts.gpu_threshold", 75.0)
        
        # Configura le notifiche email
        self.email_notifications = self.core.get_config("alerts.email_notifications", False)
        self.email_to = self.core.get_config("alerts.email_to", "")
        self.email_from = self.core.get_config("alerts.email_from", "monitor@example.com")
        self.email_server = self.core.get_config("alerts.email_server", "smtp.example.com")
        
        self.logger.debug(f"Gestore configurato con soglie: {self.thresholds}")
    
    def start(self) -> None:
        """Avvia il controllo periodico degli allarmi."""
        if self.running:
            self.logger.warning("Il gestore degli allarmi è già in esecuzione")
            return
        
        self.running = True
        self.check_thread = threading.Thread(target=self._check_loop, daemon=True)
        self.check_thread.start()
        self.logger.info("Controllo degli allarmi avviato")
    
    def stop(self) -> None:
        """Ferma il controllo periodico degli allarmi."""
        if not self.running:
            self.logger.warning("Il gestore degli allarmi non è in esecuzione")
            return
        
        self.running = False
        if self.check_thread:
            self.check_thread.join(timeout=2.0)
        self.logger.info("Controllo degli allarmi fermato")
    
    def _check_loop(self) -> None:
        """Loop di controllo degli allarmi in background."""
        while self.running:
            try:
                # Controlla le risorse
                self._check_resources()
                
                # Attendi prima del prossimo controllo
                time.sleep(60)  # Controlla ogni minuto
            except Exception as e:
                self.logger.error(f"Errore nel loop di controllo degli allarmi: {e}")
                time.sleep(60)  # Attendi un minuto in caso di errore
    
    def _check_resources(self) -> None:
        """Controlla le risorse e genera allarmi se necessario."""
        try:
            # Ottieni le risorse attuali
            resources = self.system_monitor.get_system_resources()
            
            # Controlla la CPU
            self._check_resource("cpu", resources.cpu_usage)
            
            # Controlla la memoria
            self._check_resource("memory", resources.memory_usage)
            
            # Controlla il disco
            self._check_resource("disk", resources.disk_usage)
            
            # Controlla la rete se disponibile
            if resources.network_usage is not None:
                self._check_resource("network", resources.network_usage)
            
            # Controlla la GPU se disponibile
            if resources.gpu_usage is not None:
                self._check_resource("gpu", resources.gpu_usage)
            
            # Controlla se ci sono allarmi da risolvere
            self._resolve_alerts(resources)
            
        except Exception as e:
            self.logger.error(f"Errore durante il controllo delle risorse: {e}")
    
    def _check_resource(self, resource_type: str, value: float) -> None:
        """
        Controlla una risorsa e genera un allarme se supera la soglia.
        
        Args:
            resource_type: Tipo di risorsa.
            value: Valore attuale della risorsa.
        """
        threshold = self.thresholds.get(resource_type, 100.0)
        
        # Controlla se il valore supera la soglia
        if value > threshold:
            # Controlla se è stato già inviato un allarme recentemente per questa risorsa
            now = time.time()
            last_alert_time = self.sent_alerts.get(resource_type, 0)
            
            if now - last_alert_time > self.alert_cooldown:
                # Crea un nuovo allarme
                message = f"L'utilizzo di {resource_type} ({value:.1f}%) ha superato la soglia ({threshold:.1f}%)"
                alert = Alert(
                    timestamp=datetime.now(),
                    resource_type=resource_type,
                    value=value,
                    threshold=threshold,
                    message=message
                )
                
                # Aggiunge l'allarme alla lista degli allarmi attivi
                self.active_alerts.append(alert)
                
                # Registra l'orario dell'ultimo allarme
                self.sent_alerts[resource_type] = now
                
                # Notifica i callback registrati
                self._notify_callbacks(alert)
                
                # Invia notifiche email se configurate
                if self.email_notifications and self.email_to:
                    self._send_email_notification(alert)
                
                self.logger.warning(f"Generato allarme per {resource_type}: {message}")
    
    def _resolve_alerts(self, resources) -> None:
        """
        Risolve gli allarmi quando i valori tornano sotto la soglia.
        
        Args:
            resources: Risorse di sistema attuali.
        """
        # Crea una copia della lista per evitare problemi durante l'iterazione
        current_alerts = self.active_alerts.copy()
        
        for alert in current_alerts:
            value = getattr(resources, f"{alert.resource_type}_usage", None)
            
            # Se il valore è tornato sotto la soglia, rimuovi l'allarme
            if value is not None and value < alert.threshold:
                self.active_alerts.remove(alert)
                self.logger.info(f"Risolto allarme per {alert.resource_type}")
    
    def _notify_callbacks(self, alert: Alert) -> None:
        """
        Notifica tutti i callback registrati.
        
        Args:
            alert: Allarme da notificare.
        """
        for callback in self.callbacks:
            try:
                callback(alert)
            except Exception as e:
                self.logger.error(f"Errore durante la notifica del callback: {e}")
            
    def _send_email_notification(self, alert: Alert) -> None:
        """
        Invia una notifica email per un allarme.
        
        Args:
            alert: Allarme da notificare.
        """
        try:
            # Crea il messaggio
            msg = MIMEMultipart()
            msg['From'] = self.email_from
            msg['To'] = self.email_to
            msg['Subject'] = f"[ALLARME] {alert.resource_type.upper()} - Sistema di monitoraggio"
            
            # Costruisci il corpo del messaggio
            body = f"""
            <html>
            <body>
                <h2>Allarme del sistema di monitoraggio</h2>
                <p><strong>Risorsa:</strong> {alert.resource_type}</p>
                <p><strong>Valore:</strong> {alert.value:.1f}%</p>
                <p><strong>Soglia:</strong> {alert.threshold:.1f}%</p>
                <p><strong>Data/ora:</strong> {alert.timestamp.strftime('%Y-%m-%d %H:%M:%S')}</p>
                <p><strong>Messaggio:</strong> {alert.message}</p>
                <p><strong>Criticità:</strong> {'CRITICO' if alert.is_critical else 'Avviso'}</p>
            </body>
            </html>
            """
            
            # Allega il corpo HTML
            msg.attach(MIMEText(body, 'html'))
            
            # Invia l'email
            with smtplib.SMTP(self.email_server) as server:
                server.send_message(msg)
                
            self.logger.info(f"Notifica email inviata per {alert.resource_type}")
        except Exception as e:
            self.logger.error(f"Errore nell'invio della notifica email: {e}")
        
    def register_callback(self, callback: Callable[[Alert], None]) -> None:
        """
        Registra un callback da chiamare quando viene generato un allarme.
        
        Args:
            callback: Funzione da chiamare con l'allarme come argomento.
        """
        if callback not in self.callbacks:
            self.callbacks.append(callback)
            self.logger.debug("Callback registrata")
    
    def unregister_callback(self, callback: Callable[[Alert], None]) -> None:
        """
        Rimuove un callback registrato.
        
        Args:
            callback: Funzione da rimuovere.
        """
        if callback in self.callbacks:
            self.callbacks.remove(callback)
            self.logger.debug("Callback rimossa")
    
    def get_active_alerts(self) -> List[Alert]:
        """
        Ottiene la lista degli allarmi attivi.
        
        Returns:
            Lista degli allarmi attivi.
        """
        return self.active_alerts.copy()
        
    def get_thresholds(self) -> Dict[str, float]:
        """
        Ottiene le soglie di allarme attuali.
            
        Returns:
            Dizionario con le soglie per ciascuna risorsa.
        """
        return self.thresholds.copy()
    
    def set_threshold(self, resource_type: str, value: float) -> bool:
        """
        Imposta una soglia di allarme.
        
        Args:
            resource_type: Tipo di risorsa (cpu, memory, disk, network, gpu).
            value: Nuovo valore della soglia.
            
        Returns:
            True se la soglia è stata impostata con successo, False altrimenti.
        """
        if resource_type not in self.thresholds:
            self.logger.warning(f"Tipo di risorsa non valido: {resource_type}")
            return False
            
        if not 0 <= value <= 100:
            self.logger.warning(f"Valore di soglia non valido: {value}")
            return False
        
        # Aggiorna la soglia
        self.thresholds[resource_type] = value
        
        # Aggiorna anche la configurazione nel Core
        config_key = f"alerts.{resource_type}_threshold"
        self.core.set_config(config_key, value)
        
        self.logger.info(f"Soglia per {resource_type} impostata a {value}%")
        return True
    
    def clear_alerts(self) -> None:
        """Cancella tutti gli allarmi attivi."""
        self.active_alerts.clear()
        self.logger.info("Tutti gli allarmi sono stati cancellati")