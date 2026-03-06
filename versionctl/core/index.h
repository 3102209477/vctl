#ifndef CORE_INDEX_H
#define CORE_INDEX_H

#include <string>
#include <vector>
#include <ctime>
#include "../include/types.h"

namespace versionctl {
namespace core {

// Index 条目结构
struct IndexEntry {
    std::string path;           // 相对路径
    std::string hash;           // Blob 哈希
    std::time_t mtime;          // 最后修改时间
    size_t size;                // 文件大小
    uint32_t flags;             // 标志位
    
    IndexEntry() : mtime(0), size(0), flags(0) {}
    IndexEntry(const std::string& p, const std::string& h, std::time_t m, size_t s)
        : path(p), hash(h), mtime(m), size(s), flags(0) {}
};

// Index 类 - 暂存区管理
class Index {
private:
    std::vector<IndexEntry> entries;
    std::string indexPath;      // index 文件路径
    bool dirty;                 // 是否有未保存的变更
    
public:
    Index();
    
    // 初始化 index
    bool load(const std::string& root);
    bool save(const std::string& root) const;
    
    // 添加/移除条目
    void add(const std::string& path, const std::string& hash, 
             std::time_t mtime, size_t size);
    bool remove(const std::string& path);
    void clear();
    
    // 查询操作
    bool hasFile(const std::string& path) const;
    const IndexEntry* getEntry(const std::string& path) const;
    const std::vector<IndexEntry>& getEntries() const { return entries; }
    
    // 获取已暂存的文件列表
    std::vector<std::string> getStagedFiles() const;
    
    // 检查是否有变更
    bool isDirty() const { return dirty; }
    
    // 从 index 构建 tree
    Tree buildTree() const;
};

// 获取全局 index 实例
Index& getIndex();

} // namespace core
} // namespace versionctl

#endif // CORE_INDEX_H
