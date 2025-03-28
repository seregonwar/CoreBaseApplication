#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace CoreNS {

struct TemplateVariable {
    std::string name;
    std::string description;
    std::string defaultValue;
};

struct ProjectTemplate {
    std::string name;
    std::string description;
    std::string version;
    std::vector<TemplateVariable> variables;
    std::filesystem::path sourcePath;
};

class ProjectTemplateManager {
public:
    ProjectTemplateManager();
    
    bool registerTemplate(const std::filesystem::path& templatePath);
    bool createProject(const std::string& templateName, 
                      const std::string& projectName,
                      const std::filesystem::path& targetPath,
                      const std::map<std::string, std::string>& variables = {});
    
    std::vector<ProjectTemplate> listTemplates() const;
    ProjectTemplate getTemplate(const std::string& name) const;
    
private:
    std::map<std::string, ProjectTemplate> m_templates;
    
    bool loadTemplateConfig(const std::filesystem::path& configPath, ProjectTemplate& tmpl);
    void processTemplateFiles(const std::filesystem::path& sourcePath,
                            const std::filesystem::path& targetPath,
                            const std::map<std::string, std::string>& variables);
    std::string replaceVariables(const std::string& content,
                               const std::map<std::string, std::string>& variables);
};

} // namespace CoreNS 