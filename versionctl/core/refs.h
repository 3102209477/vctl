#ifndef CORE_REFS_H
#define CORE_REFS_H

#include <string>
#include <vector>
#include "../include/types.h"

namespace versionctl {
namespace core {

// 获取所有本地分支
std::vector<BranchInfo> getLocalBranches(const std::string& root);

// 创建新分支
bool createBranch(const std::string& root, const std::string& branchName, 
                  const std::string& startPoint = "HEAD");

// 删除分支
bool deleteBranch(const std::string& root, const std::string& branchName);

// 切换分支
bool checkoutBranch(const std::string& root, const std::string& branchName);

// 获取当前分支名称
std::string getCurrentBranch(const std::string& root);

// 设置当前分支
bool setCurrentBranch(const std::string& root, const std::string& branchName);

// 获取 HEAD 指向的 commit hash
std::string getHeadCommit(const std::string& root);

// 更新分支引用
bool updateBranchRef(const std::string& root, const std::string& branchName, 
                    const std::string& commitHash);

// 更新 HEAD
bool updateHEAD(const std::string& root, const std::string& branchName);

// 读取 Commit 对象
Commit readCommitObject(const std::string& root, const std::string& hash);

// 获取提交历史
std::vector<Commit> getCommitHistory(const std::string& root, const std::string& startHash, int maxCount);

} // namespace core
} // namespace versionctl

#endif // CORE_REFS_H
