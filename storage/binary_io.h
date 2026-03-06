#ifndef STORAGE_BINARY_IO_H
#define STORAGE_BINARY_IO_H

#include <string>
#include "../include/types.h"

namespace versionctl {
namespace storage {

// 将整数转换为二进制标记
std::string intToMarker(int value);

// 将二进制标记转换为整数
int markerToInt(const char* marker);

// 将 uint64_t 转换为 8 字节二进制
std::string uint64ToBinary(uint64_t value);

// 将 8 字节二进制转换为 uint64_t
uint64_t binaryToUint64(const char* data);

// Blob 对象操作
bool writeBlob(const std::string& path, const Blob& blob);
Blob readBlob(const std::string& path);

// Tree 对象操作
bool writeTree(const std::string& path, const Tree& tree);
Tree readTree(const std::string& path);

// Commit 对象操作
bool writeCommit(const std::string& path, const Commit& commit);
Commit readCommit(const std::string& path);

} // namespace storage
} // namespace versionctl

#endif // STORAGE_BINARY_IO_H
