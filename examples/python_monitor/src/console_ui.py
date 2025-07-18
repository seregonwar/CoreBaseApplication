#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Modulo per l'interfaccia utente a console."""

import curses
import time
import threading
from typing import Dict, Any, List, Optional

from system_monitor import SystemMonitor
from data_analyzer import DataAnalyzer
from alert_manager import AlertManager

class ConsoleUI:
    """Interfaccia utente a console per il sistema di monitoraggio."""
    
    def __init__(self, monitor: SystemMonitor, analyzer: DataAnalyzer, alert_manager: AlertManager):
        """
        Inizializza l'interfaccia utente.
        
        Args:
            monitor: Monitor di sistema.
            analyzer: Analizzatore di dati.
            alert_manager: Gestore degli allarmi.
        """
        self.monitor = monitor
        self.analyzer = analyzer
        self.alert_manager = alert_manager
        self.running = False
        self.refresh_rate = 1.0  # Secondi
        
    def start(self):
        """Avvia l'interfaccia utente."""
        self.running = True
        curses.wrapper(self._main_loop)
        
    def stop(self):
        """Ferma l'interfaccia utente."""
        self.running = False
        
    def _main_loop(self, stdscr):
        """
        Loop principale dell'interfaccia.
        
        Args:
            stdscr: Schermo curses.
        """
        curses.curs_set(0)  # Nasconde il cursore
        stdscr.timeout(100)  # Non blocca l'input
        
        # Inizializza i colori
        if curses.has_colors():
            curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)  # Normale
            curses.init_pair(2, curses.COLOR_YELLOW, curses.COLOR_BLACK)  # Avviso
            curses.init_pair(3, curses.COLOR_RED, curses.COLOR_BLACK)     # Critico
            curses.init_pair(4, curses.COLOR_CYAN, curses.COLOR_BLACK)    # Titolo
            
        # Menu principale
        menu_items = [
            "1. Visualizza risorse di sistema",
            "2. Visualizza statistiche",
            "3. Visualizza allarmi",
            "4. Configura soglie",
            "5. Esci"
        ]
        
        current_view = None
        
        while self.running:
            stdscr.clear()
            h, w = stdscr.getmaxyx()
            
            # Intestazione
            title = "Sistema di monitoraggio delle risorse"
            stdscr.addstr(1, (w - len(title)) // 2, title, curses.color_pair(4) | curses.A_BOLD)
            
            # Se non c'è una vista attiva, mostra il menu
            if current_view is None:
                # Menu
                stdscr.addstr(3, 2, "Menu principale:", curses.A_BOLD)
                for i, item in enumerate(menu_items):
                    stdscr.addstr(5 + i, 4, item)
                
                # Istruzioni
                stdscr.addstr(h - 2, 2, "Seleziona un'opzione (1-5): ")
                
                # Gestione input
                try:
                    key = stdscr.getch()
                    if key != -1:
                        if key == ord('1'):
                            current_view = 'resources'
                        elif key == ord('2'):
                            current_view = 'statistics'
                        elif key == ord('3'):
                            current_view = 'alerts'
                        elif key == ord('4'):
                            current_view = 'thresholds'
                        elif key == ord('5') or key == ord('q'):
                            self.running = False
                except Exception as e:
                    stdscr.addstr(h - 1, 2, f"Errore: {str(e)}", curses.color_pair(3))
            else:
                # Gestisci la vista corrente
                if current_view == 'resources':
                    self._display_resources(stdscr)
                elif current_view == 'statistics':
                    self._display_statistics(stdscr)
                elif current_view == 'alerts':
                    self._display_alerts(stdscr)
                elif current_view == 'thresholds':
                    self._display_thresholds(stdscr)
                
                # Mostra l'opzione per tornare al menu
                stdscr.addstr(h - 2, 2, "Premi 'b' per tornare al menu principale")
                
                # Gestione input
                try:
                    key = stdscr.getch()
                    if key == ord('b'):
                        current_view = None
                except Exception as e:
                    stdscr.addstr(h - 1, 2, f"Errore: {str(e)}", curses.color_pair(3))
            
            stdscr.refresh()
            time.sleep(0.1)  # Piccola pausa per ridurre l'utilizzo della CPU
    
    def _display_resources(self, stdscr):
        """
        Visualizza le risorse di sistema.
        
        Args:
            stdscr: Schermo curses.
        """
        h, w = stdscr.getmaxyx()
        
        # Intestazione
        stdscr.addstr(1, 2, "Risorse di sistema", curses.color_pair(4) | curses.A_BOLD)
        stdscr.addstr(2, 2, "Aggiornamento in tempo reale", curses.A_ITALIC)
        
        # Ottieni le risorse
        resources = self.monitor.get_system_resources()
        
        # CPU
        stdscr.addstr(4, 2, "CPU:", curses.A_BOLD)
        cpu_color = self._get_color_for_value(resources.cpu_usage)
        cpu_bar = self._create_bar(resources.cpu_usage, 40)
        stdscr.addstr(4, 7, f"{resources.cpu_usage:.1f}% ", cpu_color)
        stdscr.addstr(4, 15, cpu_bar, cpu_color)
        
        # Memoria
        stdscr.addstr(6, 2, "Memoria:", curses.A_BOLD)
        mem_color = self._get_color_for_value(resources.memory_usage)
        mem_bar = self._create_bar(resources.memory_usage, 40)
        stdscr.addstr(6, 11, f"{resources.memory_usage:.1f}% ", mem_color)
        stdscr.addstr(6, 19, mem_bar, mem_color)
        stdscr.addstr(7, 11, f"{resources.memory_used:.1f} GB / {resources.memory_total:.1f} GB")
        
        # Disco
        stdscr.addstr(9, 2, "Disco:", curses.A_BOLD)
        disk_color = self._get_color_for_value(resources.disk_usage)
        disk_bar = self._create_bar(resources.disk_usage, 40)
        stdscr.addstr(9, 9, f"{resources.disk_usage:.1f}% ", disk_color)
        stdscr.addstr(9, 17, disk_bar, disk_color)
        stdscr.addstr(10, 9, f"{resources.disk_used:.1f} GB / {resources.disk_total:.1f} GB")
        
        # Info aggiuntive
        stdscr.addstr(12, 2, f"Temperatura CPU: {resources.cpu_temp:.1f}°C" if resources.cpu_temp else "Temperatura CPU: N/D")
        stdscr.addstr(13, 2, f"Uptime sistema: {resources.uptime}")
        
        # Legenda
        stdscr.addstr(h - 4, 2, "Legenda:", curses.A_BOLD)
        stdscr.addstr(h - 3, 4, "■", curses.color_pair(1))
        stdscr.addstr(h - 3, 6, "Normale")
        stdscr.addstr(h - 3, 16, "■", curses.color_pair(2))
        stdscr.addstr(h - 3, 18, "Attenzione")
        stdscr.addstr(h - 3, 30, "■", curses.color_pair(3))
        stdscr.addstr(h - 3, 32, "Critico")
    
    def _display_statistics(self, stdscr):
        """
        Visualizza le statistiche delle risorse.
        
        Args:
            stdscr: Schermo curses.
        """
        h, w = stdscr.getmaxyx()
        
        # Intestazione
        stdscr.addstr(1, 2, "Statistiche di utilizzo delle risorse", curses.color_pair(4) | curses.A_BOLD)
        
        # Ottieni le statistiche
        stats = self.analyzer.get_resource_usage_statistics()
        
        # CPU
        stdscr.addstr(3, 2, "CPU:", curses.A_BOLD)
        stdscr.addstr(4, 4, f"Media: {stats['cpu']['average']:.1f}%")
        stdscr.addstr(5, 4, f"Massimo: {stats['cpu']['max']:.1f}%")
        stdscr.addstr(6, 4, f"Minimo: {stats['cpu']['min']:.1f}%")
        
        # Memoria
        stdscr.addstr(8, 2, "Memoria:", curses.A_BOLD)
        stdscr.addstr(9, 4, f"Media: {stats['memory']['average']:.1f}%")
        stdscr.addstr(10, 4, f"Massimo: {stats['memory']['max']:.1f}%")
        stdscr.addstr(11, 4, f"Minimo: {stats['memory']['min']:.1f}%")
        
        # Disco
        stdscr.addstr(13, 2, "Disco:", curses.A_BOLD)
        stdscr.addstr(14, 4, f"Media: {stats['disk']['average']:.1f}%")
        stdscr.addstr(15, 4, f"Massimo: {stats['disk']['max']:.1f}%")
        stdscr.addstr(16, 4, f"Minimo: {stats['disk']['min']:.1f}%")
        
        # Periodo
        stdscr.addstr(18, 2, "Periodo di analisi: ultimi 5 minuti", curses.A_ITALIC)
    
    def _display_alerts(self, stdscr):
        """
        Visualizza gli allarmi attivi.
        
        Args:
            stdscr: Schermo curses.
        """
        h, w = stdscr.getmaxyx()
        
        # Intestazione
        stdscr.addstr(1, 2, "Allarmi attivi", curses.color_pair(4) | curses.A_BOLD)
        
        # Ottieni gli allarmi
        alerts = self.alert_manager.get_active_alerts()
        
        if not alerts:
            stdscr.addstr(3, 2, "Non ci sono allarmi attivi.", curses.color_pair(1))
        else:
            stdscr.addstr(3, 2, f"Ci sono {len(alerts)} allarmi attivi:", curses.color_pair(2))
            
            for i, alert in enumerate(alerts):
                color = curses.color_pair(3) if alert.is_critical else curses.color_pair(2)
                stdscr.addstr(5 + i, 2, f"{i+1}. {alert.timestamp.strftime('%H:%M:%S')} - {alert.message}", color)
    
    def _display_thresholds(self, stdscr):
        """
        Visualizza e modifica le soglie di allarme.
        
        Args:
            stdscr: Schermo curses.
        """
        h, w = stdscr.getmaxyx()
        
        # Intestazione
        stdscr.addstr(1, 2, "Configurazione soglie di allarme", curses.color_pair(4) | curses.A_BOLD)
        
        # Ottieni le soglie correnti
        thresholds = self.alert_manager.get_thresholds()
        
        # Mostra le soglie attuali
        stdscr.addstr(3, 2, "Soglie attuali:", curses.A_BOLD)
        stdscr.addstr(4, 4, f"1. CPU: {thresholds['cpu']}%")
        stdscr.addstr(5, 4, f"2. Memoria: {thresholds['memory']}%")
        stdscr.addstr(6, 4, f"3. Disco: {thresholds['disk']}%")
        
        # Istruzioni
        stdscr.addstr(8, 2, "Per modificare una soglia, premi il numero corrispondente")
        stdscr.addstr(9, 2, "Per tornare al menu principale, premi 'b'")
        
        # Gestione input
        key = stdscr.getch()
        
        if key == ord('1'):
            self._edit_threshold(stdscr, 'cpu', thresholds['cpu'])
        elif key == ord('2'):
            self._edit_threshold(stdscr, 'memory', thresholds['memory'])
        elif key == ord('3'):
            self._edit_threshold(stdscr, 'disk', thresholds['disk'])
    
    def _edit_threshold(self, stdscr, resource: str, current_value: float):
        """
        Modifica una soglia di allarme.
        
        Args:
            stdscr: Schermo curses.
            resource: Tipo di risorsa.
            current_value: Valore attuale della soglia.
        """
        h, w = stdscr.getmaxyx()
        
        # Pulisci lo schermo
        stdscr.clear()
        
        # Intestazione
        stdscr.addstr(1, 2, f"Modifica soglia per {resource}", curses.color_pair(4) | curses.A_BOLD)
        
        # Valore attuale
        stdscr.addstr(3, 2, f"Valore attuale: {current_value}%")
        
        # Richiesta nuovo valore
        stdscr.addstr(5, 2, "Inserisci il nuovo valore (0-100): ")
        curses.echo()
        curses.curs_set(1)
        
        # Inizializza il buffer di input
        input_buffer = ""
        while True:
            key = stdscr.getch()
            
            # Gestisci l'input
            if key == 10:  # Enter
                break
            elif key == 27:  # Escape
                input_buffer = ""
                break
            elif key == curses.KEY_BACKSPACE or key == 127:
                input_buffer = input_buffer[:-1]
            elif key >= 48 and key <= 57:  # Numeri 0-9
                input_buffer += chr(key)
            
            # Mostra l'input corrente
            stdscr.move(5, 37)
            stdscr.clrtoeol()
            stdscr.addstr(5, 37, input_buffer)
        
        # Ripristina lo stato del terminale
        curses.noecho()
        curses.curs_set(0)
        
        # Aggiorna la soglia se l'input è valido
        if input_buffer:
            try:
                new_value = float(input_buffer)
                if 0 <= new_value <= 100:
                    self.alert_manager.set_threshold(resource, new_value)
                    stdscr.addstr(7, 2, f"Soglia aggiornata a {new_value}%", curses.color_pair(1))
                else:
                    stdscr.addstr(7, 2, "Errore: il valore deve essere compreso tra 0 e 100", curses.color_pair(3))
            except ValueError:
                stdscr.addstr(7, 2, "Errore: inserire un valore numerico valido", curses.color_pair(3))
        
        # Attendi che l'utente prema un tasto
        stdscr.addstr(9, 2, "Premi un tasto per continuare...")
        stdscr.getch()
    
    def _create_bar(self, percentage: float, width: int) -> str:
        """
        Crea una barra di progresso.
        
        Args:
            percentage: Percentuale di completamento.
            width: Larghezza della barra.
            
        Returns:
            Stringa rappresentante la barra di progresso.
        """
        filled_width = int(width * percentage / 100)
        bar = '█' * filled_width + '░' * (width - filled_width)
        return bar
    
    def _get_color_for_value(self, value: float) -> int:
        """
        Determina il colore in base al valore.
        
        Args:
            value: Valore percentuale.
            
        Returns:
            Attributo colore curses.
        """
        if value < 70:
            return curses.color_pair(1)  # Verde (normale)
        elif value < 90:
            return curses.color_pair(2)  # Giallo (attenzione)
        else:
            return curses.color_pair(3)  # Rosso (critico)


if __name__ == "__main__":
    # Esempio di utilizzo dell'interfaccia console
    from system_monitor import SystemMonitor
    from data_analyzer import DataAnalyzer
    from alert_manager import AlertManager
    
    system_monitor = SystemMonitor()
    data_analyzer = DataAnalyzer(system_monitor)
    alert_manager = AlertManager(system_monitor, data_analyzer)
    
    console = ConsoleUI(system_monitor, data_analyzer, alert_manager)
    console.start()
