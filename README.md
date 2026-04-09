# vctl

一个轻量级的本地版本控制系统，模仿 Git 的核心概念实现提交、分支、克隆、暂存区与对象存储。

## 项目目标

- 提供本地版本管理功能
- 使用内容寻址存储（CAS）保存对象
- 支持分支、提交、状态检查和克隆
- 通过简化的对象模型提高可读性与可维护性

## 特性

- `init`：初始化本地仓库
- `add`：添加文件到暂存区
- `commit`：提交暂存变更
- `status`：查看当前工作区状态
- `log`：查看历史提交
- `branch`：管理本地分支
- `checkout`：切换分支 / 分离 HEAD
- `clone`：本地仓库克隆与增量同步
- 自动对象缓存与压缩存储

## 快速编译

### Windows (MinGW)

```powershell
cd d:\Desktop\test\vctl
g++ -std=c++17 -Wall -O2 -Iinclude \
  main.cpp \
  utils/path_utils.cpp utils/platform.cpp \
  storage/hash.cpp storage/binary_io.cpp storage/cache.cpp storage/compression.cpp \
  core/objects.cpp core/refs.cpp core/index.cpp \
  config/config_manager.cpp config/ignore_rules.cpp \
  commands/cmd_init.cpp commands/cmd_add.cpp commands/cmd_commit.cpp \
  commands/cmd_status.cpp commands/cmd_log.cpp commands/cmd_branch.cpp commands/cmd_clone.cpp \
  -lcrypt32 -lz -o vctl.exe
```

### Linux / macOS

```bash
cd /path/to/vctl
g++ -std=c++17 -Wall -O2 -Iinclude \
  main.cpp \
  utils/path_utils.cpp utils/platform.cpp \
  storage/hash.cpp storage/binary_io.cpp storage/cache.cpp storage/compression.cpp \
  core/objects.cpp core/refs.cpp core/index.cpp \
  config/config_manager.cpp config/ignore_rules.cpp \
  commands/cmd_init.cpp commands/cmd_add.cpp commands/cmd_commit.cpp \
  commands/cmd_status.cpp commands/cmd_log.cpp commands/cmd_branch.cpp commands/cmd_clone.cpp \
  -lssl -lcrypto -lz -o vctl
```

> 依赖项：`zlib`、`OpenSSL`（Linux/macOS）或 `crypt32`（Windows）。

## 使用示例

### 初始化仓库

```bash
vctl init
```

### 添加并提交

```bash
vctl add .
vctl commit -m "Initial commit"
```

### 查看状态与历史

```bash
vctl status
vctl log
```

### 分支操作

```bash
vctl branch
vctl branch -b feature-1
vctl checkout feature-1
vctl branch -d feature-1
```

### 克隆仓库

```bash
vctl clone /path/to/source /path/to/target
```

## 命令参考

| 命令 | 说明 |
| --- | --- |
| `vctl init` | 初始化仓库 |
| `vctl add <file|pattern>` | 添加文件到暂存区 |
| `vctl commit -m "message"` | 提交暂存变更 |
| `vctl status` | 显示当前工作区状态 |
| `vctl log` | 显示提交历史 |
| `vctl branch` | 列出本地分支 |
| `vctl branch -b <name>` | 创建分支 |
| `vctl branch -d <name>` | 删除分支 |
| `vctl checkout <branch|hash>` | 切换分支或提交 |
| `vctl clone <source> [target]` | 克隆本地仓库 |
| `vctl config <key> <value>` | 设置仓库配置 |

## 仓库结构

```bash
.version/
├── HEAD
├── config
├── ignore
├── objects/
└── refs/
    └── heads/
```

## 项目优化说明

本次更新包含：

- `checkout` 切换分支后自动恢复工作区文件
- `commit` 输出变更文件数前先统计
- `clone` 更新忽略 `.version` / `.git` / `.svn` / `.hg` 根目录处理
- `add` 使用更稳定的时间戳转换方式
- Windows 构建命令补充 `-lz` 链接

## 许可

本项目遵循 `LICENSE` 文件中的条款。
