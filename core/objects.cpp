#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../config/config_manager.h"
#include "../config/ignore_rules.h"
#include "../core/objects.h"
#include "../storage/hash.h"
#include "../storage/binary_io.h"
#include "../storage/cache.h"
#include "../storage/compression.h"

namespace versionctl {
namespace core {

// 计算文件的哈希值
std::string hashFile(const std::string& filepath) {
    return storage::SHA256::computeFile(filepath);
}

// 计算内容的哈希值
std::string hashContent(const std::string& content) {
    return storage::SHA256::compute(content);
}

// 创建 Blob 对象（Git 格式 + zlib 压缩）
std::string createBlob(const std::string& root, const std::string& content) {
    // Git 格式：<type> <size>\0<content>
    std::string header = "blob " + std::to_string(content.length());
    header.append(1, '\0');  // 添加 null 分隔符
    std::string hashData = header + content;
    std::string hash = storage::SHA256::compute(hashData);
    
    // 检查对象是否已存在
    std::string objectPath = utils::getObjectPath(root, hash);
    if (utils::fileExists(objectPath)) {
        return hash;
    }
    
    // 准备存储的数据（Git 格式）
    std::string storeData = header + content;
    
    // 使用 zlib 压缩
    std::string compressed = storage::Compression::compress(storeData);
    if (compressed.empty()) {
        // 压缩失败，使用未压缩
        compressed = storeData;
    }
    
    // 确保对象目录存在
    std::string objectDir = utils::getObjectsDir(root);
    utils::createDirectories(objectDir);
    
    // 写入压缩的对象文件
    std::ofstream file(objectPath, std::ios::binary);
    if (!file) {
        return "";
    }
    file.write(compressed.data(), compressed.size());
    file.close();
    
    // 添加到缓存
    Blob blob(content, hash);
    storage::getGlobalCache().putBlob(hash, blob);
    
    return hash;
}

// 读取 Blob 对象（Git 格式 + zlib 解压）
Blob readBlob(const std::string& root, const std::string& hash) {
    // 先尝试从缓存获取
    Blob cached;
    if (storage::getGlobalCache().getBlob(hash, cached)) {
        return cached;
    }
    
    std::string objectPath = utils::getObjectPath(root, hash);
    
    // 读取压缩的文件
    std::ifstream file(objectPath, std::ios::binary);
    if (!file) {
        return Blob();
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string compressed = buffer.str();
    file.close();
    
    // 尝试解压（如果是压缩的）
    std::string decompressed = storage::Compression::decompress(compressed);
    if (decompressed.empty()) {
        // 解压失败，可能是未压缩的 Git 格式
        decompressed = compressed;
    }
    
    // 解析 Git 格式：<type> <size>\0<content>
    size_t nullPos = decompressed.find('\0');
    if (nullPos == std::string::npos || nullPos < 5) {
        return Blob();
    }
    
    std::string header = decompressed.substr(0, nullPos);
    if (header.find("blob ") != 0) {
        return Blob();
    }
    
    size_t spacePos = header.find(' ');
    if (spacePos == std::string::npos) {
        return Blob();
    }
    
    try {
        size_t size = std::stoull(header.substr(spacePos + 1));
        std::string content = decompressed.substr(nullPos + 1);
        
        if (content.length() != size) {
            return Blob();
        }
        
        Blob blob(content, hash);
        // 添加到缓存
        storage::getGlobalCache().putBlob(hash, blob);
        return blob;
        
    } catch (...) {
        return Blob();
    }
}

// 读取 Blob 对象
Blob readBlobObject(const std::string& root, const std::string& hash) {
    // 先从缓存中查找
    Blob cachedBlob;
    if (storage::getGlobalCache().getBlob(hash, cachedBlob)) {
        return cachedBlob;
    }
    
    Blob blob = readBlob(root, hash);
    if (!blob.content.empty()) {
        storage::getGlobalCache().putBlob(hash, blob);
    }
    
    return blob;
}

// 创建 Tree 对象（Git 格式 + zlib 压缩）
std::string createTree(const std::string& root, Tree& tree) {
    // Git Tree 格式：<mode> <name>\0<hash-bytes>
    std::stringstream ss;
    for (const auto& entry : tree.entries) {
        ss << entry.mode << " " << entry.name;
        ss.write("\0", 1);

        // 将十六进制 hash 转换为二进制（32 字节 SHA-256）
        if (entry.hash.length() != HASH_LENGTH) {
            return "";
        }

        std::string hashBinary;
        hashBinary.reserve(HASH_LENGTH / 2);
        for (size_t i = 0; i < entry.hash.length(); i += 2) {
            std::string byteStr = entry.hash.substr(i, 2);
            char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
            hashBinary.push_back(byte);
        }
        ss << hashBinary;
    }

    std::string content = ss.str();

    // Git 格式：<type> <size>\0<content>
    std::string header = "tree " + std::to_string(content.length());
    header.append(1, '\0');
    std::string hashData = header + content;
    std::string hash = storage::SHA256::compute(hashData);

    std::string objectPath = utils::getObjectPath(root, hash);
    if (utils::fileExists(objectPath)) {
        return hash;
    }

    std::string storeData = header + content;
    std::string compressed = storage::Compression::compress(storeData);
    if (compressed.empty()) {
        compressed = storeData;
    }

    std::string objectDir = utils::getObjectsDir(root);
    utils::createDirectories(objectDir);

    std::ofstream file(objectPath, std::ios::binary);
    if (!file) {
        return "";
    }
    file.write(compressed.data(), compressed.size());
    file.close();

    tree.hash = hash;
    storage::getGlobalCache().putTree(hash, tree);

    return hash;
}

// 读取 Tree 对象（Git 格式 + zlib 解压）
Tree readTree(const std::string& root, const std::string& hash) {
    // 先从缓存中查找
    Tree cachedTree;
    if (storage::getGlobalCache().getTree(hash, cachedTree)) {
        return cachedTree;
    }
    
    // 从磁盘读取
    std::string objectPath = utils::getObjectPath(root, hash);
    
    std::ifstream file(objectPath, std::ios::binary);
    if (!file) {
        return Tree();
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string compressed = buffer.str();
    file.close();
    
    // 尝试解压
    std::string decompressed = storage::Compression::decompress(compressed);
    if (decompressed.empty()) {
        decompressed = compressed;
    }
    
    // 解析 Git 格式：<type> <size>\0<content>
    size_t nullPos = decompressed.find('\0');
    if (nullPos == std::string::npos || nullPos < 5) {
        return Tree();
    }
    
    std::string header = decompressed.substr(0, nullPos);
    if (header.find("tree ") != 0) {
        return Tree();
    }
    
    Tree tree;
    tree.hash = hash;
    
    // 解析 Tree 内容
    std::string content = decompressed.substr(nullPos + 1);
    size_t pos = 0;
    size_t totalSize = content.length();
    
    while (pos < totalSize) {
        size_t spacePos = content.find(' ', pos);
        if (spacePos == std::string::npos) {
            break;
        }

        std::string mode = content.substr(pos, spacePos - pos);
        pos = spacePos + 1;

        size_t nullPos2 = content.find('\0', pos);
        if (nullPos2 == std::string::npos) {
            break;
        }

        std::string name = content.substr(pos, nullPos2 - pos);
        pos = nullPos2 + 1;

        const size_t hashBytes = HASH_LENGTH / 2;
        if (pos + hashBytes > totalSize) {
            break;
        }

        std::string hashBinary = content.substr(pos, hashBytes);
        pos += hashBytes;

        std::stringstream hashSS;
        hashSS << std::hex << std::setfill('0');
        for (size_t i = 0; i < hashBytes; i++) {
            hashSS << std::setw(2) << (static_cast<unsigned int>(static_cast<unsigned char>(hashBinary[i])));
        }
        std::string entryHash = hashSS.str();

        ObjectType type = (mode == TREE_MODE) ? ObjectType::TREE : ObjectType::BLOB;
        TreeEntry entry(name, entryHash, type, mode);
        tree.addEntry(entry);
    }
    
    if (!tree.entries.empty()) {
        storage::getGlobalCache().putTree(hash, tree);
    }
    
    return tree;
}

// 递归创建目录树
std::string createTreeFromDirectory(const std::string& root, const std::string& dirPath, 
                                    const std::string& /*relativePath*/) {
    Tree tree;
    
    std::filesystem::path fsDirPath(dirPath);
    
    // 遍历目录中的所有条目
    for (const auto& entry : std::filesystem::directory_iterator(fsDirPath)) {
        std::string name = entry.path().filename().string();
        
        // 跳过版本控制目录
        if (name == VERSION_DIR || name == ".git") {
            continue;
        }
        
        if (entry.is_regular_file()) {
            // 读取文件内容并创建 Blob
            std::string content = utils::readFile(entry.path().string());
            std::string blobHash = createBlob(root, content);
            
            if (!blobHash.empty()) {
                TreeEntry blobEntry(name, blobHash, ObjectType::BLOB, FILE_MODE_REGULAR);
                tree.addEntry(blobEntry);
            }
        } else if (entry.is_directory()) {
            // 递归处理子目录
            std::string subTreeHash = createTreeFromDirectory(
                root, entry.path().string(), name);
            
            if (!subTreeHash.empty()) {
                TreeEntry treeEntry(name, subTreeHash, ObjectType::TREE, TREE_MODE);
                tree.addEntry(treeEntry);
            }
        }
    }
    
    // 如果树为空，返回空字符串
    if (tree.entries.empty()) {
        return "";
    }
    
    // 创建 Tree 对象
    return createTree(root, tree);
}

static void collectTreeFiles(const std::string& root, const Tree& tree,
                              const std::string& prefix,
                              std::set<std::string>& paths) {
    for (const auto& entry : tree.entries) {
        std::string path = prefix.empty() ? entry.name : prefix + "/" + entry.name;
        if (entry.type == ObjectType::TREE) {
            Tree subtree = readTree(root, entry.hash);
            if (!subtree.entries.empty()) {
                collectTreeFiles(root, subtree, path, paths);
            }
        } else {
            paths.insert(path);
        }
    }
}

static std::set<std::string> collectTreeFileSet(const std::string& root,
                                                const std::string& treeHash) {
    std::set<std::string> paths;
    if (treeHash.empty()) {
        return paths;
    }

    Tree tree = readTree(root, treeHash);
    if (!tree.entries.empty()) {
        collectTreeFiles(root, tree, "", paths);
    }
    return paths;
}

static void removeEmptyDirectories(const std::filesystem::path& dir,
                                   const std::string& root) {
    if (dir.string() == root) {
        return;
    }

    try {
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            return;
        }

        auto it = std::filesystem::directory_iterator(dir);
        if (it != std::filesystem::directory_iterator()) {
            return;
        }

        std::filesystem::remove(dir);
        removeEmptyDirectories(dir.parent_path(), root);
    } catch (...) {
        // 忽略删除失败，保持工作区稳定
    }
}

static bool cleanupTrackedPaths(const std::string& root,
                                const std::set<std::string>& oldPaths,
                                const std::set<std::string>& newPaths,
                                const versionctl::RepositoryConfig& config) {
    for (const auto& path : oldPaths) {
        if (newPaths.find(path) != newPaths.end()) {
            continue;
        }

        if (config::shouldProtect(path, config)) {
            continue;
        }

        std::string fullPath = utils::joinPath(root, path);
        if (utils::fileExists(fullPath)) {
            if (!utils::deleteFile(fullPath)) {
                return false;
            }

            std::filesystem::path parentDir(fullPath);
            if (parentDir.has_parent_path()) {
                removeEmptyDirectories(parentDir.parent_path(), root);
            }
        }
    }
    return true;
}

// 递归恢复工作区文件
bool checkoutTree(const std::string& root, const Tree& tree, const std::string& destination) {
    if (!utils::fileExists(destination)) {
        if (!utils::createDirectories(destination)) {
            return false;
        }
    }

    for (const auto& entry : tree.entries) {
        std::string targetPath = utils::joinPath(destination, entry.name);

        if (entry.type == ObjectType::TREE) {
            Tree subtree = readTree(root, entry.hash);
            if (subtree.entries.empty()) {
                return false;
            }
            if (!checkoutTree(root, subtree, targetPath)) {
                return false;
            }
        } else {
            Blob blob = readBlobObject(root, entry.hash);
            if (blob.content.empty()) {
                return false;
            }
            if (!utils::writeFile(targetPath, blob.content)) {
                return false;
            }
        }
    }

    return true;
}

bool restoreWorkingTree(const std::string& root, const std::string& commitOrBranch,
                        const std::string& oldCommitHash) {
    std::string commitHash;
    if (utils::isValidHash(commitOrBranch)) {
        commitHash = commitOrBranch;
    } else {
        std::string refPath = utils::getHeadsDir(root) + "/" + commitOrBranch;
        if (utils::fileExists(refPath)) {
            commitHash = utils::trim(utils::readFile(refPath));
        }
    }

    if (commitHash.empty()) {
        return false;
    }

    Commit commit = readCommitObject(root, commitHash);
    if (commit.tree.empty()) {
        return false;
    }

    versionctl::RepositoryConfig config = config::loadRepositoryConfig(root);

    if (!oldCommitHash.empty() && oldCommitHash != commitHash) {
        Commit oldCommit = readCommitObject(root, oldCommitHash);
        if (!oldCommit.tree.empty()) {
            std::set<std::string> oldPaths = collectTreeFileSet(root, oldCommit.tree);
            std::set<std::string> newPaths = collectTreeFileSet(root, commit.tree);
            if (!cleanupTrackedPaths(root, oldPaths, newPaths, config)) {
                return false;
            }
        }
    }

    Tree tree = readTree(root, commit.tree);
    if (tree.entries.empty()) {
        return false;
    }

    return checkoutTree(root, tree, root);
}

// 创建 Commit 对象（Git 格式 + zlib 压缩）
std::string createCommit(const std::string& root, Commit& commit) {
    // Git Commit 格式：头部 + 内容
    // tree <hash>\n
    // parent <hash>\n (可选，多个)
    // author <name> <email> <timestamp>\n
    // committer <name> <email> <timestamp>\n
    // \n
    // message
    std::stringstream ss;
    ss << "tree " << commit.tree << "\n";
    
    for (const auto& parent : commit.parents) {
        ss << "parent " << parent << "\n";
    }
    
    ss << "author " << commit.author << " " << commit.timestamp << "\n";
    ss << "committer " << commit.committer << " " << commit.timestamp << "\n";
    ss << "\n";
    ss << commit.message;
    
    std::string content = ss.str();
    
    // Git 格式：<type> <size>\0<content>
    // 注意：必须使用 string::append 来包含 null 字节
    std::string header = "commit " + std::to_string(content.length());
    header.append(1, '\0');  // 添加 null 分隔符
    std::string hashData = header + content;
    std::string hash = storage::SHA256::compute(hashData);
    
    // 检查对象是否已存在
    std::string objectPath = utils::getObjectPath(root, hash);
    if (utils::fileExists(objectPath)) {
        return hash;
    }
    
    // 准备存储的数据（包含 null 分隔符）
    std::string storeData = header + content;
    
    // 使用 zlib 压缩
    std::string compressed = storage::Compression::compress(storeData);
    if (compressed.empty()) {
        compressed = storeData;
    }
    
    // 确保对象目录存在
    std::string objectDir = utils::getObjectsDir(root);
    utils::createDirectories(objectDir);
    
    // 写入压缩的对象文件
    std::ofstream file(objectPath, std::ios::binary);
    if (!file) {
        return "";
    }
    file.write(compressed.data(), compressed.size());
    file.close();
    
    // 添加到缓存
    commit.hash = hash;
    storage::getGlobalCache().putCommit(hash, commit);
    
    return hash;
}

// 读取 Commit 对象（Git 格式 + zlib 解压）
Commit readCommitObject(const std::string& root, const std::string& hash) {
    // 先从缓存中查找
    Commit cachedCommit;
    if (storage::getGlobalCache().getCommit(hash, cachedCommit)) {
        return cachedCommit;
    }
    
    // 从磁盘读取
    std::string objectPath = utils::getObjectPath(root, hash);
    
    std::ifstream file(objectPath, std::ios::binary);
    if (!file) {
        return Commit();
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string compressed = buffer.str();
    file.close();
    
    // 尝试解压
    std::string decompressed = storage::Compression::decompress(compressed);
    if (decompressed.empty()) {
        decompressed = compressed;
    }
    
    // 解析 Git 格式：<type> <size>\0<content>
    size_t nullPos = decompressed.find('\0');
    if (nullPos == std::string::npos || nullPos < 7) {
        return Commit();
    }
    
    std::string header = decompressed.substr(0, nullPos);
    if (header.find("commit ") != 0) {
        return Commit();
    }
    
    Commit commit;
    commit.hash = hash;
    
    // 解析 Commit 内容
    std::string content = decompressed.substr(nullPos + 1);
    std::istringstream iss(content);
    std::string line;
    
    // 读取 tree
    if (std::getline(iss, line)) {
        if (line.find("tree ") == 0) {
            commit.tree = line.substr(5);
        } else {
            return Commit(); // 格式错误
        }
    }
    
    // 读取 parents（可选，多个）
    while (std::getline(iss, line)) {
        if (line.find("parent ") == 0) {
            commit.parents.push_back(line.substr(7));
        } else {
            break;
        }
    }
    
    // 读取 author
    if (line.find("author ") == 0) {
        // 格式：author Name <email> timestamp
        std::string authorLine = line.substr(7);
        size_t lastSpace = authorLine.rfind(' ');
        if (lastSpace != std::string::npos) {
            commit.author = authorLine.substr(0, lastSpace);
            try {
                commit.timestamp = std::stoll(authorLine.substr(lastSpace + 1));
            } catch (...) {}
        }
    }
    
    // 读取 committer
    if (std::getline(iss, line)) {
        if (line.find("committer ") == 0) {
            std::string committerLine = line.substr(10);
            size_t lastSpace = committerLine.rfind(' ');
            if (lastSpace != std::string::npos) {
                commit.committer = committerLine.substr(0, lastSpace);
            }
        }
    }
    
    // 跳过空行
    std::getline(iss, line);
    
    // 读取 message（剩余所有行）
    std::stringstream messageSS;
    messageSS << line;
    while (std::getline(iss, line)) {
        messageSS << "\n" << line;
    }
    commit.message = messageSS.str();
    
    if (!commit.tree.empty()) {
        storage::getGlobalCache().putCommit(hash, commit);
    }
    
    return commit;
}


// 获取当前分支名称
std::string getCurrentBranch(const std::string& root) {
    std::string headPath = utils::getHeadPath(root);
    
    if (!utils::fileExists(headPath)) {
        return "";
    }
    
    std::string content = utils::readFile(headPath);
    
    // 解析 "ref: refs/heads/<branch_name>"
    if (content.find("ref: refs/heads/") == 0) {
        return content.substr(16);
    }
    
    // 如果是分离 HEAD 状态，返回空字符串
    return "";
}

// 获取 HEAD 指向的提交哈希
std::string getHeadCommit(const std::string& root) {
    std::string currentBranch = getCurrentBranch(root);
    
    if (currentBranch.empty()) {
        // 可能是分离 HEAD 状态
        std::string headPath = utils::getHeadPath(root);
        if (utils::fileExists(headPath)) {
            std::string content = utils::readFile(headPath);
            content = utils::trim(content);
            
            // 检查是否是有效的哈希值
            if (utils::isValidHash(content)) {
                return content;
            }
        }
        return "";
    }
    
    // 读取分支引用文件
    std::string refPath = utils::getHeadsDir(root) + "/" + currentBranch;
    
    if (!utils::fileExists(refPath)) {
        return "";
    }
    
    std::string content = utils::readFile(refPath);
    return utils::trim(content);
}

// 更新 HEAD 引用
bool updateHEAD(const std::string& root, const std::string& branchOrCommit) {
    std::string headPath = utils::getHeadPath(root);
    
    // 确保 refs 目录存在
    utils::createDirectories(utils::getHeadsDir(root));
    
    // 判断是分支还是提交
    if (utils::isValidHash(branchOrCommit)) {
        // 直接指向提交哈希
        return utils::writeFile(headPath, branchOrCommit);
    } else {
        // 指向分支引用
        std::string refContent = "ref: refs/heads/" + branchOrCommit;
        return utils::writeFile(headPath, refContent);
    }
}

// 更新分支引用
bool updateBranchRef(const std::string& root, const std::string& branchName, 
                     const std::string& commitHash) {
    std::string refPath = utils::getHeadsDir(root) + "/" + branchName;
    
    // 确保目录存在
    utils::createDirectories(utils::getHeadsDir(root));
    
    // 写入新的提交哈希
    return utils::writeFile(refPath, commitHash);
}

} // namespace core
} // namespace versionctl
