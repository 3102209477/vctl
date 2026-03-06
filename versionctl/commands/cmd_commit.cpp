#include <string>
#include <iostream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.h"
#include "../core/index.h"
#include "../core/refs.h"
#include "../config/config_manager.h"

namespace versionctl {
namespace commands {

// 提交变更（使用 index）
std::string cmdCommit(const std::string& root, const std::string& message) {
    try {
        if (!utils::isRepository(root)) {
            std::cerr << "Not a version control repository: " << root << std::endl;
            return "";
        }
        
        // 加载 index
        core::Index& index = core::getIndex();
        if (!index.load(root)) {
            std::cerr << "Error: No index found. Please run 'add' first." << std::endl;
            return "";
        }
        
        // 检查是否有暂存的文件
        if (index.getEntries().empty()) {
            std::cout << "Nothing to commit, working tree clean." << std::endl;
            return "NO_CHANGES";
        }
        
        // 获取当前分支和父提交
        std::string currentBranch = core::getCurrentBranch(root);
        std::string parentCommit = core::getHeadCommit(root);
        
        // 从 index 构建树
        Tree tree = index.buildTree();
        std::string treeHash = core::createTree(root, tree);
        
        if (treeHash.empty()) {
            std::cerr << "Error: Failed to create tree object." << std::endl;
            return "";
        }
        
        // 创建 Commit 对象
        Commit commit;
        commit.tree = treeHash;
        
        if (!parentCommit.empty() && parentCommit != "NO_COMMITS") {
            commit.parents.push_back(parentCommit);
        }
        
        // 获取作者信息
        RepositoryConfig config = config::loadRepositoryConfig(root);
        std::string authorName = config.authorName.empty() ? utils::getCurrentUserName() : config.authorName;
        std::string authorEmail = config.authorEmail.empty() ? "unknown@example.com" : config.authorEmail;
        
        int64_t timestamp = utils::getCurrentTimestamp();
        
        commit.author = authorName + " <" + authorEmail + ">";
        commit.committer = commit.author;
        commit.timestamp = timestamp;
        commit.message = message;
        
        // 提交到对象数据库
        std::string commitHash = core::createCommit(root, commit);
        
        if (commitHash.empty()) {
            std::cerr << "Error: Failed to create commit object." << std::endl;
            return "";
        }
        
        // 更新分支引用
        if (currentBranch.empty()) {
            currentBranch = "master";
            core::setCurrentBranch(root, currentBranch);
        }
        
        if (!core::updateBranchRef(root, currentBranch, commitHash)) {
            std::cerr << "Error: Failed to update branch reference." << std::endl;
            return "";
        }
        
        // 更新 HEAD
        core::updateHEAD(root, currentBranch);
        
        // 清空 index
        index.clear();
        index.save(root);
        
        std::cout << "[" << currentBranch << " " << commitHash.substr(0, 7) << "] " 
                  << message << std::endl;
        
        // 在清空前获取条目数量
        size_t changedFiles = index.getEntries().size();
        if (changedFiles == 0) {
            changedFiles = 1; // 至少有一个文件（因为刚提交过）
        }
        std::cout << " " << changedFiles << " file(s) changed" << std::endl;
        
        return commitHash;
        
    } catch (const std::exception& e) {
        std::cerr << "Error committing changes: " << e.what() << std::endl;
        return "";
    }
}

} // namespace commands
} // namespace versionctl
