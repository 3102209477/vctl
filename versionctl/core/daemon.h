#ifndef CORE_DAEMON_H
#define CORE_DAEMON_H

#include <string>
#include <atomic>

namespace versionctl {
namespace core {

// 守护进程管理器（简化版）
class DaemonManager {
private:
    static std::atomic<bool> running;
    
public:
    // 初始化守护进程（预留接口）
    static bool init();
    
    // 检查是否在守护进程模式下运行
    static bool isRunning() { return running.load(); }
    
    // 停止守护进程
    static void stop() { running.store(false); }
};

} // namespace core
} // namespace versionctl

#endif // CORE_DAEMON_H
