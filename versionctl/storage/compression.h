#ifndef STORAGE_COMPRESSION_H
#define STORAGE_COMPRESSION_H

#include <string>
#include <vector>

namespace versionctl {
namespace storage {

// 压缩工具类（使用 zlib）
class Compression {
public:
    // 压缩数据
    static std::string compress(const std::string& data);
    
    // 解压数据
    static std::string decompress(const std::string& compressedData);
    
    // 压缩文件
    static bool compressFile(const std::string& inputPath, const std::string& outputPath);
    
    // 解压文件
    static bool decompressFile(const std::string& inputPath, const std::string& outputPath);
};

} // namespace storage
} // namespace versionctl

#endif // STORAGE_COMPRESSION_H
