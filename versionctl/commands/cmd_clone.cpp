#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../core/objects.h"
#include "../core/refs.h"
#include "../config/config_manager.h"

namespace versionctl {
namespace commands {

// 递归复制目录（排除不需要的文件）
bool copyDirectory(const std::string& source, const std::string& target) {
    try {
        // 创建目标目录
        if (!std::filesystem::create_directories(target)) {
            std::cerr << "Failed to create directory: " << target << std::endl;
            return false;
        }
        
        // 遍历源目录
        for (const auto& entry : std::filesystem::recursive_directory_iterator(source)) {
            std::string relativePath = utils::relativePath(entry.path().string(), source);
            
            // 跳过系统文件和临时文件
            if (utils::startsWith(relativePath, "$") ||
                utils::endsWith(relativePath, ".tmp")) {
                continue;
            }
            
            std::string targetPath = utils::joinPath(target, relativePath);
            
            if (entry.is_regular_file()) {
                // 确保父目录存在
                std::filesystem::path fsPath(targetPath);
                if (fsPath.has_parent_path()) {
                    std::filesystem::create_directories(fsPath.parent_path());
                }
                
                // 复制文件
                std::filesystem::copy_file(entry.path(), targetPath, 
                                          std::filesystem::copy_options::overwrite_existing);
            } else if (entry.is_directory()) {
                std::filesystem::create_directories(targetPath);
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error copying directory: " << e.what() << std::endl;
        return false;
    }
}

// 克隆仓库（本地复制）
bool cmdClone(const std::string& source, const std::string& target) {
    try {
        // 验证源路径
        if (!utils::fileExists(source)) {
            std::cerr << "Source path does not exist: " << source << std::endl;
            return false;
        }
        
        // 检查是否是有效的版本控制仓库
        if (!utils::isRepository(source)) {
            std::cerr << "Source is not a version control repository: " << source << std::endl;
            return false;
        }
        
        // 检查目标是否已存在
        if (utils::fileExists(target)) {
            std::cerr << "Target path already exists: " << target << std::endl;
            return false;
        }
        
        std::cout << "Cloning from '" << source << "' to '" << target << "'" << std::endl;
        
        // 创建目标目录
        if (!std::filesystem::create_directories(target)) {
            std::cerr << "Failed to create target directory" << std::endl;
            return false;
        }
        
        // 复制工作树文件（排除版本控制元数据）
        int fileCount = 0;
        int dirCount = 0;
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(source)) {
            std::string relativePath = utils::relativePath(entry.path().string(), source);
            
            // 跳过 .version 目录（版本控制元数据）
            // 跳过 .git 目录（Git 元数据，避免与未来远程仓库冲突）
            // 跳过 .svn 目录（SVN 元数据）
            if (utils::startsWith(relativePath, VERSION_DIR + "/") || 
                utils::startsWith(relativePath, VERSION_DIR + "\\") ||
                utils::startsWith(relativePath, ".git/") ||
                utils::startsWith(relativePath, ".git\\") ||
                utils::startsWith(relativePath, ".svn/") ||
                utils::startsWith(relativePath, ".svn\\") ||
                utils::startsWith(relativePath, ".hg/") ||
                utils::startsWith(relativePath, ".hg\\")) {  // Mercurial
                continue;
            }
            
            // 跳过系统文件和临时文件
            if (utils::startsWith(relativePath, "$") ||
                utils::endsWith(relativePath, ".tmp") ||
                utils::endsWith(relativePath, ".swp")) {
                continue;
            }
            
            // 保留重要的配置文件（如 .gitignore, .editorconfig 等）
            bool isConfigFile = utils::startsWith(entry.path().filename().string(), ".");
            
            std::string targetPath = utils::joinPath(target, relativePath);
            
            if (entry.is_regular_file()) {
                // 确保父目录存在
                std::filesystem::path fsPath(targetPath);
                if (fsPath.has_parent_path()) {
                    std::filesystem::create_directories(fsPath.parent_path());
                }
                
                // 复制文件
                std::filesystem::copy_file(entry.path(), targetPath, 
                                          std::filesystem::copy_options::overwrite_existing);
                fileCount++;
            } else if (entry.is_directory()) {
                std::filesystem::create_directories(targetPath);
                dirCount++;
            }
        }
        
        // 复制 .version 目录（完整的版本历史）
        std::string sourceVersionDir = utils::getVersionDir(source);
        std::string targetVersionDir = utils::getVersionDir(target);
        
        if (utils::fileExists(sourceVersionDir)) {
            // 如果目标已存在，先删除（避免 copyDirectory 失败）
            if (std::filesystem::exists(targetVersionDir)) {
                std::filesystem::remove_all(targetVersionDir);
            }
            
            if (!copyDirectory(sourceVersionDir, targetVersionDir)) {
                std::cerr << "Failed to copy version history" << std::endl;
                return false;
            }
        }
        
        // 创建 REMOTE_README 文件，说明如何配置远程 Git 仓库
        std::string readmePath = utils::joinPath(target, "REMOTE_README.txt");
        std::ofstream readmeFile(readmePath);
        if (readmeFile) {
            readmeFile << "# Remote Repository Setup Guide\n";
            readmeFile << "\n";
            readmeFile << "This repository was cloned using vctl (local version control).\n";
            readmeFile << "To push to a remote Git repository, follow these steps:\n";
            readmeFile << "\n";
            readmeFile << "## Option 1: Initialize as Git Repository\n";
            readmeFile << "\n";
            readmeFile << "```bash\n";
            readmeFile << "# Remove .version directory (vctl metadata)\n";
            readmeFile << "rm -rf .version\n";
            readmeFile << "\n";
            readmeFile << "# Initialize Git\n";
            readmeFile << "git init\n";
            readmeFile << "\n";
            readmeFile << "# Add all files and commit\n";
            readmeFile << "git add .\n";
            readmeFile << "git commit -m \"Initial commit from vctl repository\"\n";
            readmeFile << "\n";
            readmeFile << "# Add remote and push\n";
            readmeFile << "git remote add origin <your-git-repo-url>\n";
            readmeFile << "git push -u origin master\n";
            readmeFile << "```\n";
            readmeFile << "\n";
            readmeFile << "## Option 2: Keep Both Version Systems\n";
            readmeFile << "\n";
            readmeFile << "You can keep both .version (for local backups) and .git (for remote sync).\n";
            readmeFile << "The .version directory is already excluded from Git via .gitignore.\n";
            readmeFile << "\n";
            readmeFile << "## Notes\n";
            readmeFile << "- The .version directory contains your complete local version history\n";
            readmeFile << "- Git integration is planned for future versions\n";
            readmeFile << "- See documentation for advanced usage\n";
            readmeFile.close();
        }
        
        // 更新配置（如果需要）
        config::loadRepositoryConfig(target);
        
        std::cout << "Clone completed successfully!" << std::endl;
        std::cout << "  - Copied " << fileCount << " file(s)" << std::endl;
        std::cout << "  - Created " << dirCount << " directory/directories" << std::endl;
        std::cout << "  - Preserved complete version history" << std::endl;
        
        // 显示克隆后的状态
        std::string currentBranch = core::getCurrentBranch(target);
        if (!currentBranch.empty()) {
            std::cout << "  - Current branch: " << currentBranch << std::endl;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error cloning repository: " << e.what() << std::endl;
        return false;
    }
}

} // namespace commands
} // namespace versionctl
