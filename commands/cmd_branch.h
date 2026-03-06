#ifndef COMMANDS_CMD_BRANCH_H
#define COMMANDS_CMD_BRANCH_H

#include <string>
#include <vector>

namespace versionctl {
namespace commands {

// 分支操作
bool cmdBranch(const std::string& root, const std::vector<std::string>& args);

// 检出操作
bool cmdCheckout(const std::string& root, const std::vector<std::string>& args);

} // namespace commands
} // namespace versionctl

#endif // COMMANDS_CMD_BRANCH_H
