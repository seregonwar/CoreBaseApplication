#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <SFML/Graphics.hpp>
#include "../../CoreAPI.h"

namespace CoreNS {
namespace Monitoring {

class MonitoringDashboard {
public:
    MonitoringDashboard();
    ~MonitoringDashboard();

    void initialize();
    void run();
    void shutdown();

private:
    void updateResources();
    void render();
    void handleEvents();
    void drawResourceBar(float value, float maxValue, float yPos, const sf::Color& color);
    void drawText(const std::string& text, float x, float y);

    sf::RenderWindow m_window;
    sf::Font m_font;
    std::vector<sf::RectangleShape> m_resourceBars;
    std::vector<sf::Text> m_resourceLabels;
    std::vector<sf::Text> m_resourceValues;
    
    CoreAPI m_api;
    std::thread m_updateThread;
    std::atomic<bool> m_running;
    
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;
    static constexpr float BAR_WIDTH = 600.0f;
    static constexpr float BAR_HEIGHT = 30.0f;
    static constexpr float BAR_SPACING = 50.0f;
};

} // namespace Monitoring
} // namespace CoreNS 