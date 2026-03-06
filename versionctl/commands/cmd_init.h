#ifndef COMMANDS_CMD_INIT_H
#define COMMANDS_CMD_INIT_H

#include <string>

namespace versionctl {
namespace commands {

// 初始化仓库
bool cmdInit(const std::string& root);

} // namespace commands
} // namespace versionctl

#endif // COMMANDS_CMD_INIT_H
