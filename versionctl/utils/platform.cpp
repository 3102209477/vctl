#include <string>
#include "../include/types.h"

// 平台特定的头文件
#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
#else
    #include <unistd.h>
#endif

namespace versionctl {
namespace utils {

// 平台特定的工具函数实现
#ifdef _WIN32
    #include <windows.h>
#endif

// 获取当前用户名称
std::string getCurrentUserName() {
#ifdef _WIN32
    char username[256];
    DWORD size = sizeof(username);
    if (GetUserNameA(username, &size)) {
        return std::string(username, size - 1);
    }
    return "Unknown User";
#else
    const char* username = getenv("USER");
    if (username) {
        return std::string(username);
    }
    return "Unknown User";
#endif
}

// 获取主机名
std::string getHostName() {
#ifdef _WIN32
    char hostname[256];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size)) {
        return std::string(hostname, size);
    }
    return "localhost";
#else
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
    return "localhost";
#endif
}

// 设置环境变量
bool setEnvironmentVariable(const std::string& name, const std::string& value) {
#ifdef _WIN32
    return SetEnvironmentVariableA(name.c_str(), value.c_str()) != 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

// 获取环境变量
std::string getEnvironmentVariable(const std::string& name, const std::string& defaultValue = "") {
#ifdef _WIN32
    char* value = nullptr;
    size_t len = 0;
    _dupenv_s(&value, &len, name.c_str());
    if (value) {
        std::string result(value, len);
        free(value);
        return result;
    }
    return defaultValue;
#else
    const char* value = getenv(name.c_str());
    if (value) {
        return std::string(value);
    }
    return defaultValue;
#endif
}

// 检查是否在终端中
bool isTerminal() {
#ifdef _WIN32
    return _isatty(_fileno(stdin)) && _isatty(_fileno(stdout));
#else
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
#endif
}

// 控制台颜色输出支持
void enableConsoleColors() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

} // namespace utils
} // namespace versionctl
