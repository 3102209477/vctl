#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <map>
#include <mutex>
#include <vector>
#include "../include/types.h"

namespace versionctl {
namespace storage {

// 简单的内存缓存，用于存储最近访问的对象
class ObjectCache {
private:
    std::map<std::string, Blob> blobCache;
    std::map<std::string, Tree> treeCache;
    std::map<std::string, Commit> commitCache;
    
    // 缓存大小限制
    const size_t MAX_CACHE_SIZE = 100;
    
    // 线程安全锁
    mutable std::mutex cacheMutex;
    
    // LRU 列表 (简单实现)
    std::vector<std::string> lruList;
    
    void updateLRU(const std::string& hash);
    
public:
    // Blob 缓存操作
    void putBlob(const std::string& hash, const Blob& blob);
    bool getBlob(const std::string& hash, Blob& blob);
    
    // Tree 缓存操作
    void putTree(const std::string& hash, const Tree& tree);
    bool getTree(const std::string& hash, Tree& tree);
    
    // Commit 缓存操作
    void putCommit(const std::string& hash, const Commit& commit);
    bool getCommit(const std::string& hash, Commit& commit);
    
    // 清除所有缓存
    void clear();
    
    // 获取缓存统计信息
    size_t getBlobCount() const;
    size_t getTreeCount() const;
    size_t getCommitCount() const;
};

// 获取全局缓存
ObjectCache& getGlobalCache();

} // namespace storage
} // namespace versionctl

#endif // CACHE_H
