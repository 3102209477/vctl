#include <string>
#include <iostream>
#include <filesystem>
#include <set>
#include <functional>
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
        
        // 读取当前 HEAD 提交文件列表，用于判断已提交文件是否为清洁状态
        std::set<std::string> committedFiles;
        std::string headCommit = core::getHeadCommit(root);
        if (!headCommit.empty()) {
            std::function<void(const std::string&, const std::string&, const std::string&)> collectFiles;
            collectFiles = [&](const std::string& repoRoot, const std::string& treeHash, const std::string& prefix) {
                if (treeHash.empty()) {
                    return;
                }
                Tree tree = core::readTree(repoRoot, treeHash);
                for (const auto& entry : tree.entries) {
                    std::string path = prefix.empty() ? entry.name : prefix + "/" + entry.name;
                    if (entry.type == ObjectType::TREE) {
                        collectFiles(repoRoot, entry.hash, path);
                    } else {
                        committedFiles.insert(path);
                    }
                }
            };
            Commit head = core::readCommitObject(root, headCommit);
            if (!head.tree.empty()) {
                collectFiles(root, head.tree, "");
            }
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
            } else if (committedFiles.find(relativePath) == committedFiles.end()) {
                // 未跟踪的文件，但如果它已经是 HEAD 提交的一部分则视为已清洁
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
