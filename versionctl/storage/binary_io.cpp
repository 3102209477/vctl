#include <fstream>
#include <string>
#include <cstring>
#include <filesystem>
#include "../include/types.h"
#include "../include/constants.h"

namespace versionctl {
namespace storage {

// 将整数转换为二进制标记
std::string intToMarker(int value) {
    std::string result;
    for (int i = 0; i < 4; i++) {
        result += static_cast<char>((value >> (i * 8)) & 0xFF);
    }
    return result;
}

// 将二进制标记转换为整数
int markerToInt(const char* marker) {
    return static_cast<int>(static_cast<unsigned char>(marker[0])) |
           (static_cast<int>(static_cast<unsigned char>(marker[1])) << 8) |
           (static_cast<int>(static_cast<unsigned char>(marker[2])) << 16) |
           (static_cast<int>(static_cast<unsigned char>(marker[3])) << 24);
}

// 将 uint64_t 转换为 8 字节二进制
std::string uint64ToBinary(uint64_t value) {
    std::string result;
    for (int i = 0; i < 8; i++) {
        result += static_cast<char>((value >> (i * 8)) & 0xFF);
    }
    return result;
}

// 将 8 字节二进制转换为 uint64_t
uint64_t binaryToUint64(const char* data) {
    uint64_t result = 0;
    for (int i = 0; i < 8; i++) {
        result |= static_cast<uint64_t>(static_cast<unsigned char>(data[i])) << (i * 8);
    }
    return result;
}

// 写入 Blob 对象 (二进制格式)
bool writeBlob(const std::string& path, const Blob& blob) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // 写入类型标记 (4 字节): "BLOB"
    file.write(BLOB_MARKER, 4);
    
    // 写入内容大小 (8 字节)
    uint64_t size = blob.content.length();
    std::string sizeData = uint64ToBinary(size);
    file.write(sizeData.c_str(), 8);
    
    // 写入原始内容
    file.write(blob.content.c_str(), blob.content.length());
    
    file.close();
    return true;
}

// 读取 Blob 对象 (二进制格式)
Blob readBlob(const std::string& path) {
    Blob blob;
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return blob;
    }
    
    // 读取类型标记 (4 字节)
    char marker[4];
    file.read(marker, 4);
    
    // 检查是否为旧版文本格式
    file.seekg(0, std::ios::beg);
    std::string firstLine;
    std::getline(file, firstLine);
    
    if (firstLine.find("blob ") == 0 || firstLine.empty()) {
        // 旧版文本格式，回退到文本解析
        file.close();
        file.open(path, std::ios::binary);
        
        std::string line;
        std::getline(file, line); // "blob <size>"
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            blob.size = std::stoull(line.substr(spacePos + 1));
        }
        
        std::getline(file, line); // 空行
        
        // 读取剩余内容
        std::stringstream buffer;
        buffer << file.rdbuf();
        blob.content = buffer.str();
        
        return blob;
    }
    
    // 验证标记
    if (memcmp(marker, BLOB_MARKER, 4) != 0) {
        file.close();
        return blob;
    }
    
    // 读取内容大小 (8 字节)
    char sizeData[8];
    file.read(sizeData, 8);
    blob.size = binaryToUint64(sizeData);
    
    // 读取原始内容
    blob.content.resize(blob.size);
    file.read(&blob.content[0], blob.size);
    
    file.close();
    return blob;
}

// 写入 Tree 对象 (二进制格式)
bool writeTree(const std::string& path, const Tree& tree) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    // 写入类型标记 (4 字节): "TREE"
    file.write(TREE_MARKER, 4);
    
    // 写入条目数量 (8 字节)
    uint64_t count = tree.entries.size();
    std::string countData = uint64ToBinary(count);
    file.write(countData.c_str(), 8);
    
    // 写入每个条目
    for (const auto& entry : tree.entries) {
        // 写入模式 (10 字节，固定长度)
        std::string mode = entry.mode;
        mode.resize(10, ' '); // 填充到 10 字节
        file.write(mode.c_str(), 10);
        
        // 写入类型 (1 字节)
        char typeByte = (entry.type == ObjectType::TREE) ? 'T' : 'B';
        file.write(&typeByte, 1);
        
        // 写入哈希 (64 字节)
        file.write(entry.hash.c_str(), 64);
        
        // 写入名称长度 (4 字节)
        uint64_t nameLen = entry.name.length();
        std::string lenData = uint64ToBinary(nameLen);
        file.write(lenData.c_str(), 8);
        
        // 写入名称
        file.write(entry.name.c_str(), nameLen);
    }
    
    file.close();
    return true;
}

// 读取 Tree 对象 (二进制格式)
Tree readTree(const std::string& path) {
    Tree tree;
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return tree;
    }
    
    // 读取类型标记 (4 字节)
    char marker[4];
    file.read(marker, 4);
    
    // 验证标记
    if (memcmp(marker, TREE_MARKER, 4) != 0) {
        file.close();
        return tree;
    }
    
    // 读取条目数量 (8 字节)
    char countData[8];
    file.read(countData, 8);
    uint64_t count = binaryToUint64(countData);
    
    // 读取每个条目
    for (uint64_t i = 0; i < count; i++) {
        TreeEntry entry;
        
        // 读取模式 (10 字节)
        char modeData[10];
        file.read(modeData, 10);
        entry.mode = std::string(modeData, 10);
        // 移除尾部空格
        while (!entry.mode.empty() && entry.mode.back() == ' ') {
            entry.mode.pop_back();
        }
        
        // 读取类型 (1 字节)
        char typeByte;
        file.read(&typeByte, 1);
        entry.type = (typeByte == 'T') ? ObjectType::TREE : ObjectType::BLOB;
        
        // 读取哈希 (64 字节)
        entry.hash.resize(64);
        file.read(&entry.hash[0], 64);
        
        // 读取名称长度 (8 字节)
        char lenData[8];
        file.read(lenData, 8);
        uint64_t nameLen = binaryToUint64(lenData);
        
        // 读取名称
        entry.name.resize(nameLen);
        file.read(&entry.name[0], nameLen);
        
        tree.addEntry(entry);
    }
    
    file.close();
    return tree;
}

// 写入 Commit对象 (文本格式，便于阅读和调试)
bool writeCommit(const std::string& path, const Commit& commit) {
    std::ofstream file(path);
    if (!file) {
        return false;
    }
    
    file << "tree " << commit.tree << "\n";
    
    for (const auto& parent : commit.parents) {
        file << "parent " << parent << "\n";
    }
    
    file << "author " << commit.author << "\n";
    file << "committer " << commit.committer << "\n";
    file << "timestamp " << commit.timestamp << "\n";
    file << "\n"; // 空行分隔头部和消息
    
    // 写入提交消息 (支持多行)
    file << commit.message;
    
    file.close();
    return true;
}

// 读取 Commit对象 (文本格式)
Commit readCommit(const std::string& path) {
    Commit commit;
    
    std::ifstream file(path);
    if (!file) {
        return commit;
    }
    
    std::string line;
    bool inMessage = false;
    std::stringstream messageBuffer;
    
    while (std::getline(file, line)) {
        // 清理 CRLF 和 LF
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        
        if (inMessage) {
            messageBuffer << line << "\n";
        } else if (line.empty()) {
            inMessage = true;
        } else if (line.find("tree ") == 0) {
            commit.tree = line.substr(5);
        } else if (line.find("parent ") == 0) {
            commit.parents.push_back(line.substr(7));
        } else if (line.find("author ") == 0) {
            commit.author = line.substr(7);
        } else if (line.find("committer ") == 0) {
            commit.committer = line.substr(10);
        } else if (line.find("timestamp ") == 0) {
            commit.timestamp = std::stoll(line.substr(10));
        }
    }
    
    commit.message = messageBuffer.str();
    // 移除末尾多余的换行符
    while (!commit.message.empty() && commit.message.back() == '\n') {
        commit.message.pop_back();
    }
    
    file.close();
    return commit;
}

} // namespace storage
} // namespace versionctl
