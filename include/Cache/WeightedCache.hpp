
#ifndef CACHE_WEIGHTEDCACHE_HPP
#define CACHE_WEIGHTEDCACHE_HPP

#include "Cache.hpp"
#include <utility>
#include <unordered_map>
#include <stdexcept>
#include <type_traits>
#include <set>

/// 加权优先策略缓存:仅支持 V = std::pair<ValueType, WeightType> 的情况
/// 要求 WeightType 为整数类型,且全局唯一(相同权重不会并存)
/// - 当容量满时,淘汰 weight 最小的元素;
/// - 如果插入时发现已有元素的 weight == 新 weight,则仅替换那条目的 value。
///
/// 专门化:V = std::pair<T, W>,且 W 为整数类型
template<typename K, typename V>
class WeightedCache;

template<typename K, typename T, typename W>
class WeightedCache<K, std::pair<T, W>> : public Cache<K, std::pair<T, W>> {
private:
    static_assert(
            std::is_invocable_r_v<bool, std::less<W>, const W &, const W &>,
            "Weight type W must be comparable via std::less<W>"
    );
    static_assert(
            std::is_default_constructible_v<std::hash<K>>,
            "Key type K must be hashable: provide specialization of std::hash<K> if needed"
    );
    // 最大容量
    std::size_t m_capacity;
    // key -> (value, weight)
    std::unordered_map<K, std::pair<T, W>> m_map;
    // weight -> key (保证同一时刻只有一个 entry 拥有该 weight)
    std::unordered_map<W, K> m_w2k;
    // 有序的 weight 集合,用于 O(log n) 淘汰最小 weight
    std::set<W> m_weights;

public:
    explicit WeightedCache(std::size_t capacity)
            : m_capacity(capacity) {
        if (capacity == 0)
            throw std::invalid_argument("WeightedCache capacity must be > 0");
    }

    void put(const K &key, const std::pair<T, W> &entry) override {
        T val = entry.first;
        W w = entry.second;

        // 1) 权重冲突:只更新那条目的 value
        auto wit = m_w2k.find(w);
        if (wit != m_w2k.end()) {
            K old_key = wit->second;
            m_map[old_key].first = val;
            return;
        }

        // 2) 相同 key,不同 weight:移除旧 weight,再插入新 weight
        auto kit = m_map.find(key);
        if (kit != m_map.end()) {
            W old_w = kit->second.second;
            // 删除旧 weight 的映射
            m_weights.erase(old_w);
            m_w2k.erase(old_w);
            // 更新 map 中的 value 和 weight
            kit->second = { val, w };
            // 插入新 weight
            m_w2k[w] = key;
            m_weights.insert(w);
            return;
        }

        // 3) 全新 key+weight:容量满则淘汰最小 weight
        if (m_map.size() >= m_capacity) {
            W min_w = *m_weights.begin();
            K  min_k = m_w2k[min_w];
            m_weights.erase(m_weights.begin());
            m_w2k.erase(min_w);
            m_map.erase(min_k);
        }

        // 插入新节点
        m_map[key]   = { val, w };
        m_w2k[w]     = key;
        m_weights.insert(w);
    }

    std::optional<std::pair<T, W>> get(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end())
            return std::nullopt;
        return it->second;
    }

    void erase(const K &key) override {
        auto it = m_map.find(key);
        if (it == m_map.end()) return;
        W w = it->second.second;
        m_map.erase(it);
        m_w2k.erase(w);
        m_weights.erase(w);
    }

    [[nodiscard]] bool contains(const K &key) const override {
        return m_map.count(key) != 0;
    }

    [[nodiscard]] std::size_t size() const override {
        return m_map.size();
    }
};

#endif //CACHE_WEIGHTEDCACHE_HPP
