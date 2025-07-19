#include <gtest/gtest.h>
#include "core/CoreAPI.h"
#include "core/ProjectTemplate.h"

class CoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        Core::CoreAPI::ConfigParams params;
        params.configFilePath = "config.json";
        params.logLevel = Core::LogLevel::INFO;
        params.applicationName = "TestApp";
        params.applicationVersion = "1.0.0";
        
        ASSERT_TRUE(Core::CoreAPI::initialize(params));
    }
    
    void TearDown() override {
        Core::CoreAPI::shutdown();
    }
};

TEST_F(CoreTest, CoreInitialization) {
    auto& core = Core::CoreAPI::getInstance();
    ASSERT_TRUE(core.isInitialized());
}

TEST_F(CoreTest, TemplateManager) {
    Core::ProjectTemplateManager templateManager;
    
    // Registra il template
    ASSERT_TRUE(templateManager.registerTemplate("templates/cpp_project"));
    
    // Verifica che il template sia stato caricato
    auto templates = templateManager.listTemplates();
    ASSERT_FALSE(templates.empty());
    
    // Verifica le proprietà del template
    auto template = templateManager.getTemplate("cpp_project");
    ASSERT_EQ(template.name, "cpp_project");
    ASSERT_EQ(template.version, "1.0.0");
    
    // Crea un nuovo progetto
    std::map<std::string, std::string> variables = {
        {"project_name", "TestProject"},
        {"author", "Test Author"},
        {"version", "1.0.0"},
        {"description", "Test project"}
    };
    
    ASSERT_TRUE(templateManager.createProject("cpp_project", "TestProject", "./test_projects", variables));
    
    // Verifica che il progetto sia stato creato
    ASSERT_TRUE(std::filesystem::exists("./test_projects/TestProject"));
    ASSERT_TRUE(std::filesystem::exists("./test_projects/TestProject/CMakeLists.txt"));
    
    // Pulizia
    std::filesystem::remove_all("./test_projects");
}

TEST_F(CoreTest, Configuration) {
    auto& core = Core::CoreAPI::getInstance();
    
    // Verifica la configurazione
    ASSERT_EQ(core.getConfigString("application.name"), "TestApp");
    ASSERT_EQ(core.getConfigString("application.version"), "1.0.0");
    ASSERT_TRUE(core.getConfigBool("monitoring.enabled"));
} 