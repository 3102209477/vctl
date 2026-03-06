#ifndef STORAGE_HASH_H
#define STORAGE_HASH_H

#include <string>

namespace versionctl {
namespace storage {

class SHA256 {
public:
    static std::string compute(const std::string& data);
    static std::string computeFile(const std::string& filepath);
};

} // namespace storage
} // namespace versionctl

#endif // STORAGE_HASH_H
