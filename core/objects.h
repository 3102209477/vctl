#ifndef CORE_OBJECTS_H
#define CORE_OBJECTS_H

#include <string>
#include <vector>
#include "../include/types.h"

namespace versionctl {
namespace core {

// Blob 对象操作
std::string createBlob(const std::string& root, const std::string& content);
Blob readBlob(const std::string& root, const std::string& hash);

// Tree 对象操作
std::string createTree(const std::string& root, Tree& tree);
Tree readTree(const std::string& root, const std::string& hash);

// Commit 对象操作
std::string createCommit(const std::string& root, Commit& commit);
Commit readCommitObject(const std::string& root, const std::string& hash);

// 高级操作
std::string createBlobFromFile(const std::string& root, const std::string& filePath);
std::string createTreeFromDirectory(const std::string& root, const std::string& dirPath, 
                                    const std::string& /*relativePath*/);

} // namespace core
} // namespace versionctl

#endif // CORE_OBJECTS_H
