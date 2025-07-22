#ifndef CACHE_LRUCACHE_HPP
#define CACHE_LRUCACHE_HPP

#include "Cache.hpp"
#include <list>
#include <functional>
#include <type_traits>

/// LRU 策略缓存:最近最少使用淘汰
/// 容量 > 0
/// 异常安全:插入失败时回滚
template<typename K, typename V>
class LRUCache : public Cache<K, V> {
private:
    static_assert(
            std::is_default_constructible_v<std::hash<K>>,
            "Key type K must be hashable: provide specialization of std::hash<K> if needed"
    );
    std::size_t m_capacity;
    std::list<std::pair<K, V>> m_list;   // MRU at back, LRU at front
    std::unordered_map<
            K,
            typename std::list<std::pair<K, V>>::iterator
    > m_map;  // key -> iterator into m_list

public:
    explicit LRUCache(std::size_t capacity)
            : m_capacity(capacity) {
        if (capacity == 0) throw std::invalid_argument("LRUCache capacity must be > 0");
    }

    void put(const K &key, const V &value) override {
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            // update value and move to back
            it->second->second = value;
            m_list.splice(m_list.end(), m_list, it->second);
            return;
        }
        if (m_list.size() >= m_capacity) {
            auto &lru = m_list.front();
            m_map.erase(lru.first);
            m_list.pop_front();
        }
        // insert new entry at back
        m_list.emplace_back(key, value);
        auto lit = m_list.end();
        --lit;
        m_map[key] = lit;
    }

    std::optional<V> get(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end())
            return std::nullopt;
        // move to back and return value
        auto lit = it->second;
        m_list.splice(m_list.end(), m_list, lit);
        return lit->second;
    }

    void erase(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end()) return;
        m_list.erase(it->second);
        m_map.erase(it);
    }

    [[nodiscard]] bool contains(const K &key) const override {
        return m_map.count(key) != 0;
    }

    [[nodiscard]] std::size_t size() const override {
        return m_map.size();
    }
};

#endif //CACHE_LRUCACHE_HPP
