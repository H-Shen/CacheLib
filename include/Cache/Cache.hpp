#ifndef CACHE_CACHE_HPP
#define CACHE_CACHE_HPP

#include <cstddef>

/// 通用缓存接口
template<typename K, typename V>
class Cache {
public:
    virtual ~Cache() = default;

    Cache() = default;

    Cache(const Cache &) = delete;

    Cache &operator=(const Cache &) = delete;

    Cache(Cache &&) = delete;

    Cache &operator=(Cache &&) = delete;

    /// 插入新元素或更新已有元素
    virtual void put(const K &key, const V &value) = 0;

    /// 访问元素;不存在时抛出 std::out_of_range
    virtual V get(const K &key) = 0;

    /// 删除元素;不存在时无操作
    virtual void erase(const K &key) = 0;

    /// 判断是否包含 key
    virtual bool contains(const K &key) const = 0;

    /// 当前缓存中元素个数
    virtual std::size_t size() const = 0;
};

#endif //CACHE_CACHE_HPP
