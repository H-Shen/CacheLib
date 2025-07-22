#ifndef CACHE_RANDOMREPLACEMENTCACHE_HPP
#define CACHE_RANDOMREPLACEMENTCACHE_HPP


#include "Cache.hpp"
#include <vector>
#include <unordered_map>
#include <random>
#include <stdexcept>
#include <type_traits>

/// 随机替换策略缓存:当容量满时,随机淘汰一个元素
/// 要求 K 可哈希

template<typename K, typename V>
class RandomReplacementCache : public Cache<K, V> {
private:
    static_assert(
            std::is_default_constructible_v<std::hash<K>>,
            "Key type K must be hashable: provide specialization of std::hash<K> if needed"
    );
    std::size_t m_capacity;
    std::vector<K> m_keys;                              // 用于随机访问的 key 列表
    std::unordered_map<K, std::pair<V, std::size_t>> m_map; // key -> (value, index in m_keys)
    mutable std::mt19937 m_gen;                         // 随机数引擎
public:
    explicit RandomReplacementCache(std::size_t capacity)
            : m_capacity(capacity),
              m_gen(std::random_device{}()) {
        if (capacity == 0)
            throw std::invalid_argument("RandomCache capacity must be > 0");
        m_keys.reserve(capacity);  // 预先分配,避免扩容开销
    }

    void put(const K &key, const V &value) override {
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            // 已存在:仅更新值
            it->second.first = value;
            return;
        }
        if (m_map.size() >= m_capacity) {
            // 随机选择一个下标淘汰
            std::uniform_int_distribution<std::size_t> dist(0, m_keys.size() - 1);
            std::size_t idx = dist(m_gen);
            K evict = m_keys[idx];
            // 用最后一个元素填补 idx,然后 pop_back
            K last = m_keys.back();
            m_keys[idx] = last;
            m_map[last].second = idx;
            m_keys.pop_back();
            m_map.erase(evict);
        }
        // 插入新元素
        m_keys.push_back(key);
        m_map.emplace(key, std::make_pair(value, m_keys.size() - 1));
    }

    V get(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end())
            throw std::out_of_range("Key not found");
        return it->second.first;
    }

    void erase(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end()) return;
        // 删除时同样用最后一个元素填补空位
        std::size_t idx = it->second.second;
        K last = m_keys.back();
        m_keys[idx] = last;
        m_map[last].second = idx;
        m_keys.pop_back();
        m_map.erase(it);
    }

    [[nodiscard]] bool contains(const K &key) const override {
        return m_map.count(key) != 0;
    }

    [[nodiscard]] std::size_t size() const override {
        return m_map.size();
    }
};

#endif //CACHE_RANDOMREPLACEMENTCACHE_HPP
