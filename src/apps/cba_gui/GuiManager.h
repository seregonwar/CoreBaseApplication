#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "CoreAPI.h"

class GuiManager {
public:
    GuiManager();
    ~GuiManager();

    void Run();

private:
    void Init();
    void Shutdown();
    void MainLoop();
    void RenderUI();
    void RenderSidebar(); // Nuova funzione per la sidebar/tab
    void RenderTab_Modules();
    void RenderTab_Python();
    void RenderTab_Java();
    void RenderTab_Resources();
    void RenderTab_IPC();
    void RenderTab_Log();
    void RenderTab_Config();
    int m_SelectedTab = 0; // 0: Modules, 1: Python, 2: Java, 3: Resources, 4: IPC, 5: Log, 6: Config
    GLFWwindow* m_Window = nullptr;
    CoreNS::CoreAPI m_CoreAPI;
    std::string m_PythonScriptInput;
    std::string m_PythonScriptOutput;
};