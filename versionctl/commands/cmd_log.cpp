#include <string>
#include <iostream>
#include <iomanip>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.cpp"
#include "../core/refs.cpp"

namespace versionctl {
namespace commands {

// 显示提交历史
void cmdLog(const std::string& root, int maxCount = 10) {
    try {
        if (!utils::isRepository(root)) {
            std::cerr << "Not a version control repository: " << root << std::endl;
            return;
        }
        
        std::vector<Commit> history = core::getCommitHistory(root, "", maxCount);
        
        if (history.empty()) {
            std::cout << "No commits yet." << std::endl;
            return;
        }
        
        for (const auto& commit : history) {
            std::cout << "\ncommit " << commit.hash << std::endl;
            
            if (commit.parents.size() > 1) {
                std::cout << "Merge: ";
                for (size_t i = 0; i < std::min(commit.parents.size(), size_t(2)); i++) {
                    std::cout << commit.parents[i].substr(0, 7) << " ";
                }
                std::cout << std::endl;
            }
            
            std::cout << "Author: " << commit.author << std::endl;
            std::cout << "Date:   " << utils::formatTimestamp(commit.timestamp) << std::endl;
            std::cout << std::endl;
            
            // 格式化提交消息
            std::stringstream ss(commit.message);
            std::string line;
            bool firstLine = true;
            while (std::getline(ss, line, '\n')) {
                if (firstLine) {
                    std::cout << "    " << line << std::endl;
                    firstLine = false;
                } else {
                    std::cout << "    " << line << std::endl;
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error showing log: " << e.what() << std::endl;
    }
}

} // namespace commands
} // namespace versionctl
