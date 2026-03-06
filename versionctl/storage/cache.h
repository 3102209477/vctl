#ifndef STORAGE_CACHE_H
#define STORAGE_CACHE_H

#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include "../include/types.h"

namespace versionctl {
namespace storage {

// 高性能 LRU 缓存 - O(1) 时间复杂度
class ObjectCache {
private:
    // Blob 缓存：hash -> (blob, list_iterator)
    std::unordered_map<std::string, std::pair<Blob, std::list<std::string>::iterator>> blobCache;
    std::unordered_map<std::string, std::pair<Tree, std::list<std::string>::iterator>> treeCache;
    std::unordered_map<std::string, std::pair<Commit, std::list<std::string>::iterator>> commitCache;
    
    // LRU 列表：最近使用的在头部
    std::list<std::string> lruList;
    
    // 缓存大小限制
    const size_t MAX_CACHE_SIZE = 100;
    
    // 线程安全锁
    mutable std::mutex cacheMutex;
    
    void updateLRU(const std::string& hash) {
        // 检查是否已在缓存中
        auto blobIt = blobCache.find(hash);
        if (blobIt != blobCache.end()) {
            // 移动到 LRU 头部
            lruList.erase(blobIt->second.second);
            lruList.push_front(hash);
            blobIt->second.second = lruList.begin();
            return;
        }
        
        auto treeIt = treeCache.find(hash);
        if (treeIt != treeCache.end()) {
            lruList.erase(treeIt->second.second);
            lruList.push_front(hash);
            treeIt->second.second = lruList.begin();
            return;
        }
        
        auto commitIt = commitCache.find(hash);
        if (commitIt != commitCache.end()) {
            lruList.erase(commitIt->second.second);
            lruList.push_front(hash);
            commitIt->second.second = lruList.begin();
            return;
        }
        
        // 新元素添加到头部
        lruList.push_front(hash);
        
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
    
public:
    // Blob 缓存操作
    void putBlob(const std::string& hash, const Blob& blob) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = blobCache.find(hash);
        if (it != blobCache.end()) {
            // 更新现有元素
            it->second.first = blob;
            updateLRU(hash);
        } else {
            // 添加新元素
            lruList.push_front(hash);
            blobCache[hash] = {blob, lruList.begin()};
            
            // 检查大小限制
            if (lruList.size() > MAX_CACHE_SIZE) {
                std::string oldest = lruList.back();
                lruList.pop_back();
                blobCache.erase(oldest);
            }
        }
    }
    
    bool getBlob(const std::string& hash, Blob& blob) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = blobCache.find(hash);
        if (it != blobCache.end()) {
            updateLRU(hash);
            blob = it->second.first;
            return true;
        }
        return false;
    }
    
    // Tree 缓存操作
    void putTree(const std::string& hash, const Tree& tree) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = treeCache.find(hash);
        if (it != treeCache.end()) {
            it->second.first = tree;
            updateLRU(hash);
        } else {
            lruList.push_front(hash);
            treeCache[hash] = {tree, lruList.begin()};
            
            if (lruList.size() > MAX_CACHE_SIZE) {
                std::string oldest = lruList.back();
                lruList.pop_back();
                treeCache.erase(oldest);
            }
        }
    }
    
    bool getTree(const std::string& hash, Tree& tree) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = treeCache.find(hash);
        if (it != treeCache.end()) {
            updateLRU(hash);
            tree = it->second.first;
            return true;
        }
        return false;
    }
    
    // Commit 缓存操作
    void putCommit(const std::string& hash, const Commit& commit) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = commitCache.find(hash);
        if (it != commitCache.end()) {
            it->second.first = commit;
            updateLRU(hash);
        } else {
            lruList.push_front(hash);
            commitCache[hash] = {commit, lruList.begin()};
            
            if (lruList.size() > MAX_CACHE_SIZE) {
                std::string oldest = lruList.back();
                lruList.pop_back();
                commitCache.erase(oldest);
            }
        }
    }
    
    bool getCommit(const std::string& hash, Commit& commit) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = commitCache.find(hash);
        if (it != commitCache.end()) {
            updateLRU(hash);
            commit = it->second.first;
            return true;
        }
        return false;
    }
    
    // 清除所有缓存
    void clear() {
        std::lock_guard<std::mutex> lock(cacheMutex);
        blobCache.clear();
        treeCache.clear();
        commitCache.clear();
        lruList.clear();
    }
    
    // 获取缓存统计信息
    size_t getBlobCount() const {
        std::lock_guard<std::mutex> lock(cacheMutex);
        return blobCache.size();
    }
    
    size_t getTreeCount() const {
        std::lock_guard<std::mutex> lock(cacheMutex);
        return treeCache.size();
    }
    
    size_t getCommitCount() const {
        std::lock_guard<std::mutex> lock(cacheMutex);
        return commitCache.size();
    }
    
    // 获取总缓存数
    size_t getTotalCount() const {
        std::lock_guard<std::mutex> lock(cacheMutex);
        return blobCache.size() + treeCache.size() + commitCache.size();
    }
};

// 全局缓存实例
extern ObjectCache globalCache;

// 获取全局缓存
ObjectCache& getGlobalCache();

} // namespace storage
} // namespace versionctl

#endif // STORAGE_CACHE_H
