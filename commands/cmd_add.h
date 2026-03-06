#ifndef COMMANDS_CMD_ADD_H
#define COMMANDS_CMD_ADD_H

#include <string>

namespace versionctl {
namespace commands {

// 添加文件到暂存区
bool cmdAdd(const std::string& root, const std::string& pathPattern = ".");

} // namespace commands
} // namespace versionctl

#endif // COMMANDS_CMD_ADD_H
