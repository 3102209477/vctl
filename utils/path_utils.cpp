#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include "../include/types.h"
#include "../include/constants.h"

namespace versionctl {
namespace utils {

// 路径处理函数实现
std::string joinPath(const std::string& base, const std::string& path) {
    if (base.empty()) return path;
    if (path.empty()) return base;
    
    if (base.back() == '/' || base.back() == '\\') {
        return base + path;
    }
    
    return base + "/" + path;
}

std::string normalizePath(const std::string& path) {
    std::string result = path;
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

std::string relativePath(const std::string& path, const std::string& base) {
    namespace fs = std::filesystem;
    try {
        fs::path relPath = fs::relative(path, base);
        return relPath.string();
    } catch (...) {
        return path;
    }
}

bool isInsideDir(const std::string& path, const std::string& dir) {
    namespace fs = std::filesystem;
    try {
        fs::path absPath = fs::absolute(path);
        fs::path absDir = fs::absolute(dir);
        
        std::string pathStr = normalizePath(absPath.string());
        std::string dirStr = normalizePath(absDir.string());
        
        if (dirStr.back() != '/') {
            dirStr += '/';
        }
        
        return pathStr.find(dirStr) == 0 || pathStr == dirStr.substr(0, dirStr.length() - 1);
    } catch (...) {
        return false;
    }
}

// 文件操作函数实现
bool fileExists(const std::string& path) {
    try {
        return std::filesystem::exists(path);
    } catch (...) {
        return false;
    }
}

bool isDirectory(const std::string& path) {
    try {
        return std::filesystem::is_directory(path);
    } catch (...) {
        return false;
    }
}

bool createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directory(path);
    } catch (...) {
        return false;
    }
}

bool createDirectories(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (...) {
        return false;
    }
}

bool writeFile(const std::string& path, const std::string& content) {
    try {
        // 确保父目录存在
        std::filesystem::path fsPath(path);
        if (fsPath.has_parent_path()) {
            createDirectories(fsPath.parent_path().string());
        }
        
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }
        file.write(content.c_str(), content.length());
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

std::string readFile(const std::string& path) {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    } catch (...) {
        return "";
    }
}

bool copyFile(const std::string& from, const std::string& to) {
    try {
        // 确保目标目录存在
        std::filesystem::path fsTo(to);
        if (fsTo.has_parent_path()) {
            createDirectories(fsTo.parent_path().string());
        }
        
        return std::filesystem::copy_file(from, to, 
                                          std::filesystem::copy_options::overwrite_existing);
    } catch (...) {
        return false;
    }
}

bool copyDirectory(const std::string& from, const std::string& to) {
    try {
        createDirectories(to);
        
        for (const auto& entry : std::filesystem::directory_iterator(from)) {
            std::string destPath = joinPath(to, entry.path().filename().string());
            
            if (entry.is_directory()) {
                copyDirectory(entry.path().string(), destPath);
            } else {
                copyFile(entry.path().string(), destPath);
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool deleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (...) {
        return false;
    }
}

bool deleteDirectory(const std::string& path) {
    try {
        return std::filesystem::remove_all(path) > 0;
    } catch (...) {
        return false;
    }
}

// 时间处理函数实现
int64_t getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

std::string formatTimestamp(int64_t timestamp) {
    std::time_t time = timestamp;
    std::tm* tm = std::localtime(&time);
    
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 字符串处理函数实现
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
    std::string result;
    
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) {
            result += delimiter;
        }
        result += parts[i];
    }
    
    return result;
}

bool startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

// Git 目录相关函数实现
std::string getVersionDir(const std::string& root) {
    return joinPath(root, VERSION_DIR);
}

std::string getObjectsDir(const std::string& root) {
    return joinPath(getVersionDir(root), OBJECTS_DIR);
}

std::string getRefsDir(const std::string& root) {
    return joinPath(getVersionDir(root), REFS_DIR);
}

std::string getHeadsDir(const std::string& root) {
    return joinPath(getRefsDir(root), HEADS_DIR);
}

std::string getConfigPath(const std::string& root) {
    return joinPath(getVersionDir(root), CONFIG_FILE);
}

std::string getIgnorePath(const std::string& root) {
    return joinPath(getVersionDir(root), IGNORE_FILE);
}

std::string getTargetIgnorePath(const std::string& root) {
    return joinPath(root, TARGET_IGNORE_FILE);
}

std::string getHeadPath(const std::string& root) {
    return joinPath(getVersionDir(root), HEAD_FILE);
}

std::string getCommitLogPath(const std::string& root) {
    return joinPath(getVersionDir(root), COMMIT_LOG_FILE);
}

// 对象存储相关函数实现
std::string getObjectPath(const std::string& root, const std::string& hash) {
    // 使用扁平化的对象存储结构
    return joinPath(getObjectsDir(root), hash);
}

std::string getObjectTypeDir(const std::string& root, ObjectType /*type*/) {
    // 所有对象都存储在 objects 目录下，不使用子目录
    return getObjectsDir(root);
}

// 验证函数实现
bool isValidHash(const std::string& hash) {
    // SHA-256 哈希值为 64 个十六进制字符
    if (hash.length() != HASH_LENGTH) {
        return false;
    }
    
    for (char c : hash) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    
    return true;
}

// 检查文件是否发生变化（基于 mtime 和 size）
bool hasFileChanged(const std::string& filePath, std::time_t cachedMtime, size_t /*cachedSize*/) {
    struct stat st;
    if (stat(filePath.c_str(), &st) != 0) {
        return true; // 文件不存在或无法访问
    }
    
    // 快速检查：mtime 未变则内容未变
    return st.st_mtime != cachedMtime;
}

bool isRepository(const std::string& path) {
    std::string versionDir = getVersionDir(path);
    return fileExists(versionDir) && isDirectory(versionDir);
}

} // namespace utils
} // namespace versionctl
