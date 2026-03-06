#ifndef COMMANDS_CMD_CLONE_H
#define COMMANDS_CMD_CLONE_H

#include <string>

namespace versionctl {
namespace commands {

// 克隆仓库（本地复制）
bool cmdClone(const std::string& source, const std::string& target);

} // namespace commands
} // namespace versionctl

#endif // COMMANDS_CMD_CLONE_H
