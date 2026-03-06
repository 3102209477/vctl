#include "index.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../storage/binary_io.h"
#include <fstream>
#include <filesystem>

namespace versionctl {
namespace core {

// 全局 index 实例
static Index globalIndex;

Index::Index() : dirty(false) {}

bool Index::load(const std::string& root) {
    indexPath = utils::joinPath(root, VERSION_DIR);
    indexPath = utils::joinPath(indexPath, "index");
    
    if (!utils::fileExists(indexPath)) {
        entries.clear();
        return true;
    }
    
    try {
        std::ifstream file(indexPath, std::ios::binary);
        if (!file) {
            return false;
        }
        
        // 读取条目数量
        uint32_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        entries.clear();
        entries.reserve(count);
        
        for (uint32_t i = 0; i < count; i++) {
            IndexEntry entry;
            
            // 读取路径长度
            uint16_t pathLen;
            file.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen));
            
            // 读取路径
            std::vector<char> pathBuf(pathLen);
            file.read(pathBuf.data(), pathLen);
            entry.path.assign(pathBuf.begin(), pathBuf.end());
            
            // 读取哈希（64 字符）
            std::vector<char> hashBuf(HASH_LENGTH);
            file.read(hashBuf.data(), HASH_LENGTH);
            entry.hash.assign(hashBuf.begin(), hashBuf.end());
            
            // 读取 mtime
            file.read(reinterpret_cast<char*>(&entry.mtime), sizeof(entry.mtime));
            
            // 读取 size
            file.read(reinterpret_cast<char*>(&entry.size), sizeof(entry.size));
            
            // 读取 flags
            file.read(reinterpret_cast<char*>(&entry.flags), sizeof(entry.flags));
            
            entries.push_back(entry);
        }
        
        dirty = false;
        return true;
        
    } catch (...) {
        entries.clear();
        return false;
    }
}

bool Index::save(const std::string& root) const {
    std::string indexDir = utils::joinPath(root, VERSION_DIR);
    utils::createDirectories(indexDir);
    
    std::string indexPath = utils::joinPath(indexDir, "index");
    
    try {
        std::ofstream file(indexPath, std::ios::binary);
        if (!file) {
            return false;
        }
        
        // 写入条目数量
        uint32_t count = static_cast<uint32_t>(entries.size());
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        // 写入每个条目
        for (const auto& entry : entries) {
            // 写入路径长度
            uint16_t pathLen = static_cast<uint16_t>(entry.path.length());
            file.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
            
            // 写入路径
            file.write(entry.path.c_str(), pathLen);
            
            // 写入哈希
            file.write(entry.hash.c_str(), HASH_LENGTH);
            
            // 写入 mtime
            file.write(reinterpret_cast<const char*>(&entry.mtime), sizeof(entry.mtime));
            
            // 写入 size
            file.write(reinterpret_cast<const char*>(&entry.size), sizeof(entry.size));
            
            // 写入 flags
            file.write(reinterpret_cast<const char*>(&entry.flags), sizeof(entry.flags));
        }
        
        return true;
        
    } catch (...) {
        return false;
    }
}

void Index::add(const std::string& path, const std::string& hash, 
                std::time_t mtime, size_t size) {
    // 检查是否已存在
    for (auto& entry : entries) {
        if (entry.path == path) {
            // 更新现有条目
            entry.hash = hash;
            entry.mtime = mtime;
            entry.size = size;
            dirty = true;
            return;
        }
    }
    
    // 添加新条目
    entries.emplace_back(path, hash, mtime, size);
    dirty = true;
}

bool Index::remove(const std::string& path) {
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->path == path) {
            entries.erase(it);
            dirty = true;
            return true;
        }
    }
    return false;
}

void Index::clear() {
    entries.clear();
    dirty = true;
}

bool Index::hasFile(const std::string& path) const {
    for (const auto& entry : entries) {
        if (entry.path == path) {
            return true;
        }
    }
    return false;
}

const IndexEntry* Index::getEntry(const std::string& path) const {
    for (const auto& entry : entries) {
        if (entry.path == path) {
            return &entry;
        }
    }
    return nullptr;
}

std::vector<std::string> Index::getStagedFiles() const {
    std::vector<std::string> files;
    files.reserve(entries.size());
    for (const auto& entry : entries) {
        files.push_back(entry.path);
    }
    return files;
}

Tree Index::buildTree() const {
    Tree tree;
    
    // 按目录结构组织文件
    std::map<std::string, std::vector<const IndexEntry*>> dirMap;
    
    for (const auto& entry : entries) {
        size_t lastSep = entry.path.find_last_of('/');
        std::string dir = (lastSep == std::string::npos) ? "" : entry.path.substr(0, lastSep);
        dirMap[dir].push_back(&entry);
    }
    
    // 为每个目录创建条目
    for (const auto& [dir, dirEntries] : dirMap) {
        TreeEntry treeEntry;
        treeEntry.name = dir.empty() ? "" : dir.substr(dir.find_last_of('/') + 1);
        
        // 这里简化处理，实际应该递归构建树
        // 对于本地使用场景，单层目录结构已足够
        
        tree.entries.push_back(treeEntry);
    }
    
    return tree;
}

Index& getIndex() {
    return globalIndex;
}

} // namespace core
} // namespace versionctl
