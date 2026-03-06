#include <string>
#include <iostream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.h"
#include "../core/refs.h"

namespace versionctl {
namespace commands {

// 分支操作
bool cmdBranch(const std::string& root, const std::vector<std::string>& args) {
    try {
        if (!utils::isRepository(root)) {
            std::cerr << "Not a version control repository: " << root << std::endl;
            return false;
        }
        
        if (args.empty()) {
            // 列出所有分支
            std::vector<BranchInfo> branches = core::getLocalBranches(root);
            
            for (const auto& branch : branches) {
                if (branch.isCurrent) {
                    std::cout << "* ";
                } else {
                    std::cout << "  ";
                }
                std::cout << branch.name << std::endl;
            }
            
            return true;
        }
        
        // 创建新分支
        if (args[0] == "-b" || args[0] == "--create") {
            if (args.size() < 2) {
                std::cerr << "Usage: branch -b <branch-name> [start-point]" << std::endl;
                return false;
            }
            
            std::string branchName = args[1];
            std::string startPoint = (args.size() >= 3) ? args[2] : "HEAD";
            
            if (core::createBranch(root, branchName, startPoint)) {
                std::cout << "Created branch '" << branchName << "'" << std::endl;
                return true;
            }
            return false;
        }
        
        // 删除分支
        if (args[0] == "-d" || args[0] == "--delete") {
            if (args.size() < 2) {
                std::cerr << "Usage: branch -d <branch-name>" << std::endl;
                return false;
            }
            
            if (core::deleteBranch(root, args[1])) {
                std::cout << "Deleted branch '" << args[1] << "'" << std::endl;
                return true;
            }
            return false;
        }
        
        // 其他情况视为创建分支 (不带参数)
        if (args.size() == 1) {
            if (core::createBranch(root, args[0])) {
                std::cout << "Created branch '" << args[0] << "'" << std::endl;
                return true;
            }
            return false;
        }
        
        return false;
        
    } catch (const std::exception& e) {
        std::cerr << "Error with branch operation: " << e.what() << std::endl;
        return false;
    }
}

// 切换分支或恢复文件
bool cmdCheckout(const std::string& root, const std::vector<std::string>& args) {
    try {
        if (!utils::isRepository(root)) {
            std::cerr << "Not a version control repository: " << root << std::endl;
            return false;
        }
        
        if (args.empty()) {
            std::cerr << "Usage: checkout <branch-name> | <commit-hash>" << std::endl;
            return false;
        }
        
        std::string target = args[0];
        
        // 检查是否是分支名
        std::string refPath = utils::getHeadsDir(root) + "/" + target;
        if (utils::fileExists(refPath)) {
            // 切换到分支
            if (core::checkoutBranch(root, target)) {
                std::cout << "Switched to branch '" << target << "'" << std::endl;
                
                // TODO: 更新工作区文件
                
                return true;
            }
            return false;
        }
        
        // 检查是否是提交哈希
        if (utils::isValidHash(target)) {
            // 分离 HEAD 状态
            if (core::updateHEAD(root, target)) {
                std::cout << "Note: switching to '" << target.substr(0, 7) << "'." << std::endl;
                std::cout << "You are in 'detached HEAD' state." << std::endl;
                
                // TODO: 更新工作区文件
                
                return true;
            }
            return false;
        }
        
        std::cerr << "Branch or commit not found: " << target << std::endl;
        return false;
        
    } catch (const std::exception& e) {
        std::cerr << "Error with checkout operation: " << e.what() << std::endl;
        return false;
    }
}

} // namespace commands
} // namespace versionctl
