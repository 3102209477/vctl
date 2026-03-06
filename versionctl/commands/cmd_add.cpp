#include <string>
#include <iostream>
#include <filesystem>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.cpp"
#include "../config/config_manager.cpp"
#include "../config/ignore_rules.cpp"

namespace versionctl {
namespace commands {

// 添加文件到暂存区 (简化实现，直接添加到对象数据库)
bool cmdAdd(const std::string& root, const std::string& pathPattern = ".") {
    try {
        if (!utils::isRepository(root)) {
            std::cerr << "Not a version control repository: " << root << std::endl;
            return false;
        }
        
        RepositoryConfig config = config::loadRepositoryConfig(root);
        std::string currentPath = std::filesystem::current_path().string();
        
        int addedCount = 0;
        int skippedCount = 0;
        
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
                           utils::matchPattern(relativePath, pathPattern));
            
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
            
            // 读取文件内容并创建 Blob
            std::string content = utils::readFile(fullPath);
            std::string blobHash = core::createBlob(root, content);
            
            if (!blobHash.empty()) {
                std::cout << "Added: " << relativePath << " (" << blobHash.substr(0, 8) << ")" << std::endl;
                addedCount++;
            } else {
                std::cerr << "Failed to add: " << relativePath << std::endl;
            }
        }
        
        std::cout << "\nSummary:" << std::endl;
        std::cout << "  Added: " << addedCount << " file(s)" << std::endl;
        std::cout << "  Skipped: " << skippedCount << " file(s) (ignored)" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error adding files: " << e.what() << std::endl;
        return false;
    }
}

} // namespace commands
} // namespace versionctl
