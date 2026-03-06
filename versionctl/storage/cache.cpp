#include <string>
#include <map>
#include <mutex>
#include <vector>
#include "../include/types.h"
#include "../include/cache.h"

namespace versionctl {
namespace storage {

void ObjectCache::updateLRU(const std::string& hash) {
    // 从当前位置移除
    for (auto it = lruList.begin(); it != lruList.end(); ++it) {
        if (*it == hash) {
            lruList.erase(it);
            break;
        }
    }
    
    // 添加到头部
    lruList.insert(lruList.begin(), hash);
    
    // 如果超过限制，移除最旧的
    if (lruList.size() > MAX_CACHE_SIZE) {
        std::string oldest = lruList.back();
        lruList.pop_back();
        
        // 从各缓存中清除
        blobCache.erase(oldest);
        treeCache.erase(oldest);
        commitCache.erase(oldest);
    }
}

// Blob 缓存操作
void ObjectCache::putBlob(const std::string& hash, const Blob& blob) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    blobCache[hash] = blob;
    updateLRU(hash);
}

bool ObjectCache::getBlob(const std::string& hash, Blob& blob) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = blobCache.find(hash);
    if (it != blobCache.end()) {
        updateLRU(hash);
        blob = it->second;
        return true;
    }
    return false;
}

// Tree 缓存操作
void ObjectCache::putTree(const std::string& hash, const Tree& tree) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    treeCache[hash] = tree;
    updateLRU(hash);
}

bool ObjectCache::getTree(const std::string& hash, Tree& tree) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = treeCache.find(hash);
    if (it != treeCache.end()) {
        updateLRU(hash);
        tree = it->second;
        return true;
    }
    return false;
}

// Commit 缓存操作
void ObjectCache::putCommit(const std::string& hash, const Commit& commit) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    commitCache[hash] = commit;
    updateLRU(hash);
}

bool ObjectCache::getCommit(const std::string& hash, Commit& commit) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = commitCache.find(hash);
    if (it != commitCache.end()) {
        updateLRU(hash);
        commit = it->second;
        return true;
    }
    return false;
}

// 清除所有缓存
void ObjectCache::clear() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    blobCache.clear();
    treeCache.clear();
    commitCache.clear();
    lruList.clear();
}

// 获取缓存统计信息
size_t ObjectCache::getBlobCount() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return blobCache.size();
}

size_t ObjectCache::getTreeCount() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return treeCache.size();
}

size_t ObjectCache::getCommitCount() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return commitCache.size();
}

// 全局缓存实例
ObjectCache globalCache;

// 获取全局缓存
ObjectCache& getGlobalCache() {
    return globalCache;
}

} // namespace storage
} // namespace versionctl