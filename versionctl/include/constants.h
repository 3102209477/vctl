#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace versionctl {

// 版本控制目录名称
const std::string VERSION_DIR = ".version";

// 子目录名称
const std::string OBJECTS_DIR = "objects";
const std::string REFS_DIR = "refs";
const std::string HEADS_DIR = "heads";
const std::string CONFIG_FILE = "config";
const std::string IGNORE_FILE = "ignore";
const std::string TARGET_IGNORE_FILE = ".target_ignore";
const std::string COMMIT_LOG_FILE = "commit.log";
const std::string INDEX_FILE = "index";
const std::string HEAD_FILE = "HEAD";

// 对象类型标记 (二进制格式)
const char BLOB_MARKER[4] = {'B', 'L', 'O', 'B'};
const char TREE_MARKER[4] = {'T', 'R', 'E', 'E'};
const char COMMIT_MARKER[4] = {'C', 'O', 'M', 'T'};

// 文件模式
const std::string FILE_MODE_REGULAR = "100644";
const std::string FILE_MODE_EXECUTABLE = "100755";
const std::string TREE_MODE = "040000";

// 哈希值长度 (SHA-256 为 64 个十六进制字符)
const size_t HASH_LENGTH = 64;

// 块大小 (用于大文件分块处理)
const size_t CHUNK_SIZE = 64 * 1024; // 64KB

// 并行处理阈值
const size_t PARALLEL_THRESHOLD = 10;

// 默认忽略模式
const std::vector<std::string> DEFAULT_SOURCE_IGNORE_PATTERNS = {
    // 版本控制系统目录
    ".git/",
    ".git",
    ".version/",
    ".version",
    
    // Node.js 依赖
    "node_modules/",
    "node_modules",
    
    // PHP Composer 依赖
    "vendor/",
    "vendor",
    
    // 包管理器缓存和锁定文件
    ".npm/",
    ".yarn/",
    "package-lock.json",
    "yarn.lock",
    
    // 构建产物和临时文件
    "build/",
    "temp/",
    "obj/",
    "*.o",
    "*.a",
    "*.so",
    "*.dll",
    "*.exe",
    
    // Python 缓存和虚拟环境
    "__pycache__/",
    ".venv/",
    "*.pyc",
    "*.pyo",
    
    // IDE 配置
    ".vscode/",
    ".idea/",
    "*.swp",
    "*.swo",
    
    // 系统文件
    ".DS_Store",
    "Thumbs.db"
};

// 默认目标保护模式
const std::vector<std::string> DEFAULT_TARGET_PROTECT_PATTERNS = {
    "*.config",
    "*.env",
    ".env.local",
    "local.settings.json"
};

} // namespace versionctl

#endif // CONSTANTS_H
