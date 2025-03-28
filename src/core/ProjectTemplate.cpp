#include "ProjectTemplate.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CoreNS {

ProjectTemplateManager::ProjectTemplateManager() {
    // Cerca i template nella directory predefinita
    auto templateDir = std::filesystem::path("templates");
    if (std::filesystem::exists(templateDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(templateDir)) {
            if (entry.is_directory()) {
                registerTemplate(entry.path());
            }
        }
    }
}

bool ProjectTemplateManager::registerTemplate(const std::filesystem::path& templatePath) {
    auto configPath = templatePath / "template.json";
    if (!std::filesystem::exists(configPath)) {
        return false;
    }
    
    ProjectTemplate tmpl;
    if (!loadTemplateConfig(configPath, tmpl)) {
        return false;
    }
    
    tmpl.sourcePath = templatePath;
    m_templates[tmpl.name] = tmpl;
    return true;
}

bool ProjectTemplateManager::loadTemplateConfig(const std::filesystem::path& configPath, ProjectTemplate& tmpl) {
    try {
        std::ifstream file(configPath);
        json j;
        file >> j;
        
        tmpl.name = j["name"];
        tmpl.description = j["description"];
        tmpl.version = j["version"];
        
        for (const auto& var : j["variables"]) {
            TemplateVariable variable;
            variable.name = var["name"];
            variable.description = var["description"];
            variable.defaultValue = var["default"];
            tmpl.variables.push_back(variable);
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool ProjectTemplateManager::createProject(const std::string& templateName,
                                         const std::string& projectName,
                                         const std::filesystem::path& targetPath,
                                         const std::map<std::string, std::string>& variables) {
    auto it = m_templates.find(templateName);
    if (it == m_templates.end()) {
        return false;
    }
    
    const auto& tmpl = it->second;
    auto projectPath = targetPath / projectName;
    
    if (std::filesystem::exists(projectPath)) {
        return false;
    }
    
    std::filesystem::create_directories(projectPath);
    processTemplateFiles(tmpl.sourcePath, projectPath, variables);
    
    return true;
}

void ProjectTemplateManager::processTemplateFiles(const std::filesystem::path& sourcePath,
                                                const std::filesystem::path& targetPath,
                                                const std::map<std::string, std::string>& variables) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(sourcePath)) {
        if (entry.is_directory()) {
            auto relativePath = std::filesystem::relative(entry.path(), sourcePath);
            auto targetDir = targetPath / relativePath;
            std::filesystem::create_directories(targetDir);
            continue;
        }
        
        if (entry.path().filename() == "template.json") {
            continue;
        }
        
        auto relativePath = std::filesystem::relative(entry.path(), sourcePath);
        auto targetFile = targetPath / relativePath;
        
        std::ifstream sourceFile(entry.path());
        std::ofstream targetFileStream(targetFile);
        
        std::string content((std::istreambuf_iterator<char>(sourceFile)),
                           std::istreambuf_iterator<char>());
        
        targetFileStream << replaceVariables(content, variables);
    }
}

std::string ProjectTemplateManager::replaceVariables(const std::string& content,
                                                   const std::map<std::string, std::string>& variables) {
    std::string result = content;
    
    for (const auto& [name, value] : variables) {
        std::string pattern = "\\{\\{" + name + "\\}\\}";
        result = std::regex_replace(result, std::regex(pattern), value);
    }
    
    return result;
}

std::vector<ProjectTemplate> ProjectTemplateManager::listTemplates() const {
    std::vector<ProjectTemplate> result;
    for (const auto& [name, tmpl] : m_templates) {
        result.push_back(tmpl);
    }
    return result;
}

ProjectTemplate ProjectTemplateManager::getTemplate(const std::string& name) const {
    auto it = m_templates.find(name);
    if (it != m_templates.end()) {
        return it->second;
    }
    return ProjectTemplate{};
}

} // namespace CoreNS 