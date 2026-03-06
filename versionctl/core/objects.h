#ifndef CORE_OBJECTS_H
#define CORE_OBJECTS_H

#include <string>
#include "../include/types.h"

namespace versionctl {
namespace core {

// 计算文件的哈希值
std::string hashFile(const std::string& filepath);

// 计算内容的哈希值
std::string hashContent(const std::string& content);

// 创建 Blob 对象
std::string createBlob(const std::string& root, const std::string& content);

// 读取 Blob 对象
Blob readBlobObject(const std::string& root, const std::string& hash);

// 创建 Tree 对象
std::string createTree(const std::string& root, Tree& tree);

// 读取 Tree 对象
Tree readTreeObject(const std::string& root, const std::string& hash);

// 递归创建目录树
std::string createTreeFromDirectory(const std::string& root, const std::string& dirPath, 
                                    const std::string& relativePath = "");

// 创建 Commit 对象
std::string createCommit(const std::string& root, Commit& commit);

// 读取 Commit 对象
Commit readCommitObject(const std::string& root, const std::string& hash);

} // namespace core
} // namespace versionctl

#endif // CORE_OBJECTS_H
