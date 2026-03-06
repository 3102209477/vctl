#ifndef CONFIG_CONFIG_MANAGER_H
#define CONFIG_CONFIG_MANAGER_H

#include <string>
#include "../include/types.h"

namespace versionctl {
namespace config {

// 解析配置文件
RepositoryConfig parseConfig(const std::string& configPath);

// 保存配置到文件
bool saveConfig(const std::string& configPath, const RepositoryConfig& config);

// 加载仓库配置
RepositoryConfig loadRepositoryConfig(const std::string& root);

// 保存仓库配置
bool saveRepositoryConfig(const std::string& root, const RepositoryConfig& config);

// 初始化仓库配置
bool initializeRepositoryConfig(const std::string& root);

// 设置配置值
bool setConfigValue(const std::string& root, const std::string& key, const std::string& value);

} // namespace config
} // namespace versionctl

#endif // CONFIG_CONFIG_MANAGER_H
