#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"

namespace versionctl {
namespace config {

// 解析配置文件
RepositoryConfig parseConfig(const std::string& configPath) {
    RepositoryConfig config;
    
    if (!utils::fileExists(configPath)) {
        return config;
    }
    
    std::ifstream file(configPath);
    if (!file) {
        return config;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        // 清理行首尾空白
        line = utils::trim(line);
        
        // 跳过空行和注释
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // 检查是否是 section 头
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // 解析 key = value
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = utils::trim(line.substr(0, eqPos));
            std::string value = utils::trim(line.substr(eqPos + 1));
            
            if (currentSection == "repository") {
                if (key == "name") {
                    config.repositoryName = value;
                } else if (key == "authorName") {
                    config.authorName = value;
                } else if (key == "authorEmail") {
                    config.authorEmail = value;
                }
            } else if (currentSection == "source-ignore") {
                if (!value.empty()) {
                    config.sourceIgnorePatterns.push_back(value);
                }
            } else if (currentSection == "target-protect") {
                if (!value.empty()) {
                    config.targetProtectPatterns.push_back(value);
                }
            }
        } else {
            // 纯模式列表 (向后兼容旧格式)
            if (currentSection == "source-ignore") {
                config.sourceIgnorePatterns.push_back(line);
            } else if (currentSection == "target-protect") {
                config.targetProtectPatterns.push_back(line);
            }
        }
    }
    
    file.close();
    return config;
}

// 保存配置到文件
bool saveConfig(const std::string& configPath, const RepositoryConfig& config) {
    std::ofstream file(configPath);
    if (!file) {
        return false;
    }
    
    file << "# Version Control System Configuration\n\n";
    
    file << "[repository]\n";
    if (!config.repositoryName.empty()) {
        file << "name = " << config.repositoryName << "\n";
    }
    if (!config.authorName.empty()) {
        file << "authorName = " << config.authorName << "\n";
    }
    if (!config.authorEmail.empty()) {
        file << "authorEmail = " << config.authorEmail << "\n";
    }
    
    file << "\n[source-ignore]\n";
    for (const auto& pattern : config.sourceIgnorePatterns) {
        file << pattern << "\n";
    }
    
    file << "\n[target-protect]\n";
    for (const auto& pattern : config.targetProtectPatterns) {
        file << pattern << "\n";
    }
    
    file.close();
    return true;
}

// 加载仓库配置
RepositoryConfig loadRepositoryConfig(const std::string& root) {
    RepositoryConfig config;
    
    // 优先从 config 文件加载
    std::string configPath = utils::getConfigPath(root);
    config = parseConfig(configPath);
    
    // 如果 config 为空，尝试从 ignore 文件加载 (向后兼容)
    if (config.sourceIgnorePatterns.empty()) {
        std::string ignorePath = utils::getIgnorePath(root);
        if (utils::fileExists(ignorePath)) {
            RepositoryConfig ignoreConfig = parseConfig(ignorePath);
            config.sourceIgnorePatterns = ignoreConfig.sourceIgnorePatterns;
        }
    }
    
    // 如果没有自定义配置，使用默认值
    if (config.sourceIgnorePatterns.empty()) {
        config.sourceIgnorePatterns = DEFAULT_SOURCE_IGNORE_PATTERNS;
    }
    
    if (config.targetProtectPatterns.empty()) {
        config.targetProtectPatterns = DEFAULT_TARGET_PROTECT_PATTERNS;
    }
    
    return config;
}

// 初始化仓库配置
bool initializeRepositoryConfig(const std::string& root) {
    RepositoryConfig config;
    config.repositoryName = utils::relativePath(root, std::filesystem::current_path().string());
    config.sourceIgnorePatterns = DEFAULT_SOURCE_IGNORE_PATTERNS;
    config.targetProtectPatterns = DEFAULT_TARGET_PROTECT_PATTERNS;
    
    std::string configPath = utils::getConfigPath(root);
    return saveConfig(configPath, config);
}

// 更新配置项
bool setConfigValue(const std::string& root, const std::string& key, const std::string& value) {
    RepositoryConfig config = loadRepositoryConfig(root);
    
    if (key == "repository.name") {
        config.repositoryName = value;
    } else if (key == "user.name") {
        config.authorName = value;
    } else if (key == "user.email") {
        config.authorEmail = value;
    } else {
        return false;
    }
    
    std::string configPath = utils::getConfigPath(root);
    return saveConfig(configPath, config);
}

// 获取配置项
std::string getConfigValue(const std::string& root, const std::string& key) {
    RepositoryConfig config = loadRepositoryConfig(root);
    
    if (key == "repository.name") {
        return config.repositoryName;
    } else if (key == "user.name") {
        return config.authorName;
    } else if (key == "user.email") {
        return config.authorEmail;
    }
    
    return "";
}

} // namespace config
} // namespace versionctl
