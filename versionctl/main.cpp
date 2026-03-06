#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "include/types.h"
#include "include/constants.h"
#include "include/utils.h"
#include "commands/cmd_init.h"
#include "commands/cmd_add.h"
#include "commands/cmd_commit.h"
#include "commands/cmd_status.h"
#include "commands/cmd_log.h"
#include "commands/cmd_branch.h"
#include "commands/cmd_clone.h"
#include "config/config_manager.h"

using namespace versionctl;

void printUsage() {
    std::cout << "Version Control System - A Git-like version control tool\n";
    std::cout << "\nUsage:\n";
    std::cout << "  vctl init                    Initialize a new repository\n";
    std::cout << "  vctl add <file|pattern>      Add files to staging area\n";
    std::cout << "  vctl commit -m \"message\"     Commit staged changes\n";
    std::cout << "  vctl status                  Show working tree status\n";
    std::cout << "  vctl log                     Show commit history\n";
    std::cout << "  vctl branch                  List branches\n";
    std::cout << "  vctl branch -b <name>        Create a new branch\n";
    std::cout << "  vctl branch -d <name>        Delete a branch\n";
    std::cout << "  vctl checkout <branch>       Switch branches\n";
    std::cout << "  vctl config <key> <value>    Set configuration\n";
    std::cout << "  vctl clone <source> [target] Clone a repository\n";
    std::cout << "\nExamples:\n";
    std::cout << "  vctl init\n";
    std::cout << "  vctl add .\n";
    std::cout << "  vctl commit -m \"Initial commit\"\n";
    std::cout << "  vctl branch -b feature-1\n";
    std::cout << "  vctl checkout feature-1\n";
    std::cout << "  vctl clone /path/to/repo my-clone\n";
}

int main(int argc, char* argv[]) {
    try {
        // 启用控制台颜色
        utils::enableConsoleColors();
        
        if (argc < 2) {
            printUsage();
            return 1;
        }
        
        std::string command = argv[1];
        std::string root = std::filesystem::current_path().string();
        
        if (command == "init") {
            if (!commands::cmdInit(root)) {
                return 1;
            }
        }
        else if (command == "add") {
            std::string pattern = ".";
            if (argc >= 3) {
                pattern = argv[2];
            }
            
            if (!commands::cmdAdd(root, pattern)) {
                return 1;
            }
        }
        else if (command == "commit") {
            std::string message;
            bool hasMessage = false;
            
            for (int i = 2; i < argc; i++) {
                std::string arg = argv[i];
                
                if ((arg == "-m" || arg == "--message") && i + 1 < argc) {
                    message = argv[++i];
                    hasMessage = true;
                }
            }
            
            if (!hasMessage) {
                std::cerr << "Error: commit requires a message (-m \"message\")" << std::endl;
                return 1;
            }
            
            std::string result = commands::cmdCommit(root, message);
            
            if (result.empty()) {
                return 1;
            } else if (result == "NO_CHANGES") {
                std::cout << "No changes to commit." << std::endl;
            }
        }
        else if (command == "status") {
            commands::cmdStatus(root);
        }
        else if (command == "log") {
            int maxCount = 10;
            
            for (int i = 2; i < argc; i++) {
                std::string arg = argv[i];
                
                if ((arg == "-n" || arg == "--max-count") && i + 1 < argc) {
                    maxCount = std::stoi(argv[++i]);
                }
            }
            
            commands::cmdLog(root, maxCount);
        }
        else if (command == "branch") {
            std::vector<std::string> args;
            
            for (int i = 2; i < argc; i++) {
                args.push_back(argv[i]);
            }
            
            if (!commands::cmdBranch(root, args)) {
                return 1;
            }
        }
        else if (command == "checkout") {
            std::vector<std::string> args;
            
            for (int i = 2; i < argc; i++) {
                args.push_back(argv[i]);
            }
            
            if (!commands::cmdCheckout(root, args)) {
                return 1;
            }
        }
        else if (command == "clone") {
            if (argc < 3) {
                std::cerr << "Usage: clone <source> [target]" << std::endl;
                std::cerr << "  source: path to the source repository" << std::endl;
                std::cerr << "  target: path for the cloned repository (optional, defaults to source name)" << std::endl;
                return 1;
            }
            
            std::string source = argv[2];
            std::string target = (argc >= 4) ? argv[3] : "";
            
            // 如果未指定目标路径，使用源路径的名称
            if (target.empty()) {
                std::filesystem::path sourcePath(source);
                target = sourcePath.filename().string();
                if (target.empty()) {
                    target = "clone";
                }
            }
            
            if (!commands::cmdClone(source, target)) {
                return 1;
            }
        }
        else if (command == "config") {
            if (argc < 4) {
                std::cout << "Usage: config <key> <value>" << std::endl;
                std::cout << "Keys: repository.name, user.name, user.email" << std::endl;
                return 1;
            }
            
            std::string key = argv[2];
            std::string value = argv[3];
            
            if (!config::setConfigValue(root, key, value)) {
                std::cerr << "Invalid configuration key: " << key << std::endl;
                return 1;
            }
            
            std::cout << "Set " << key << " = " << value << std::endl;
        }
        else if (command == "help" || command == "--help" || command == "-h") {
            printUsage();
        }
        else {
            std::cerr << "Unknown command: " << command << std::endl;
            std::cerr << "Run 'vctl help' for usage information." << std::endl;
            return 1;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
