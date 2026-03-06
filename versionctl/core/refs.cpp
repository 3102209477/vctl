#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"

namespace versionctl {
namespace core {

// 前向声明
std::string getCurrentBranch(const std::string& root);
std::string getHeadCommit(const std::string& root);
bool updateBranchRef(const std::string& root, const std::string& branchName, const std::string& commitHash);
bool updateHEAD(const std::string& root, const std::string& branchName);
Commit readCommitObject(const std::string& root, const std::string& hash);

// 获取所有本地分支
std::vector<BranchInfo> getLocalBranches(const std::string& root) {
    std::vector<BranchInfo> branches;
    
    std::string headsDir = utils::getHeadsDir(root);
    
    if (!utils::fileExists(headsDir)) {
        return branches;
    }
    
    std::string currentBranch = getCurrentBranch(root);
    
    // 遍历所有分支引用文件
    for (const auto& entry : std::filesystem::directory_iterator(headsDir)) {
        if (entry.is_regular_file()) {
            std::string branchName = entry.path().filename().string();
            std::string content = utils::readFile(entry.path().string());
            std::string commitHash = utils::trim(content);
            
            bool isCurrent = (branchName == currentBranch);
            branches.emplace_back(branchName, commitHash, isCurrent);
        }
    }
    
    return branches;
}

// 创建新分支
bool createBranch(const std::string& root, const std::string& branchName, 
                  const std::string& startPoint = "HEAD") {
    // 检查分支名是否合法
    if (branchName.empty() || branchName.find('/') != std::string::npos) {
        std::cerr << "Invalid branch name: " << branchName << std::endl;
        return false;
    }
    
    // 检查分支是否已存在
    std::string refPath = utils::getHeadsDir(root) + "/" + branchName;
    if (utils::fileExists(refPath)) {
        std::cerr << "Branch '" << branchName << "' already exists." << std::endl;
        return false;
    }
    
    // 确定起始点
    std::string commitHash;
    if (startPoint == "HEAD") {
        commitHash = getHeadCommit(root);
    } else if (utils::isValidHash(startPoint)) {
        commitHash = startPoint;
    } else {
        // 尝试从其他分支获取
        std::string otherRef = utils::getHeadsDir(root) + "/" + startPoint;
        if (utils::fileExists(otherRef)) {
            commitHash = utils::trim(utils::readFile(otherRef));
        }
    }
    
    if (commitHash.empty()) {
        std::cerr << "Cannot create branch: invalid start point." << std::endl;
        return false;
    }
    
    // 创建分支引用
    return updateBranchRef(root, branchName, commitHash);
}

// 删除分支
bool deleteBranch(const std::string& root, const std::string& branchName) {
    std::string currentBranch = getCurrentBranch(root);
    
    // 不能删除当前分支
    if (branchName == currentBranch) {
        std::cerr << "Cannot delete the currently checked out branch: " << branchName << std::endl;
        return false;
    }
    
    std::string refPath = utils::getHeadsDir(root) + "/" + branchName;
    
    if (!utils::fileExists(refPath)) {
        std::cerr << "Branch '" << branchName << "' not found." << std::endl;
        return false;
    }
    
    return utils::deleteFile(refPath);
}

// 切换到分支
bool checkoutBranch(const std::string& root, const std::string& branchName) {
    // 检查分支是否存在
    std::string refPath = utils::getHeadsDir(root) + "/" + branchName;
    
    if (!utils::fileExists(refPath)) {
        std::cerr << "Branch '" << branchName << "' not found." << std::endl;
        return false;
    }
    
    // 更新 HEAD 引用
    return updateHEAD(root, branchName);
}

// 获取提交历史
std::vector<Commit> getCommitHistory(const std::string& root, 
                                     const std::string& startHash = "",
                                     int maxCount = -1) {
    std::vector<Commit> history;
    
    std::string currentHash = startHash;
    if (currentHash.empty()) {
        currentHash = getHeadCommit(root);
    }
    
    while (!currentHash.empty()) {
        Commit commit = readCommitObject(root, currentHash);
        
        if (commit.tree.empty()) {
            break; // 读取失败
        }
        
        history.push_back(commit);
        
        // 达到最大数量限制
        if (maxCount > 0 && static_cast<int>(history.size()) >= maxCount) {
            break;
        }
        
        // 移动到父提交
        if (commit.parents.empty()) {
            break;
        }
        
        currentHash = commit.parents[0];
    }
    
    return history;
}

// 获取指定分支的提交历史
std::vector<Commit> getBranchHistory(const std::string& root, 
                                     const std::string& branchName,
                                     int maxCount = -1) {
    std::string refPath = utils::getHeadsDir(root) + "/" + branchName;
    
    if (!utils::fileExists(refPath)) {
        return {};
    }
    
    std::string commitHash = utils::trim(utils::readFile(refPath));
    return getCommitHistory(root, commitHash, maxCount);
}

// 设置当前分支名称
bool setCurrentBranch(const std::string& root, const std::string& branchName) {
    std::string headPath = utils::getHeadPath(root);
    
    // 确保目录存在
    utils::createDirectories(std::filesystem::path(headPath).parent_path().string());
    
    std::ofstream file(headPath);
    if (!file) {
        return false;
    }
    
    file << "ref: refs/heads/" << branchName;
    return true;
}

} // namespace core
} // namespace versionctl
