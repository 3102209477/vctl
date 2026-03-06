#ifndef COMMANDS_CMD_LOG_H
#define COMMANDS_CMD_LOG_H

#include <string>

namespace versionctl {
namespace commands {

// 显示提交历史
void cmdLog(const std::string& root, int maxCount = 10);

} // namespace commands
} // namespace versionctl

#endif // COMMANDS_CMD_LOG_H
