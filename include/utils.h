#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <filesystem>
#include "types.h"

namespace versionctl {

namespace utils {

// 路径处理函数
std::string joinPath(const std::string& base, const std::string& path);
std::string normalizePath(const std::string& path);
std::string relativePath(const std::string& path, const std::string& base);
bool isInsideDir(const std::string& path, const std::string& dir);

// 文件操作函数
bool fileExists(const std::string& path);
bool isDirectory(const std::string& path);
bool createDirectory(const std::string& path);
bool createDirectories(const std::string& path);
bool writeFile(const std::string& path, const std::string& content);
std::string readFile(const std::string& path);
bool copyFile(const std::string& from, const std::string& to);
bool copyDirectory(const std::string& from, const std::string& to);
bool deleteFile(const std::string& path);
bool deleteDirectory(const std::string& path);

// 时间处理函数
int64_t getCurrentTimestamp();
std::string formatTimestamp(int64_t timestamp);
std::string getCurrentUserName();

// 字符串处理函数
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);
std::string replaceAll(std::string str, const std::string& from, const std::string& to);

// 模式匹配函数
bool matchPattern(const std::string& path, const std::string& pattern);
bool matchesAnyPattern(const std::string& path, const std::vector<std::string>& patterns);

// Git 目录相关函数
std::string getVersionDir(const std::string& root);
std::string getRefsDir(const std::string& root);
std::string getHeadsDir(const std::string& root);
std::string getConfigPath(const std::string& root);
std::string getIgnorePath(const std::string& root);
std::string getTargetIgnorePath(const std::string& root);
std::string getHeadPath(const std::string& root);
std::string getCommitLogPath(const std::string& root);

// 对象路径相关
std::string getObjectPath(const std::string& root, const std::string& hash);
std::string getObjectsDir(const std::string& root);
std::string getObjectTypeDir(const std::string& root, ObjectType type);

// 文件变更检测
bool hasFileChanged(const std::string& filePath, std::time_t cachedMtime, size_t cachedSize);

// 验证函数
bool isValidHash(const std::string& hash);
bool isRepository(const std::string& path);

// 控制台工具函数
void enableConsoleColors();

} // namespace utils

} // namespace versionctl

#endif // UTILS_H
