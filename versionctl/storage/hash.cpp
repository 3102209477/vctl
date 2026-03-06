#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include "hash.h"

#ifdef _WIN32
    #include <windows.h>
    #include <wincrypt.h>
    // Windows CryptoAPI 没有定义 SHA256_DIGEST_LENGTH，需要手动定义
    #ifndef SHA256_DIGEST_LENGTH
        #define SHA256_DIGEST_LENGTH 32
    #endif
#else
    #include <openssl/sha.h>
#endif

namespace versionctl {
namespace storage {

// SHA256::compute 实现
std::string SHA256::compute(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
#ifdef _WIN32
    // Windows 平台使用 CryptoAPI
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, 
                             CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        return "";
    }
    
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return "";
    }
    
    if (!CryptHashData(hHash, reinterpret_cast<const BYTE*>(data.c_str()), 
                       static_cast<DWORD>(data.length()), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }
    
    DWORD hashLen = SHA256_DIGEST_LENGTH;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }
    
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
#else
    // 非 Windows 平台使用 OpenSSL
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.length());
    SHA256_Final(hash, &sha256);
#endif
    
    // 转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

// SHA256::computeFile 实现
std::string SHA256::computeFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return "";
    }
    
#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, 
                             CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        return "";
    }
    
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return "";
    }
    
    // 分块读取大文件
    char buffer[65536];
    while (file.read(buffer, sizeof(buffer))) {
        CryptHashData(hHash, reinterpret_cast<const BYTE*>(buffer), 
                     static_cast<DWORD>(file.gcount()), 0);
    }
    // 处理剩余数据
    if (file.gcount() > 0) {
        CryptHashData(hHash, reinterpret_cast<const BYTE*>(buffer), 
                     static_cast<DWORD>(file.gcount()), 0);
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    DWORD hashLen = SHA256_DIGEST_LENGTH;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }
    
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
#else
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    char buffer[65536];
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    if (file.gcount() > 0) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);
#endif
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

} // namespace storage
} // namespace versionctl
