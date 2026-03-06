# Version Control Tool - Project Package

## 📦 项目打包信息

**打包时间**: 2026-03-06  
**版本**: vctl.exe (最新稳定版)  
**大小**: ~720 KB  

---

## 📁 目录结构

```
vctl_project_YYYYMMDD_HHMMSS/
│
├── 📄 main.cpp                           # 主程序入口
├── 📄 test_git_format.cpp                # Git 格式测试工具
│
├── 📂 commands/                          # 命令实现
│   ├── cmd_init.cpp/h                    # 初始化命令
│   ├── cmd_add.cpp/h                     # 添加文件命令
│   ├── cmd_commit.cpp/h                  # 提交命令
│   ├── cmd_status.cpp/h                  # 状态查询
│   ├── cmd_log.cpp/h                     # 日志查看
│   ├── cmd_branch.cpp/h                  # 分支管理
│   └── cmd_clone.cpp/h                   # 克隆/同步命令 ⭐
│
├── 📂 core/                              # 核心功能
│   ├── objects.cpp/h                     # 对象管理（Blob/Tree/Commit）
│   ├── refs.cpp/h                        # 引用管理（分支/HEAD）
│   ├── index.cpp/h                       # 暂存区管理
│   └── daemon.cpp/h                      # 后台服务
│
├── 📂 storage/                           # 存储层
│   ├── hash.cpp/h                        # SHA-256 哈希
│   ├── binary_io.cpp/h                   # 二进制读写
│   ├── cache.cpp/h                       # LRU 缓存
│   └── compression.cpp/h                 # zlib 压缩 ⭐
│
├── 📂 config/                            # 配置管理
│   ├── config_manager.cpp/h              # 配置解析
│   └── ignore_rules.cpp/h                # 忽略规则
│
├── 📂 utils/                             # 工具函数
│   ├── path_utils.cpp/h                  # 路径处理
│   └── platform.cpp/h                    # 平台适配
│
├── 📂 include/                           # 头文件
│   ├── types.h                           # 类型定义
│   ├── constants.h                       # 常量定义
│   ├── utils.h                           # 工具宏
│   └── errors.h                          # 错误处理
│
├── 📂 bin/                               # 可执行文件 ⭐
│   ├── vctl.exe                          # 主程序 (737 KB)
│   └── test_format.exe                   # 测试工具 (286 KB)
│
├── 📄 README.md                          # 本说明文档
├── 📄 PERFORMANCE_OPTIMIZATIONS.md       # 性能优化说明
├── 📄 OPTIMIZATION_COMPLETE.md           # 优化完成总结
├── 📄 GIT_FORMAT_IMPLEMENTATION.md       # Git 格式实现文档
├── 📄 GIT_COMPATIBILITY.md               # Git 兼容性说明
├── 📄 CLONE_FEATURE.md                   # Clone 功能文档
├── 📄 STRESS_TEST_REPORT.md              # 压力测试报告
├── 📄 PERFORMANCE_TEST_RESULTS.md        # 性能测试结果
├── 📄 stress_test.bat                    # 压力测试脚本
└── 📄 test_performance.bat               # 性能测试脚本
```

---

## 🚀 快速开始

### 环境要求

- **操作系统**: Windows 10/11
- **编译器**: g++ (MinGW) with C++17 support
- **依赖库**:
  - zlib (压缩库)
  - crypt32 (Windows 加密 API)

### 编译项目

```bash
cd vctl_project_YYYYMMDD_HHMMSS

# 编译主程序
g++ -std=c++17 -Wall -Wextra -O2 -D_WIN32_WINNT=0x0A00 \
    -Iinclude \
    main.cpp \
    utils\path_utils.cpp \
    utils\platform.cpp \
    storage\hash.cpp \
    storage\binary_io.cpp \
    storage\cache.cpp \
    storage\compression.cpp \
    core\objects.cpp \
    core\refs.cpp \
    core\index.cpp \
    core\daemon.cpp \
    config\config_manager.cpp \
    config\ignore_rules.cpp \
    commands\cmd_init.cpp \
    commands\cmd_add.cpp \
    commands\cmd_commit.cpp \
    commands\cmd_status.cpp \
    commands\cmd_log.cpp \
    commands\cmd_branch.cpp \
    commands\cmd_clone.cpp \
    -lcrypt32 -lz \
    -o vctl.exe

# 编译测试工具（可选）
g++ -std=c++17 -O2 test_git_format.cpp storage\compression.cpp -lz -o test_format.exe
```

### 使用已编译的可执行文件

```bash
# 进入 bin 目录
cd bin

# 查看版本
.\vctl.exe --version

# 初始化仓库
.\vctl.exe init [project_path]

# 克隆/同步项目
.\vctl.exe clone <source> <target>
```

---

## 🎯 核心功能

### 1. **版本控制基础**

- ✅ `init` - 初始化仓库
- ✅ `add` - 添加文件到暂存区
- ✅ `commit` - 提交更改
- ✅ `status` - 查看状态
- ✅ `log` - 查看历史

### 2. **分支管理**

- ✅ `branch` - 创建/切换分支
- ✅ 独立的 HEAD 指针
- ✅ 多分支并行开发

### 3. **Clone/Sync** ⭐ NEW

- ✅ 智能克隆（新目录）
- ✅ 增量同步（已存在目录）
- ✅ 保留 Git 仓库（不冲突）
- ✅ 只更新工作树文件

### 4. **Git 兼容** ⭐

- ✅ Git 格式对象存储
- ✅ zlib 压缩（节省 60-80% 空间）
- ✅ SHA-256 哈希
- ✅ .gitignore 自动生成
- ✅ 避免与 `.git` 目录冲突

### 5. **性能优化** ⭐⭐

- ✅ LRU 缓存（加速 2.4 倍）
- ✅ mtime 快速检测
- ✅ 内容寻址去重
- ✅ 线程安全设计

---

## 📊 性能数据

| 操作 | 数据规模 | 耗时 | 优化效果 |
|------|---------|------|----------|
| Add (冷) | 100 文件 | 240ms | - |
| Add (热) | 50 变更 +50 未变 | ~100ms | **2.4x** 🔥 |
| Commit | 100 文件 | 58ms | ⭐ |
| Status | 100 文件 | 163ms | ⭐ |
| Clone/Sync | 100+ 文件 | ~200ms | ⭐⭐⭐ |
| 大文件 | 1MB | 181ms | ⭐⭐⭐⭐ |

---

## 💡 使用示例

### 初始化项目

```bash
# 初始化当前目录
vctl init

# 或指定目录
vctl init D:\MyProject
```

### 添加和提交

```bash
# 添加所有文件
vctl add .

# 添加特定文件
vctl add src/main.cpp

# 提交
vctl commit -m "Initial commit"
```

### 分支操作

```bash
# 创建并切换分支
vctl checkout -b feature-xyz

# 查看分支
vctl branch

# 切回 master
vctl checkout master
```

### Clone/Sync ⭐

```bash
# 克隆到新目录（首次）
vctl clone D:\Source D:\Target

# 同步到已存在目录（保留 Git）
vctl clone D:\Source D:\Existing_Git_Project
```

---

## 🛠️ 技术架构

### 分层设计

```
┌─────────────────────────┐
│   Commands Layer        │  ← cmd_*.cpp (用户接口)
├─────────────────────────┤
│   Core Layer            │  ← objects, refs, index
├─────────────────────────┤
│   Storage Layer         │  ← hash, binary_io, cache, compression
├─────────────────────────┤
│   Utils Layer           │  ← path_utils, platform
└─────────────────────────┘
```

### 对象模型

```
Blob (文件内容)
  ↓
Tree (目录结构)
  ↓
Commit (提交记录)
  ↓
Ref (分支指针)
```

### 存储格式

```
<object-type> <size>\0<compressed-content>
```

- 使用 zlib 压缩
- SHA-256 校验
- 内容寻址存储

---

## 📝 开发日志

### 2026-03-06: 完整功能实现

- ✅ Git 格式对象存储
- ✅ zlib 压缩支持
- ✅ Clone/Sync 智能同步
- ✅ Git 兼容性改进
- ✅ 性能优化（LRU 缓存）

### 2026-03-05: 核心功能

- ✅ 基础版本控制
- ✅ 分支管理
- ✅ 暂存区实现

---

## 🧪 测试

### 运行压力测试

```bash
cd vctl_project
.\stress_test.bat
```

### 查看测试报告

- [STRESS_TEST_REPORT.md](STRESS_TEST_REPORT.md) - 详细测试报告
- [PERFORMANCE_TEST_RESULTS.md](PERFORMANCE_TEST_RESULTS.md) - 性能数据

---

## 📋 系统要求

**最低要求**:

- Windows 10
- 256 MB RAM
- 100 MB 磁盘空间

**推荐配置**:

- Windows 11
- 512 MB RAM
- 500 MB 磁盘空间

---

## 🙏 致谢

**依赖库**:

- [zlib](https://www.zlib.net/) - 数据压缩
- MinGW-w64 - Windows GCC 工具链

**技术参考**:

- Git 对象格式
- SHA-256 标准
- LRU 缓存算法

---

## 📄 许可证

本项目仅供学习和研究使用。

---

## 👨‍💻 作者信息

**开发者**: s3102  
**日期**: 2026-03-06  
**版本**: v1.0.0  
**联系**: <unknown@example.com>  

---

<div align="center">

**🎉 打包完成！祝使用愉快！**

[📦 返回目录](#vctl-project---project-package)

</div>
