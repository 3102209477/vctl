#include <string>
#include <iostream>
#include <filesystem>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.h"
#include "../core/index.h"
#include "../core/refs.h"
#include "../config/config_manager.h"
#include "../config/ignore_rules.h"

namespace versionctl {
namespace commands {

// 显示工作状态（使用 index）
void cmdStatus(const std::string& root) {
    try {
        if (!utils::isRepository(root)) {
            std::cout << "Not a version control repository." << std::endl;
            return;
        }
        
        // 加载 index
        core::Index& index = core::getIndex();
        if (!index.load(root)) {
            std::cout << "No index found. Working tree clean." << std::endl;
            return;
        }
        
        RepositoryConfig config = config::loadRepositoryConfig(root);
        std::string currentBranch = core::getCurrentBranch(root);
        
        if (currentBranch.empty()) {
            currentBranch = "master";
        }
        
        std::cout << "On branch " << currentBranch << "\n\n";
        
        // 暂存区状态
        if (!index.getEntries().empty()) {
            std::cout << "Changes to be committed:\n";
            for (const auto& entry : index.getEntries()) {
                std::cout << "        " << entry.path << "\n";
            }
            std::cout << "\n";
        } else {
            std::cout << "Nothing to commit, working tree clean.\n\n";
        }
        
        // 检查工作目录变更
        int modifiedCount = 0;
        int untrackedCount = 0;
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            std::string fullPath = entry.path().string();
            std::string relativePath = utils::relativePath(fullPath, root);
            
            // 跳过版本控制目录和忽略的文件
            if (utils::startsWith(relativePath, VERSION_DIR) || 
                utils::startsWith(relativePath, ".git") ||
                config::shouldIgnore(relativePath, config)) {
                continue;
            }
            
            if (!entry.is_regular_file()) {
                continue;
            }
            
            // 检查是否在 index 中
            const core::IndexEntry* cachedEntry = index.getEntry(relativePath);
            
            if (cachedEntry != nullptr) {
                // 文件已跟踪，检查是否修改
                if (utils::hasFileChanged(fullPath, cachedEntry->mtime, cachedEntry->size)) {
                    if (modifiedCount == 0) {
                        std::cout << "Changes not staged for commit:\n";
                    }
                    std::cout << "        modified: " << relativePath << "\n";
                    modifiedCount++;
                }
            } else {
                // 未跟踪的文件
                if (untrackedCount == 0) {
                    std::cout << "Untracked files:\n";
                }
                std::cout << "        " << relativePath << "\n";
                untrackedCount++;
            }
        }
        
        if (modifiedCount == 0 && untrackedCount == 0 && index.getEntries().empty()) {
            std::cout << "nothing to commit, working tree clean\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error getting status: " << e.what() << std::endl;
    }
}

} // namespace commands
} // namespace versionctl
