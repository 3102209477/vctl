#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace versionctl {

// 对象类型枚举
enum class ObjectType {
    BLOB,       // 文件内容
    TREE,       // 目录结构
    COMMIT      // 提交记录
};

// 将对象类型转换为字符串
inline std::string objectTypeToString(ObjectType type) {
    switch (type) {
        case ObjectType::BLOB: return "blob";
        case ObjectType::TREE: return "tree";
        case ObjectType::COMMIT: return "commit";
        default: return "unknown";
    }
}

// Blob 对象 - 存储文件内容
struct Blob {
    std::string content;      // 文件内容
    std::string hash;         // SHA-256 哈希值
    uint64_t size;            // 内容大小
    
    Blob() : size(0) {}
    Blob(const std::string& c, const std::string& h) 
        : content(c), hash(h), size(c.length()) {}
};

// Tree 条目 - 树中的一个文件/目录
struct TreeEntry {
    std::string name;         // 文件或目录名
    std::string hash;         // 对象的哈希值
    ObjectType type;          // 类型：BLOB 或 TREE
    std::string mode;         // 文件模式 (如 100644 表示普通文件)
    
    TreeEntry() : type(ObjectType::BLOB), mode("100644") {}
    TreeEntry(const std::string& n, const std::string& h, ObjectType t, const std::string& m = "100644")
        : name(n), hash(h), type(t), mode(m) {}
};

// Tree 对象 - 存储目录结构
struct Tree {
    std::vector<TreeEntry> entries;  // 条目列表
    std::string hash;                // SHA-256 哈希值
    
    void addEntry(const TreeEntry& entry) {
        entries.push_back(entry);
    }
    
    // 查找条目
    const TreeEntry* findEntry(const std::string& name) const {
        for (const auto& entry : entries) {
            if (entry.name == name) {
                return &entry;
            }
        }
        return nullptr;
    }
};

// Commit对象 - 提交记录
struct Commit {
    std::string tree;           // Tree 对象的哈希
    std::vector<std::string> parents;  // 父提交哈希列表
    std::string author;         // 作者信息
    std::string committer;      // 提交者信息
    int64_t timestamp;          // 时间戳
    std::string message;        // 提交消息
    std::string hash;           // SHA-256 哈希值
    
    Commit() : timestamp(0) {}
};

// 文件状态枚举
enum class FileStatus {
    UNTRACKED,      // 未跟踪
    UNMODIFIED,     // 未修改
    MODIFIED,       // 已修改
    ADDED,          // 已添加
    DELETED,        // 已删除
    RENAMED         // 已重命名
};

// 文件状态信息
struct StatusInfo {
    std::string path;           // 文件路径
    FileStatus status;          // 状态
    std::string oldPath;        // 旧路径 (重命名时使用)
    
    StatusInfo() : status(FileStatus::UNTRACKED) {}
    StatusInfo(const std::string& p, FileStatus s) 
        : path(p), status(s) {}
};

// 分支信息
struct BranchInfo {
    std::string name;           // 分支名称
    std::string commitHash;     // 指向的提交哈希
    bool isCurrent;             // 是否为当前分支
    
    BranchInfo() : isCurrent(false) {}
    BranchInfo(const std::string& n, const std::string& h, bool current = false)
        : name(n), commitHash(h), isCurrent(current) {}
};

// 仓库配置
struct RepositoryConfig {
    std::string repositoryName; // 仓库名称
    std::string authorName;     // 默认作者名
    std::string authorEmail;    // 默认作者邮箱
    std::vector<std::string> sourceIgnorePatterns;  // 源忽略模式
    std::vector<std::string> targetProtectPatterns; // 目标保护模式
    
    RepositoryConfig() {}
};

} // namespace versionctl

#endif // TYPES_H
