#ifndef CONFIG_IGNORE_RULES_H
#define CONFIG_IGNORE_RULES_H

#include <string>
#include <vector>
#include <regex>
#include "../include/types.h"

namespace versionctl {
namespace config {

// 将路径分隔符统一为当前平台格式
std::string normalizePathSeparators(const std::string& path);

// 将 glob 模式转换为正则表达式
std::regex patternToRegex(const std::string& pattern);

// 检查路径是否匹配模式
bool matchPattern(const std::string& path, const std::string& pattern);

// 检查路径是否匹配任一模式
bool matchesAnyPattern(const std::string& path, const std::vector<std::string>& patterns);

// 检查文件是否应被忽略
bool shouldIgnore(const std::string& relativePath, const RepositoryConfig& config);

// 检查文件是否应受保护 (不被覆盖)
bool shouldProtect(const std::string& relativePath, const RepositoryConfig& config);

// 加载忽略规则文件
RepositoryConfig loadIgnoreRules(const std::string& ignoreFilePath);

// 保存忽略规则到文件
bool saveIgnoreRules(const std::string& ignoreFilePath, const RepositoryConfig& config);

// 保存目标保护规则到文件
bool saveTargetProtectRules(const std::string& protectFilePath, const RepositoryConfig& config);

} // namespace config
} // namespace versionctl

#endif // CONFIG_IGNORE_RULES_H
