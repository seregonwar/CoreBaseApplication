#include "GuiManager.h"
#include <stdio.h>
#include <fstream> // Added for std::ifstream
#include <sstream> // Per lettura file compatibile MSVC

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GuiManager::GuiManager() {
    Init();
}

GuiManager::~GuiManager() {
    Shutdown();
}

void GuiManager::Init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    m_Window = glfwCreateWindow(1280, 720, "CBA GUI", NULL, NULL);
    if (m_Window == NULL)
        return;
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    m_CoreAPI.initialize("config.json");
}

void GuiManager::Shutdown() {
    m_CoreAPI.shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void GuiManager::Run() {
    MainLoop();
}

void GuiManager::MainLoop() {
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderUI();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_Window);
    }
}

void GuiManager::RenderUI() {
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);

    // Sidebar
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(250, display_h));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("CBA Sidebar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    RenderSidebar();
    ImGui::End();
    ImGui::PopStyleVar();

    // Main content area
    ImGui::SetNextWindowPos(ImVec2(250, 0));
    ImGui::SetNextWindowSize(ImVec2(display_w - 250, display_h));
    ImGui::Begin("CBA Main Content", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    switch (m_SelectedTab) {
        case 0: RenderTab_Modules(); break;
        case 1: RenderTab_Python(); break;
        case 2: RenderTab_Java(); break;
        case 3: RenderTab_Resources(); break;
        case 4: RenderTab_IPC(); break;
        case 5: RenderTab_Log(); break;
        case 6: RenderTab_Config(); break;
        default: ImGui::Text("Seleziona una sezione dalla sidebar."); break;
    }
    ImGui::End();
}

void GuiManager::RenderSidebar() {
    static const char* tabs[] = { "Moduli", "Python", "Java", "Risorse", "IPC", "Log", "Config" };
    for (int i = 0; i < 7; ++i) {
        if (ImGui::Selectable(tabs[i], m_SelectedTab == i)) {
            m_SelectedTab = i;
        }
    }
}

void GuiManager::RenderTab_Modules() {
    ImGui::Text("Gestione Moduli");
    ImGui::Separator();
    // Input per percorso modulo
    static char modulePath[256] = "";
    ImGui::InputText("Percorso modulo", modulePath, sizeof(modulePath));
    ImGui::SameLine();
    if (ImGui::Button("Carica")) {
        if (strlen(modulePath) > 0) m_CoreAPI.loadModule(modulePath);
    }
    ImGui::SameLine();
    if (ImGui::Button("Scarica")) {
        if (strlen(modulePath) > 0) m_CoreAPI.unloadModule(modulePath);
    }
    ImGui::Spacing();
    ImGui::Text("Moduli caricati:");
    std::vector<std::string> loadedModules = m_CoreAPI.getLoadedModules();
    if (loadedModules.empty()) {
        ImGui::Text("Nessun modulo caricato.");
    } else {
        for (const auto& modulePath : loadedModules) {
            CoreNS::APIModuleInfo info = m_CoreAPI.getModuleInfo(modulePath);
            if (ImGui::TreeNode(info.name.c_str())) {
                ImGui::Text("Versione: %s", info.version.c_str());
                ImGui::Text("Descrizione: %s", info.description.c_str());
                ImGui::Text("Autore: %s", info.author.c_str());
                ImGui::Text("Tipo: %s", info.type.c_str());
                ImGui::Text("Path: %s", modulePath.c_str());
                ImGui::TreePop();
            }
        }
    }
}

void GuiManager::RenderTab_Python() {
    ImGui::Text("Interprete Python Integrato");
    ImGui::Separator();
    
    // Informazioni sull'interprete
    if (ImGui::CollapsingHeader("Informazioni Interprete", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Stato: %s", m_CoreAPI.isPythonInitialized() ? "Inizializzato" : "Non inizializzato");
        if (m_CoreAPI.isPythonInitialized()) {
            ImGui::Text("Versione Python: %s", m_CoreAPI.getPythonVersion().c_str());
            ImGui::Text("Eseguibile: %s", m_CoreAPI.getPythonExecutable().c_str());
        }
        
        ImGui::Spacing();
        if (ImGui::Button("Inizializza Interprete")) {
            m_CoreAPI.initializePythonEngine();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Interprete")) {
            m_CoreAPI.resetPythonEngine();
        }
        ImGui::SameLine();
        if (ImGui::Button("Finalizza Interprete")) {
            m_CoreAPI.finalizePythonEngine();
        }
    }
    
    // Esecuzione script
    if (ImGui::CollapsingHeader("Esecuzione Script", ImGuiTreeNodeFlags_DefaultOpen)) {
        static char pythonInputBuf[4096] = "";
        static char pythonOutputBuf[4096] = "";
        static char pythonFilePath[512] = "";
        
        // Sincronizza buffer con stringa solo se la stringa è non vuota e il buffer è vuoto
        if (!m_PythonScriptInput.empty() && pythonInputBuf[0] == '\0') {
            strncpy(pythonInputBuf, m_PythonScriptInput.c_str(), sizeof(pythonInputBuf) - 1);
            pythonInputBuf[sizeof(pythonInputBuf) - 1] = '\0';
        }
        
        ImGui::Text("Script Python:");
        ImGui::InputTextMultiline("##PythonScript", pythonInputBuf, sizeof(pythonInputBuf), ImVec2(-1.0f, 150.0f));
        
        ImGui::Spacing();
        ImGui::InputText("Percorso file script", pythonFilePath, sizeof(pythonFilePath));
        ImGui::SameLine();
        if (ImGui::Button("Carica da file##py")) {
            std::ifstream file(pythonFilePath);
            if (file.good()) {
                std::ostringstream ss;
                ss << file.rdbuf();
                std::string content = ss.str();
                strncpy(pythonInputBuf, content.c_str(), sizeof(pythonInputBuf) - 1);
                pythonInputBuf[sizeof(pythonInputBuf) - 1] = '\0';
                m_PythonScriptInput = pythonInputBuf;
                m_PythonScriptOutput = "Script caricato con successo.";
                strncpy(pythonOutputBuf, m_PythonScriptOutput.c_str(), sizeof(pythonOutputBuf) - 1);
                pythonOutputBuf[sizeof(pythonOutputBuf) - 1] = '\0';
            } else {
                m_PythonScriptOutput = "Errore: impossibile leggere il file.";
                strncpy(pythonOutputBuf, m_PythonScriptOutput.c_str(), sizeof(pythonOutputBuf) - 1);
                pythonOutputBuf[sizeof(pythonOutputBuf) - 1] = '\0';
            }
        }
        
        ImGui::Spacing();
        if (ImGui::Button("Esegui con Output##py")) {
            m_PythonScriptInput = pythonInputBuf;
            m_PythonScriptOutput = m_CoreAPI.executePythonString(m_PythonScriptInput);
            strncpy(pythonOutputBuf, m_PythonScriptOutput.c_str(), sizeof(pythonOutputBuf) - 1);
            pythonOutputBuf[sizeof(pythonOutputBuf) - 1] = '\0';
        }
        ImGui::SameLine();
        if (ImGui::Button("Esegui Silenzioso##py")) {
            m_PythonScriptInput = pythonInputBuf;
            bool success = m_CoreAPI.executePythonStringQuiet(m_PythonScriptInput);
            m_PythonScriptOutput = success ? "Esecuzione completata con successo" : "Errore durante l'esecuzione";
            strncpy(pythonOutputBuf, m_PythonScriptOutput.c_str(), sizeof(pythonOutputBuf) - 1);
            pythonOutputBuf[sizeof(pythonOutputBuf) - 1] = '\0';
        }
        
        ImGui::Spacing();
        ImGui::Text("Output:");
        // Aggiorna buffer output se la stringa cambia
        if (!m_PythonScriptOutput.empty() && pythonOutputBuf[0] == '\0') {
            strncpy(pythonOutputBuf, m_PythonScriptOutput.c_str(), sizeof(pythonOutputBuf) - 1);
            pythonOutputBuf[sizeof(pythonOutputBuf) - 1] = '\0';
        }
        ImGui::InputTextMultiline("##PythonOutput", pythonOutputBuf, sizeof(pythonOutputBuf), ImVec2(-1.0f, 100.0f), ImGuiInputTextFlags_ReadOnly);
    }
    
    // Gestione variabili
    if (ImGui::CollapsingHeader("Gestione Variabili")) {
        static char varName[256] = "";
        static char varValue[512] = "";
        static int varType = 0; // 0=string, 1=int, 2=double, 3=bool
        
        ImGui::Text("Imposta Variabile:");
        ImGui::InputText("Nome##var", varName, sizeof(varName));
        ImGui::InputText("Valore##var", varValue, sizeof(varValue));
        ImGui::Combo("Tipo##var", &varType, "String\0Int\0Double\0Bool\0");
        
        if (ImGui::Button("Imposta Variabile##var")) {
            bool success = false;
            switch (varType) {
                case 0: success = m_CoreAPI.setPythonVariable(varName, std::string(varValue)); break;
                case 1: success = m_CoreAPI.setPythonVariable(varName, std::atoi(varValue)); break;
                case 2: success = m_CoreAPI.setPythonVariable(varName, std::atof(varValue)); break;
                case 3: success = m_CoreAPI.setPythonVariable(varName, std::string(varValue) == "true" || std::string(varValue) == "1"); break;
            }
            m_PythonScriptOutput = success ? "Variabile impostata con successo" : "Errore nell'impostazione della variabile";
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Ottieni Variabile##var")) {
            std::string value = m_CoreAPI.getPythonVariable(varName);
            m_PythonScriptOutput = "Valore di " + std::string(varName) + ": " + value;
        }
    }
    
    // Gestione moduli
    if (ImGui::CollapsingHeader("Gestione Moduli")) {
        static char moduleName[256] = "";
        static char moduleAlias[256] = "";
        
        ImGui::Text("Importa Modulo:");
        ImGui::InputText("Nome Modulo##mod", moduleName, sizeof(moduleName));
        ImGui::InputText("Alias (opzionale)##mod", moduleAlias, sizeof(moduleAlias));
        
        if (ImGui::Button("Importa##mod")) {
            bool success;
            if (strlen(moduleAlias) > 0) {
                success = m_CoreAPI.importPythonModuleAs(moduleName, moduleAlias);
            } else {
                success = m_CoreAPI.importPythonModule(moduleName);
            }
            m_PythonScriptOutput = success ? "Modulo importato con successo" : "Errore nell'importazione del modulo";
        }
        
        ImGui::Spacing();
        ImGui::Text("Moduli Caricati:");
        std::vector<std::string> loadedModules = m_CoreAPI.getLoadedPythonModules();
        if (loadedModules.empty()) {
            ImGui::Text("Nessun modulo caricato.");
        } else {
            for (const auto& module : loadedModules) {
                ImGui::BulletText("%s", module.c_str());
            }
        }
    }
    
    // Chiamata funzioni
    if (ImGui::CollapsingHeader("Chiamata Funzioni")) {
        static char funcName[256] = "";
        static char funcModule[256] = "";
        static char funcArgs[512] = "";
        
        ImGui::Text("Chiama Funzione:");
        ImGui::InputText("Nome Funzione##func", funcName, sizeof(funcName));
        ImGui::InputText("Modulo (opzionale)##func", funcModule, sizeof(funcModule));
        ImGui::InputText("Argomenti (separati da virgola)##func", funcArgs, sizeof(funcArgs));
        
        if (ImGui::Button("Chiama Funzione##func")) {
            std::vector<std::string> args;
            if (strlen(funcArgs) > 0) {
                std::string argsStr(funcArgs);
                std::stringstream ss(argsStr);
                std::string arg;
                while (std::getline(ss, arg, ',')) {
                    // Rimuovi spazi iniziali e finali
                    arg.erase(0, arg.find_first_not_of(" \t"));
                    arg.erase(arg.find_last_not_of(" \t") + 1);
                    args.push_back(arg);
                }
            }
            
            std::string result;
            if (strlen(funcModule) > 0) {
                result = m_CoreAPI.callPythonModuleFunction(funcModule, funcName, args);
            } else {
                result = m_CoreAPI.callPythonFunction(funcName, args);
            }
            m_PythonScriptOutput = "Risultato: " + result;
        }
    }
    
    // Gestione path Python
    if (ImGui::CollapsingHeader("Gestione Path Python")) {
        static char pathToAdd[512] = "";
        
        ImGui::Text("Aggiungi Path:");
        ImGui::InputText("Percorso##path", pathToAdd, sizeof(pathToAdd));
        if (ImGui::Button("Aggiungi al Path##path")) {
            bool success = m_CoreAPI.addToPythonPath(pathToAdd);
            m_PythonScriptOutput = success ? "Path aggiunto con successo" : "Errore nell'aggiunta del path";
        }
        
        ImGui::Spacing();
        ImGui::Text("Path Python Correnti:");
        std::vector<std::string> pythonPaths = m_CoreAPI.getPythonPath();
        if (pythonPaths.empty()) {
            ImGui::Text("Nessun path disponibile.");
        } else {
            for (const auto& path : pythonPaths) {
                ImGui::BulletText("%s", path.c_str());
            }
        }
    }
    
    // Gestione errori
    if (ImGui::CollapsingHeader("Gestione Errori")) {
        std::string lastError = m_CoreAPI.getPythonLastError();
        ImGui::Text("Ultimo Errore:");
        ImGui::TextWrapped("%s", lastError.c_str());
        
        if (ImGui::Button("Cancella Errori##err")) {
            m_CoreAPI.clearPythonError();
        }
    }
    
    // Gestione stato
    if (ImGui::CollapsingHeader("Gestione Stato")) {
        static char stateFile[512] = "python_state.pkl";
        
        ImGui::Text("File di Stato:");
        ImGui::InputText("Percorso File##state", stateFile, sizeof(stateFile));
        
        if (ImGui::Button("Salva Stato##state")) {
            bool success = m_CoreAPI.savePythonState(stateFile);
            m_PythonScriptOutput = success ? "Stato salvato con successo" : "Errore nel salvataggio dello stato";
        }
        ImGui::SameLine();
        if (ImGui::Button("Carica Stato##state")) {
            bool success = m_CoreAPI.loadPythonState(stateFile);
            m_PythonScriptOutput = success ? "Stato caricato con successo" : "Errore nel caricamento dello stato";
        }
    }
}

void GuiManager::RenderTab_Java() {
    ImGui::Text("Integrazione Java");
    ImGui::Separator();
    ImGui::Text("(In sviluppo)");
}

void GuiManager::RenderTab_Resources() {
    ImGui::Text("Monitoraggio Risorse di Sistema");
    ImGui::Separator();
    CoreNS::APISystemResources resources = m_CoreAPI.getSystemResourceUsage();
    ImGui::Spacing();
    ImGui::Text("CPU Usage");
    float cpuUsage = (float)(resources.cpuUsagePercent) / 100.0f;
    ImGui::ProgressBar(cpuUsage, ImVec2(-1.0f, 40.0f), nullptr);
    ImGui::SameLine();
    ImGui::Text("%.2f%%", resources.cpuUsagePercent);
    ImGui::Spacing();
    ImGui::Text("Memory Usage");
    float memUsage = (resources.totalMemoryBytes > 0) ? (float)(resources.totalMemoryBytes - resources.availableMemoryBytes) / (float)resources.totalMemoryBytes : 0.0f;
    ImGui::ProgressBar(memUsage, ImVec2(-1.0f, 40.0f), nullptr);
    ImGui::SameLine();
    ImGui::Text("%.2f / %.2f GB", (resources.totalMemoryBytes - resources.availableMemoryBytes) / (1024.0 * 1024.0 * 1024.0), resources.totalMemoryBytes / (1024.0 * 1024.0 * 1024.0));
    ImGui::Spacing();
    ImGui::Text("Disk Usage");
    float diskUsage = (resources.totalDiskBytes > 0) ? (float)(resources.totalDiskBytes - resources.availableDiskBytes) / (float)resources.totalDiskBytes : 0.0f;
    ImGui::ProgressBar(diskUsage, ImVec2(-1.0f, 40.0f), nullptr);
    ImGui::SameLine();
    ImGui::Text("%.2f / %.2f GB", (resources.totalDiskBytes - resources.availableDiskBytes) / (1024.0 * 1024.0 * 1024.0), resources.totalDiskBytes / (1024.0 * 1024.0 * 1024.0));
    ImGui::Spacing();
    ImGui::Text("Network Usage");
    ImGui::ProgressBar((float)resources.networkUsagePercent / 100.0f, ImVec2(-1.0f, 40.0f), nullptr);
    ImGui::SameLine();
    ImGui::Text("%.2f%%", resources.networkUsagePercent);
    ImGui::Spacing();
    ImGui::Text("GPU Usage");
    ImGui::ProgressBar((float)resources.gpuUsagePercent / 100.0f, ImVec2(-1.0f, 40.0f), nullptr);
    ImGui::SameLine();
    ImGui::Text("%.2f%%", resources.gpuUsagePercent);
}

void GuiManager::RenderTab_IPC() {
    ImGui::Text("Gestione IPC");
    ImGui::Separator();
    ImGui::Text("(In sviluppo)");
}

void GuiManager::RenderTab_Log() {
    ImGui::Text("Log di Sistema");
    ImGui::Separator();
    ImGui::Text("(In sviluppo)");
}

void GuiManager::RenderTab_Config() {
    ImGui::Text("Configurazione CoreAPI");
    ImGui::Separator();
    ImGui::Text("(In sviluppo)");
}