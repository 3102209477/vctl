#include <string>
#include <iostream>
#include <filesystem>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.h"
#include "../core/index.h"
#include "../config/config_manager.h"
#include "../config/ignore_rules.h"

namespace versionctl {
namespace commands {

// 添加文件到暂存区（使用 index）
bool cmdAdd(const std::string& root, const std::string& pathPattern) {
    try {
        if (!utils::isRepository(root)) {
            return false;
        }
        
        // 加载 index
        core::Index& index = core::getIndex();
        index.load(root);
        
        RepositoryConfig config = config::loadRepositoryConfig(root);
        
        int addedCount = 0;
        int skippedCount = 0;
        int unchangedCount = 0;
        
        // 遍历文件
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            std::string fullPath = entry.path().string();
            std::string relativePath = utils::relativePath(fullPath, root);
            
            // 跳过版本控制目录
            if (utils::startsWith(relativePath, VERSION_DIR) || 
                utils::startsWith(relativePath, ".git")) {
                continue;
            }
            
            // 检查是否匹配路径模式
            bool matches = (pathPattern == "." || 
                           config::matchPattern(relativePath, pathPattern));
            
            if (!matches) {
                continue;
            }
            
            // 检查是否应被忽略
            if (config::shouldIgnore(relativePath, config)) {
                skippedCount++;
                continue;
            }
            
            // 只处理普通文件
            if (!entry.is_regular_file()) {
                continue;
            }
            
            // 获取文件信息
            std::time_t mtime = std::filesystem::last_write_time(entry.path()).time_since_epoch().count() / 10000000LL;
            size_t fileSize = entry.file_size();
            
            // 检查 index 中是否已有该文件且未变化
            const core::IndexEntry* cachedEntry = index.getEntry(relativePath);
            if (cachedEntry != nullptr && 
                !utils::hasFileChanged(fullPath, cachedEntry->mtime, cachedEntry->size)) {
                // 文件未变化，直接使用缓存的 hash
                index.add(relativePath, cachedEntry->hash, mtime, fileSize);
                unchangedCount++;
                continue;
            }
            
            // 读取文件内容并创建 Blob
            std::string content = utils::readFile(fullPath);
            std::string blobHash = core::createBlob(root, content);
            
            if (!blobHash.empty()) {
                // 添加到 index
                index.add(relativePath, blobHash, mtime, fileSize);
                addedCount++;
            }
        }
        
        // 保存 index
        index.save(root);

        std::cout << "\nSummary:" << std::endl;
        std::cout << "Added: " << addedCount << std::endl;
        std::cout << "Skipped: " << skippedCount << std::endl;
        std::cout << "Unchanged: " << unchangedCount << std::endl;

        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error adding files: " << e.what() << std::endl;
        return false;
    }
}

} // namespace commands
} // namespace versionctl
