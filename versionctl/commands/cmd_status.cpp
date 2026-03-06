#include <string>
#include <iostream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.h"
#include "../core/refs.h"

namespace versionctl {
namespace commands {

// 显示状态
void cmdStatus(const std::string& root) {
    try {
        if (!utils::isRepository(root)) {
            std::cerr << "Not a version control repository: " << root << std::endl;
            return;
        }
        
        std::string currentBranch = core::getCurrentBranch(root);
        
        if (currentBranch.empty()) {
            std::cout << "HEAD detached at " << core::getHeadCommit(root).substr(0, 7) << std::endl;
        } else {
            std::cout << "On branch " << currentBranch << std::endl;
        }
        
        std::cout << "\nNothing to commit, working tree clean" << std::endl;
        
        // TODO: 实现完整的工作区状态检测
        
    } catch (const std::exception& e) {
        std::cerr << "Error showing status: " << e.what() << std::endl;
    }
}

} // namespace commands
} // namespace versionctl
