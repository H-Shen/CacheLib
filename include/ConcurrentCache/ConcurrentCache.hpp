#ifndef CACHE_CONCURRENTCACHE_HPP
#define CACHE_CONCURRENTCACHE_HPP

#include "../Cache/Cache.hpp"
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <mutex>

/// 通用并发缓存装饰器:通过组合 Cache<K,V> 实现线程安全
/// 不通过继承,而是直接包含一个 std::unique_ptr<Cache<K,V>>
/// 通用并发缓存装饰器（读写分离）
/// - 写操作（put/erase）使用 std::unique_lock
/// - 读操作（get/contains/size）使用 std::shared_lock
/// CacheImpl 必须是 Cache<K,V> 的具体实现

template<typename K, typename V, typename CacheImpl>
class ConcurrentCache {
private:
    std::unique_ptr<Cache<K, V>> m_delegate;
    mutable std::shared_mutex m_mutex;

public:
    /// 构造时将参数转发给 CacheImpl
    template<typename... Args>
    explicit ConcurrentCache(Args&&... args)
            : m_delegate(std::make_unique<CacheImpl>(std::forward<Args>(args)...))
    {}

    /// 插入或更新
    void put(const K& key, const V& value) {
        std::unique_lock lock(m_mutex);
        m_delegate->put(key, value);
    }

    /// 获取，抛出 out_of_range
    std::optional<V> get(const K& key) {
        std::shared_lock lock(m_mutex);
        return m_delegate->get(key);
    }

    /// 删除条目
    void erase(const K& key) {
        std::unique_lock lock(m_mutex);
        m_delegate->erase(key);
    }

    /// 是否包含
    bool contains(const K& key) const {
        std::shared_lock lock(m_mutex);
        return m_delegate->contains(key);
    }

    /// 当前大小
    std::size_t size() const noexcept {
        std::shared_lock lock(m_mutex);
        return m_delegate->size();
    }
};

#endif //CACHE_CONCURRENTCACHE_HPP
