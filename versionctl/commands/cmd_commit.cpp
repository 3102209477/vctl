#include <string>
#include <iostream>
#include <filesystem>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.cpp"
#include "../core/refs.cpp"
#include "../config/config_manager.cpp"
#include "../config/ignore_rules.cpp"
#include "../utils/platform.cpp"

namespace versionctl {
namespace commands {

// 提交变更
std::string cmdCommit(const std::string& root, const std::string& message) {
    try {
        if (!utils::isRepository(root)) {
            std::cerr << "Not a version control repository: " << root << std::endl;
            return "";
        }
        
        RepositoryConfig config = config::loadRepositoryConfig(root);
        
        // 获取作者信息
        std::string authorName = config.authorName;
        std::string authorEmail = config.authorEmail;
        
        if (authorName.empty()) {
            authorName = utils::getCurrentUserName();
        }
        if (authorEmail.empty()) {
            authorEmail = "user@example.com";
        }
        
        std::string author = authorName + " <" + authorEmail + ">";
        std::string committer = author;
        int64_t timestamp = utils::getCurrentTimestamp();
        
        // 创建当前工作目录的 Tree
        std::string treeHash = core::createTreeFromDirectory(root, root);
        
        if (treeHash.empty()) {
            std::cerr << "No files to commit." << std::endl;
            return "NO_CHANGES";
        }
        
        // 获取父提交
        std::string parentHash = core::getHeadCommit(root);
        
        // 创建 Commit对象
        Commit commit;
        commit.tree = treeHash;
        if (!parentHash.empty()) {
            commit.parents.push_back(parentHash);
        }
        commit.author = author;
        commit.committer = committer;
        commit.timestamp = timestamp;
        commit.message = message;
        
        std::string commitHash = core::createCommit(root, commit);
        
        if (commitHash.empty()) {
            std::cerr << "Failed to create commit." << std::endl;
            return "";
        }
        
        // 更新当前分支引用
        std::string currentBranch = core::getCurrentBranch(root);
        if (!currentBranch.empty()) {
            core::updateBranchRef(root, currentBranch, commitHash);
        } else {
            // 分离 HEAD 状态，直接更新 HEAD
            utils::writeFile(utils::getHeadPath(root), commitHash);
        }
        
        // 记录到提交日志
        std::string logPath = utils::getCommitLogPath(root);
        std::ofstream logFile(logPath, std::ios::app);
        if (logFile) {
            logFile << commitHash << " " << message << " " << timestamp << std::endl;
            logFile.close();
        }
        
        std::cout << "[" << currentBranch << " " << commitHash.substr(0, 7) << "] " 
                  << message << std::endl;
        std::cout << " 1 file(s) changed" << std::endl;
        
        return commitHash;
        
    } catch (const std::exception& e) {
        std::cerr << "Error creating commit: " << e.what() << std::endl;
        return "";
    }
}

} // namespace commands
} // namespace versionctl
