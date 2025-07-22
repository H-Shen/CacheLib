#ifndef CACHE_FIFOCACHE_HPP
#define CACHE_FIFOCACHE_HPP

#include "Cache.hpp"
#include <list>
#include <functional>
#include <stdexcept>
#include <type_traits>

/// FIFO 策略缓存:按插入顺序淘汰最老元素
template<typename K, typename V>
class FIFOCache : public Cache<K, V> {
private:
    static_assert(
            std::is_default_constructible_v<std::hash<K>>,
            "Key type K must be hashable: provide specialization of std::hash<K> if needed"
    );
    std::size_t m_capacity;  // 容量,必须 > 0
    std::list<K> m_order;     // 插入顺序队列
    std::unordered_map<
            K,
            std::pair<V, typename std::list<K>::iterator>
    > m_map;       // key → (value, 队列节点)

public:
    explicit FIFOCache(std::size_t capacity)
            : m_capacity(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("FIFOCache capacity must be > 0");
        }
    }

    void put(const K &key, const V &value) override {
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            // 已有则更新,不调整顺序
            it->second.first = value;
            return;
        }
        // 达到容量则淘汰最老元素
        if (m_map.size() == m_capacity) {
            K old_key = m_order.front();
            m_order.pop_front();
            m_map.erase(old_key);
        }
        // 插入新元素
        m_order.push_back(key);
        m_map[key] = {value, std::prev(m_order.end())};
    }

    std::optional<V> get(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end()) {
            return std::nullopt;
        }
        return it->second.first;
    }

    void erase(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end()) return;
        m_order.erase(it->second.second);
        m_map.erase(it);
    }

    [[nodiscard]] bool contains(const K &key) const override {
        return m_map.count(key) != 0;
    }

    [[nodiscard]] std::size_t size() const override {
        return m_map.size();
    }
};

#endif //CACHE_FIFOCACHE_HPP
