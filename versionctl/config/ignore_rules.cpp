#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"

namespace versionctl {
namespace config {

// 将路径分隔符统一为当前平台格式
std::string normalizePathSeparators(const std::string& path) {
    std::string result = path;
#ifdef _WIN32
    std::replace(result.begin(), result.end(), '/', '\\');
#else
    std::replace(result.begin(), result.end(), '\\', '/');
#endif
    return result;
}

// 将 glob 模式转换为正则表达式
std::regex patternToRegex(const std::string& pattern) {
    std::string regexStr;
    
    // 处理目录分隔符
    std::string normalizedPattern = normalizePathSeparators(pattern);
    
    // 转义正则特殊字符，但保留通配符
    for (size_t i = 0; i < normalizedPattern.length(); i++) {
        char c = normalizedPattern[i];
        
        switch (c) {
            case '*':
                // * 匹配除路径分隔符外的任意字符
                if (i + 1 < normalizedPattern.length() && normalizedPattern[i + 1] == '*') {
                    // ** 匹配任意内容 (包括路径分隔符)
                    regexStr += ".*";
                    i++; // 跳过下一个*
                } else {
                    regexStr += "[^/\\\\]*";
                }
                break;
                
            case '?':
                // ? 匹配单个字符 (除路径分隔符)
                regexStr += "[^/\\\\]";
                break;
                
            case '.':
                regexStr += "\\.";
                break;
                
            case '[':
                regexStr += "\\[";
                break;
                
            case ']':
                regexStr += "\\]";
                break;
                
            case '(':
                regexStr += "\\(";
                break;
                
            case ')':
                regexStr += "\\)";
                break;
                
            case '{':
                regexStr += "\\{";
                break;
                
            case '}':
                regexStr += "\\}";
                break;
                
            case '+':
                regexStr += "\\+";
                break;
                
            case '^':
                regexStr += "\\^";
                break;
                
            case '$':
                regexStr += "\\$";
                break;
                
            case '|':
                regexStr += "\\|";
                break;
                
            default:
                regexStr += c;
                break;
        }
    }
    
    // 添加行首和行尾锚点
    regexStr = "^" + regexStr + "$";
    
    return std::regex(regexStr, std::regex::icase | std::regex::ECMAScript);
}

// 检查路径是否匹配模式
bool matchPattern(const std::string& path, const std::string& pattern) {
    std::string normalizedPath = normalizePathSeparators(path);
    std::string normalizedPattern = normalizePathSeparators(pattern);
    
    try {
        std::regex re = patternToRegex(normalizedPattern);
        return std::regex_match(normalizedPath, re);
    } catch (const std::regex_error&) {
        // 如果正则表达式编译失败，回退到简单匹配
        return normalizedPath.find(normalizedPattern) != std::string::npos;
    }
}

// 检查路径是否匹配任一模式
bool matchesAnyPattern(const std::string& path, const std::vector<std::string>& patterns) {
    for (const auto& pattern : patterns) {
        if (matchPattern(path, pattern)) {
            return true;
        }
    }
    return false;
}

// 检查文件是否应被忽略
bool shouldIgnore(const std::string& relativePath, const RepositoryConfig& config) {
    return matchesAnyPattern(relativePath, config.sourceIgnorePatterns);
}

// 检查文件是否应受保护 (不被覆盖)
bool shouldProtect(const std::string& relativePath, const RepositoryConfig& config) {
    return matchesAnyPattern(relativePath, config.targetProtectPatterns);
}

// 加载忽略规则文件
RepositoryConfig loadIgnoreRules(const std::string& ignoreFilePath) {
    return config::parseConfig(ignoreFilePath);
}

// 保存忽略规则到文件
bool saveIgnoreRules(const std::string& ignoreFilePath, const RepositoryConfig& config) {
    std::ofstream file(ignoreFilePath);
    if (!file) {
        return false;
    }
    
    file << "# Source Ignore Rules\n";
    file << "# Files and directories that should not be tracked by version control\n\n";
    
    for (const auto& pattern : config.sourceIgnorePatterns) {
        file << pattern << "\n";
    }
    
    file.close();
    return true;
}

// 保存目标保护规则到文件
bool saveTargetProtectRules(const std::string& protectFilePath, const RepositoryConfig& config) {
    std::ofstream file(protectFilePath);
    if (!file) {
        return false;
    }
    
    file << "# Target Protect Rules\n";
    file << "# Files that should not be overwritten during checkout/clone\n\n";
    
    for (const auto& pattern : config.targetProtectPatterns) {
        file << pattern << "\n";
    }
    
    file.close();
    return true;
}

} // namespace config
} // namespace versionctl
