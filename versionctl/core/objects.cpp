#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "../include/types.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../storage/hash.h"
#include "../storage/binary_io.h"
#include "../storage/cache.h"

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

// 创建 Blob 对象
std::string createBlob(const std::string& root, const std::string& content) {
    // 计算哈希值 (包含类型前缀，与 Git 兼容)
    std::string header = "blob " + std::to_string(content.length()) + "\0";
    std::string hashData = header + content;
    std::string hash = storage::SHA256::compute(hashData);
    
    // 检查对象是否已存在
    std::string objectPath = utils::getObjectPath(root, hash);
    if (utils::fileExists(objectPath)) {
        return hash;
    }
    
    // 创建 Blob 对象
    Blob blob(content, hash);
    
    // 确保对象目录存在
    std::string objectDir = utils::getObjectsDir(root);
    utils::createDirectories(objectDir);
    
    // 写入对象文件
    if (!storage::writeBlob(objectPath, blob)) {
        return "";
    }
    
    // 添加到缓存
    storage::getGlobalCache().putBlob(hash, blob);
    
    return hash;
}

// 读取 Blob 对象
Blob readBlobObject(const std::string& root, const std::string& hash) {
    // 先从缓存中查找
    Blob cachedBlob;
    if (storage::getGlobalCache().getBlob(hash, cachedBlob)) {
        return cachedBlob;
    }
    
    // 从磁盘读取
    std::string objectPath = utils::getObjectPath(root, hash);
    Blob blob = storage::readBlob(objectPath);
    
    if (!blob.content.empty()) {
        storage::getGlobalCache().putBlob(hash, blob);
    }
    
    return blob;
}

// 创建 Tree 对象
std::string createTree(const std::string& root, Tree& tree) {
    // 序列化 Tree 内容用于计算哈希
    std::stringstream ss;
    for (const auto& entry : tree.entries) {
        ss << entry.mode << " " 
           << (entry.type == ObjectType::TREE ? "tree" : "blob") << " "
           << entry.hash << " " << entry.name << "\n";
    }
    
    std::string content = ss.str();
    std::string hash = storage::SHA256::compute(content);
    
    // 检查对象是否已存在
    std::string objectPath = utils::getObjectPath(root, hash);
    if (utils::fileExists(objectPath)) {
        return hash;
    }
    
    tree.hash = hash;
    
    // 确保对象目录存在
    std::string objectDir = utils::getObjectsDir(root);
    utils::createDirectories(objectDir);
    
    // 写入对象文件
    if (!storage::writeTree(objectPath, tree)) {
        return "";
    }
    
    // 添加到缓存
    storage::getGlobalCache().putTree(hash, tree);
    
    return hash;
}

// 读取 Tree 对象
Tree readTreeObject(const std::string& root, const std::string& hash) {
    // 先从缓存中查找
    Tree cachedTree;
    if (storage::getGlobalCache().getTree(hash, cachedTree)) {
        return cachedTree;
    }
    
    // 从磁盘读取
    std::string objectPath = utils::getObjectPath(root, hash);
    Tree tree = storage::readTree(objectPath);
    
    if (!tree.entries.empty()) {
        tree.hash = hash;
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

// 创建 Commit对象
std::string createCommit(const std::string& root, Commit& commit) {
    // 序列化 Commit 内容用于计算哈希
    std::stringstream ss;
    ss << "tree " << commit.tree << "\n";
    
    for (const auto& parent : commit.parents) {
        ss << "parent " << parent << "\n";
    }
    
    ss << "author " << commit.author << "\n"
       << "committer " << commit.committer << "\n"
       << "timestamp " << commit.timestamp << "\n"
       << "\n" << commit.message;
    
    std::string content = ss.str();
    std::string hash = storage::SHA256::compute(content);
    
    commit.hash = hash;
    
    // 确保对象目录存在
    std::string objectDir = utils::getObjectsDir(root);
    utils::createDirectories(objectDir);
    
    // 写入对象文件
    std::string objectPath = utils::getObjectPath(root, hash);
    if (!storage::writeCommit(objectPath, commit)) {
        return "";
    }
    
    // 添加到缓存
    storage::getGlobalCache().putCommit(hash, commit);
    
    return hash;
}

// 读取 Commit对象
Commit readCommitObject(const std::string& root, const std::string& hash) {
    // 先从缓存中查找
    Commit cachedCommit;
    if (storage::getGlobalCache().getCommit(hash, cachedCommit)) {
        return cachedCommit;
    }
    
    // 从磁盘读取
    std::string objectPath = utils::getObjectPath(root, hash);
    Commit commit = storage::readCommit(objectPath);
    
    if (!commit.tree.empty()) {
        commit.hash = hash;
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
