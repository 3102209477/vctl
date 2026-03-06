#include <string>
#include <iostream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../config/config_manager.h"
#include "../config/ignore_rules.h"

namespace versionctl {
namespace commands {

// 初始化仓库
bool cmdInit(const std::string& root) {
    try {
        // 检查是否已初始化
        if (utils::isRepository(root)) {
            std::cout << "Reinitialized existing version control repository in " 
                      << utils::getVersionDir(root) << std::endl;
            return true;
        }
        
        // 创建版本控制目录结构
        std::string versionDir = utils::getVersionDir(root);
        std::string objectsDir = utils::getObjectsDir(root);
        std::string refsDir = utils::getRefsDir(root);
        std::string headsDir = utils::getHeadsDir(root);
        
        utils::createDirectories(objectsDir);
        utils::createDirectories(headsDir);
        
        // 初始化 HEAD 文件，指向 master 分支
        std::string headPath = utils::getHeadPath(root);
        utils::writeFile(headPath, "ref: refs/heads/master");
        
        // 初始化配置文件
        config::initializeRepositoryConfig(root);
        
        // 创建忽略规则文件
        std::string ignorePath = utils::getIgnorePath(root);
        RepositoryConfig config = config::loadRepositoryConfig(root);
        config::saveIgnoreRules(ignorePath, config);
        
        // 创建目标保护规则文件
        std::string protectPath = utils::getTargetIgnorePath(root);
        config::saveTargetProtectRules(protectPath, config);
        
        std::cout << "Initialized empty version control repository in " << versionDir << std::endl;
        std::cout << "  - Created directory structure" << std::endl;
        std::cout << "  - Initialized HEAD (master branch)" << std::endl;
        std::cout << "  - Created configuration file" << std::endl;
        std::cout << "  - Created default ignore rules" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error initializing repository: " << e.what() << std::endl;
        return false;
    }
}

} // namespace commands
} // namespace versionctl
