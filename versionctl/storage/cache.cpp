#include <string>
#include <map>
#include <mutex>
#include <vector>
#include "cache.h"

namespace versionctl {
namespace storage {

// 全局缓存实例
ObjectCache globalCache;

// 获取全局缓存
ObjectCache& getGlobalCache() {
    return globalCache;
}

} // namespace storage
} // namespace versionctl
