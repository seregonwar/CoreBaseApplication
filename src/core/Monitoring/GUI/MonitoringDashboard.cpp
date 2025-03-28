#include "MonitoringDashboard.h"
#include <iostream>

namespace CoreNS {
namespace Monitoring {

MonitoringDashboard::MonitoringDashboard() 
    : m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "CoreBaseApplication - Monitoraggio Risorse")
    , m_running(false) {
    m_window.setFramerateLimit(60);
}

MonitoringDashboard::~MonitoringDashboard() {
    shutdown();
}

void MonitoringDashboard::initialize() {
    std::cout << "Inizializzazione della dashboard di monitoraggio..." << std::endl;
    
    // Pre-carica il font per evitare problemi durante l'esecuzione
    if (!m_font.loadFromFile("arial.ttf")) {
        std::cerr << "Avviso: Font non trovato. Verrà utilizzato un font di default." << std::endl;
    }
}

void MonitoringDashboard::run() {
    // Se il font non è stato caricato durante l'inizializzazione, riprova
    if (m_font.getInfo().family.empty() && !m_font.loadFromFile("arial.ttf")) {
        std::cerr << "Errore nel caricamento del font" << std::endl;
        return;
    }

    m_running = true;
    m_updateThread = std::thread(&MonitoringDashboard::updateResources, this);

    while (m_running && m_window.isOpen()) {
        handleEvents();
        render();
    }

    shutdown();
}

void MonitoringDashboard::shutdown() {
    m_running = false;
    if (m_updateThread.joinable()) {
        m_updateThread.join();
    }
    m_window.close();
}

void MonitoringDashboard::updateResources() {
    while (m_running) {
        auto resources = m_api.getSystemResources();
        
        // Aggiorna i valori delle risorse
        m_resourceValues.clear();
        
        // CPU
        std::string cpuText = "CPU: " + std::to_string(resources.cpuUsagePercent) + "%";
        sf::Text cpuValue(cpuText, m_font, 16);
        cpuValue.setFillColor(sf::Color::White);
        cpuValue.setPosition(20, 100);
        m_resourceValues.push_back(cpuValue);
        
        // Memoria
        std::string memText = "Memoria: " + std::to_string(resources.availableMemoryBytes / (1024 * 1024)) + " MB";
        sf::Text memValue(memText, m_font, 16);
        memValue.setFillColor(sf::Color::White);
        memValue.setPosition(20, 150);
        m_resourceValues.push_back(memValue);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void MonitoringDashboard::render() {
    m_window.clear(sf::Color(30, 30, 30));
    
    // Disegna i valori delle risorse
    for (const auto& value : m_resourceValues) {
        m_window.draw(value);
    }
    
    m_window.display();
}

void MonitoringDashboard::handleEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_running = false;
        }
    }
}

void MonitoringDashboard::drawResourceBar(float value, float maxValue, float yPos, const sf::Color& color) {
    sf::RectangleShape bar(sf::Vector2f(BAR_WIDTH * (value / maxValue), BAR_HEIGHT));
    bar.setFillColor(color);
    bar.setPosition(20, yPos);
    m_window.draw(bar);
}

void MonitoringDashboard::drawText(const std::string& text, float x, float y) {
    sf::Text sfText(text, m_font, 16);
    sfText.setFillColor(sf::Color::White);
    sfText.setPosition(x, y);
    m_window.draw(sfText);
}

} // namespace Monitoring
} // namespace CoreNS 