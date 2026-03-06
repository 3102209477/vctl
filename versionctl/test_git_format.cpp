#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "storage/compression.h"

using namespace versionctl;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: test_format <object_file>" << std::endl;
        return 1;
    }
    
    std::string path = argv[1];
    
    // 读取文件
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << path << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string compressed = buffer.str();
    file.close();
    
    std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
    
    // 尝试解压
    std::string decompressed = storage::Compression::decompress(compressed);
    
    if (decompressed.empty()) {
        std::cout << "Decompression failed, trying raw data" << std::endl;
        decompressed = compressed;
    } else {
        std::cout << "Decompressed size: " << decompressed.size() << " bytes" << std::endl;
    }
    
    // 打印前 50 字节的 ASCII 和十六进制
    std::cout << "\nFirst 50 bytes:" << std::endl;
    for (size_t i = 0; i < std::min(size_t(50), decompressed.length()); i++) {
        printf("%02x ", static_cast<unsigned char>(decompressed[i]));
    }
    std::cout << std::endl;
    for (size_t i = 0; i < std::min(size_t(50), decompressed.length()); i++) {
        char c = decompressed[i];
        if (c >= 32 && c <= 126) {
            std::cout << c;
        } else if (c == '\n') {
            std::cout << "\\n";
        } else if (c == '\0') {
            std::cout << "\\0";
        } else {
            std::cout << ".";
        }
    }
    std::cout << std::endl;
    
    // 查找 null 分隔符
    size_t nullPos = decompressed.find('\0');
    if (nullPos != std::string::npos) {
        std::string header = decompressed.substr(0, nullPos);
        std::cout << "Header: [" << header << "]" << std::endl;
        
        if (nullPos + 1 < decompressed.length()) {
            std::string content = decompressed.substr(nullPos + 1);
            std::cout << "Content length: " << content.length() << " bytes" << std::endl;
            std::cout << "Content preview (first 100 chars): ";
            for (size_t i = 0; i < std::min(size_t(100), content.length()); i++) {
                char c = content[i];
                if (c >= 32 && c <= 126) {
                    std::cout << c;
                } else if (c == '\n') {
                    std::cout << "\\n";
                } else {
                    std::cout << ".";
                }
            }
            std::cout << std::endl;
        }
    } else {
        std::cout << "No null separator found in " << decompressed.length() << " bytes" << std::endl;
        // 打印前 50 字节的十六进制
        std::cout << "First 50 bytes (hex): ";
        for (size_t i = 0; i < std::min(size_t(50), decompressed.length()); i++) {
            printf("%02x ", static_cast<unsigned char>(decompressed[i]));
        }
        std::cout << std::endl;
    }
    
    return 0;
}
