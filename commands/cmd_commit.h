#ifndef COMMANDS_CMD_COMMIT_H
#define COMMANDS_CMD_COMMIT_H

#include <string>

namespace versionctl {
namespace commands {

// 提交变更
std::string cmdCommit(const std::string& root, const std::string& message);

} // namespace commands
} // namespace versionctl

#endif // COMMANDS_CMD_COMMIT_H
