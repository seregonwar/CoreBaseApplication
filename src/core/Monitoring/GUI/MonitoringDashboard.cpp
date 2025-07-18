#define UNICODE
#define _UNICODE
#include "MonitoringDashboard.h"
#include <iostream>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include "imgui_impl_win32.h"

#include <windows.h>




namespace CoreNS {
namespace Monitoring {


static HWND g_hWnd = nullptr;



LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (::ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SIZE:
        // Logica di resize, se necessaria senza D3D
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

MonitoringDashboard::MonitoringDashboard() : m_running(false) {}
MonitoringDashboard::~MonitoringDashboard() { shutdown(); }

void MonitoringDashboard::initialize() {
    std::cout << "Inizializzazione della dashboard di monitoraggio (ImGui)..." << std::endl;
}

void MonitoringDashboard::run() {
    // Setup Win32 window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    RegisterClassEx(&wc);
    g_hWnd = CreateWindow(wc.lpszClassName, L"CoreBaseApplication - Monitoraggio Risorse (ImGui)", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(g_hWnd);

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);

    m_running = true;
    m_updateThread = std::thread(&MonitoringDashboard::updateResources, this);

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT && m_running) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        render();

        ImGui::Render();
        // Qui andrebbe la logica di rendering specifica (GDI/OpenGL ecc.)
    }

    shutdown();
}

void MonitoringDashboard::shutdown() {
    m_running = false;
    if (m_updateThread.joinable()) m_updateThread.join();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (g_hWnd) { DestroyWindow(g_hWnd); g_hWnd = nullptr; }
    UnregisterClass(L"ImGui Example", GetModuleHandle(NULL));
}

void MonitoringDashboard::updateResources() {
    while (m_running) {
        auto resources = m_api.getSystemResources();
        m_cpuUsage = static_cast<float>(resources.cpuUsagePercent);
        m_memoryMB = static_cast<float>(resources.availableMemoryBytes) / (1024.0f * 1024.0f);
        // Aggiorna storico
        m_cpuHistory.push_back(m_cpuUsage);
        if (m_cpuHistory.size() > HISTORY_SIZE) m_cpuHistory.pop_front();
        m_memHistory.push_back(m_memoryMB);
        if (m_memHistory.size() > HISTORY_SIZE) m_memHistory.pop_front();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void MonitoringDashboard::render() {
    // Tabs principali
    ImGui::Begin("CBA Monitoring Dashboard");
    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Monitoraggio")) {
            m_selectedTab = 0;
            // --- Monitoraggio Risorse ---
            ImGui::Text("CPU Usage: %.1f%%", m_cpuUsage);
            ImGui::Text("Memoria disponibile: %.1f MB", m_memoryMB);
            // Grafico CPU
            if (!m_cpuHistory.empty())
                ImGui::PlotLines("CPU (%)", m_cpuHistory.data(), (int)m_cpuHistory.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));
            // Grafico Memoria
            if (!m_memHistory.empty())
                ImGui::PlotLines("Memoria (MB)", m_memHistory.data(), (int)m_memHistory.size(), 0, nullptr, 0.0f, 16000.0f, ImVec2(0, 80));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("CBA API")) {
            m_selectedTab = 1;
            // --- Esempio: loadModule ---
            ImGui::Text("Esegui funzioni CoreAPI");
            ImGui::Separator();
            ImGui::Text("loadModule");
            static char modulePath[256] = "";
            ImGui::InputText("Percorso modulo", modulePath, IM_ARRAYSIZE(modulePath));
            if (ImGui::Button("Carica modulo")) {
                bool ok = m_api.loadModule(modulePath);
                std::string msg = std::string("[loadModule] ") + modulePath + (ok ? " [OK]" : " [ERRORE]");
                m_logLines.push_back(msg);
            }
            // Qui puoi aggiungere altre funzioni CBA con lo stesso schema
            ImGui::Separator();
            ImGui::Text("Log chiamate API:");
            for (const auto& line : m_logLines) {
                ImGui::TextWrapped("%s", line.c_str());
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    // Barra di stato in basso
    ImGui::Separator();
    ImGui::Text("Stato: %s", m_running ? "Attivo" : "Fermo");
    ImGui::End();
}

void MonitoringDashboard::handleEvents() {}

} // namespace Monitoring
} // namespace CoreNS