#include "compression.h"
#include <zlib.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace versionctl {
namespace storage {

std::string Compression::compress(const std::string& data) {
    if (data.empty()) return "";
    
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    
    // 初始化压缩流
    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return "";
    }
    
    // 分配输出缓冲区
    std::vector<char> outbuf;
    outbuf.resize(data.size() + 12); // zlib 头部
    
    // 设置输入
    stream.avail_in = static_cast<uInt>(data.size());
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.c_str()));
    
    // 设置输出
    stream.avail_out = static_cast<uInt>(outbuf.size());
    stream.next_out = reinterpret_cast<Bytef*>(outbuf.data());
    
    // 执行压缩
    int ret = deflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&stream);
        return "";
    }
    
    // 获取压缩后的大小
    size_t compressedSize = stream.total_out;
    
    // 清理
    deflateEnd(&stream);
    
    return std::string(outbuf.data(), compressedSize);
}

std::string Compression::decompress(const std::string& compressedData) {
    if (compressedData.empty()) return "";
    
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    
    // 初始化解压流
    if (inflateInit(&stream) != Z_OK) {
        return "";
    }
    
    // 分配输出缓冲区（假设解压后不超过原始 10 倍）
    std::vector<char> outbuf;
    outbuf.resize(compressedData.size() * 10);
    
    size_t totalDecompressed = 0;
    
    // 设置输入
    stream.avail_in = static_cast<uInt>(compressedData.size());
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressedData.c_str()));
    
    while (true) {
        // 设置输出缓冲区
        stream.avail_out = static_cast<uInt>(outbuf.size() - totalDecompressed);
        stream.next_out = reinterpret_cast<Bytef*>(outbuf.data() + totalDecompressed);
        
        // 执行解压
        int ret = inflate(&stream, Z_NO_FLUSH);
        
        if (ret == Z_STREAM_END) {
            // 解压完成
            totalDecompressed += (stream.total_out - totalDecompressed);
            break;
        } else if (ret == Z_OK) {
            // 需要更多空间
            totalDecompressed += (stream.total_out - totalDecompressed);
            outbuf.resize(outbuf.size() * 2);
        } else {
            // 错误
            inflateEnd(&stream);
            return "";
        }
    }
    
    // 清理
    inflateEnd(&stream);
    
    return std::string(outbuf.data(), totalDecompressed);
}

bool Compression::compressFile(const std::string& inputPath, const std::string& outputPath) {
    try {
        // 读取输入文件
        std::ifstream inFile(inputPath, std::ios::binary);
        if (!inFile) return false;
        
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string content = buffer.str();
        inFile.close();
        
        // 压缩
        std::string compressed = compress(content);
        if (compressed.empty()) return false;
        
        // 写入输出文件
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile) return false;
        
        outFile.write(compressed.data(), compressed.size());
        outFile.close();
        
        return true;
        
    } catch (...) {
        return false;
    }
}

bool Compression::decompressFile(const std::string& inputPath, const std::string& outputPath) {
    try {
        // 读取压缩文件
        std::ifstream inFile(inputPath, std::ios::binary);
        if (!inFile) return false;
        
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string compressed = buffer.str();
        inFile.close();
        
        // 解压
        std::string decompressed = decompress(compressed);
        if (decompressed.empty()) return false;
        
        // 写入输出文件
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile) return false;
        
        outFile.write(decompressed.data(), decompressed.size());
        outFile.close();
        
        return true;
        
    } catch (...) {
        return false;
    }
}

} // namespace storage
} // namespace versionctl
