//
// Created by Haohu Shen on 2025-07-20.
//

#ifndef CACHE_LFUCACHE_HPP
#define CACHE_LFUCACHE_HPP

#include "Cache.hpp"
#include <cstddef>
#include <list>
#include <unordered_map>
#include <stdexcept>
#include <functional>
#include <type_traits>

/// LFU 策略缓存:访问频率最低淘汰,频率相同时按最近插入顺序
/// 容量 > 0
/// 异常安全:在调整频率列表时保证状态一致性
template<typename K, typename V>
class LFUCache : public Cache<K, V> {
private:
    static_assert(
            std::is_default_constructible_v<std::hash<K>>,
            "Key type K must be hashable: provide specialization of std::hash<K> if needed"
    );
    // 节点信息:存储值、访问频率和在频率链表中的迭代器位置
    struct Node {
        V val;
        int freq;
        std::list<K>::iterator iter; // 在对应频率链表中的位置
    };
    std::size_t m_capacity;  // 缓存容量
    int m_min_freq = 0; // 当前最小频率
    std::unordered_map<K, Node> m_nodes;     // key -> Node 映射
    std::unordered_map<int, std::list<K>> m_freq_list;  // 频率 -> keys 列表
public:
    explicit LFUCache(std::size_t capacity)
            : m_capacity(capacity) {
        if (capacity == 0) throw std::invalid_argument("LRUCache capacity must be > 0");
    }

    void put(const K &key, const V &value) override {
        if (m_capacity == 0) return; // 零容量直接返回
        auto it = m_nodes.find(key);
        if (it != m_nodes.end()) {
            // 如果已有该 key,更新其值并提升频率
            it->second.val = value;
            get(key); // 调用 get 逻辑,提升频率并更新位置
            return;
        }
        // 如果已达容量上限,淘汰最少使用的节点
        if (m_nodes.size() >= m_capacity) {
            auto &lst = m_freq_list[m_min_freq];       // 最小频率对应的列表
            K old_key = lst.front();                 // 列表头为最久未使用
            lst.pop_front();                         // 从列表中移除
            m_nodes.erase(old_key);                  // 从节点映射中移除
        }
        // 插入新节点,初始频率为 1
        m_min_freq = 1;                              // 重置最小频率
        auto &lst = m_freq_list[m_min_freq];                  // 获取频率为 1 的列表
        lst.push_back(key);                         // 将 key 加入列表尾
        // 在节点映射中创建新 Node
        m_nodes[key] = {value, m_min_freq, std::prev(lst.end())};
    }

    V get(const K &key) override {
        auto it = m_nodes.find(key);
        if (it == m_nodes.end())
            throw std::out_of_range("Key not found");

        int freq = it->second.freq;
        auto &old_freq_list = m_freq_list[freq];
        old_freq_list.erase(it->second.iter);        // 从旧频率列表中移除

        // 如果移除后列表为空且该频率为最小频率,则删除该列表并更新 m_min_freq
        if (old_freq_list.empty() && freq == m_min_freq) {
            m_freq_list.erase(freq);
            ++m_min_freq;
        }

        // 将 key 插入到新频率列表
        int new_freq = freq + 1;
        auto &new_freq_list = m_freq_list[new_freq];
        new_freq_list.push_back(key);                 // 加入列表尾
        it->second.freq = new_freq;                   // 更新频率
        it->second.iter = std::prev(new_freq_list.end()); // 更新迭代器位置

        return it->second.val;
    }

    void erase(const K &key) override {
        auto it = m_nodes.find(key);
        if (it == m_nodes.end()) return;              // 不存在直接返回

        auto &freq_list = m_freq_list[it->second.freq];
        freq_list.erase(it->second.iter);             // 从列表中移除
        // 如果移除后列表为空且该频率为最小频率,则删除该列表
        if (freq_list.empty() && it->second.freq == m_min_freq) {
            m_freq_list.erase(it->second.freq);
            // m_min_freq 不用立即调整
        }
        m_nodes.erase(it);                             // 从节点映射中移除
    }

    [[nodiscard]] bool contains(const K &key) const override {
        return m_nodes.count(key) != 0;
    }

    [[nodiscard]] std::size_t size() const override {
        return m_nodes.size();
    }
};

#endif //CACHE_LFUCACHE_HPP
