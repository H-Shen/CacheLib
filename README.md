# Cache Library

A header-only, C++20-compliant cache library offering multiple eviction strategies and optional thread-safety via a concurrent wrapper.

---

## Features

- **Unified Interface**: `Cache<K,V>` with methods:
    - `void put(const K&, const V&)`
    - `std::optional<V> get(const K&)` (returns `std::nullopt` if missing)
    - `void erase(const K&)`
    - `bool contains(const K&) const`
    - `std::size_t size() const noexcept`
- **Eviction Policies**:
    - FIFO (First-In, First-Out)
    - LRU (Least Recently Used)
    - LFU (Least Frequently Used)
    - Random Replacement
    - Weighted Replacement (evicts smallest weight via weight-to-key mapping)
- **Concurrent Wrapper**: `ConcurrentCache<K,V,Policy>` using `std::shared_mutex`:
    - Read operations (`get`, `contains`, `size`) use shared locks
    - Write operations (`put`, `erase`) use exclusive locks

---

## Complexity Analysis

| Policy                   | put         | get         | erase      | contains   | size     |
|--------------------------|-------------|-------------|------------|------------|----------|
| FIFO                     | O(1)        | O(1)        | O(1)       | O(1)       | O(1)     |
| LRU                      | O(1)        | O(1)        | O(1)       | O(1)       | O(1)     |
| LFU (bucket-based)       | O(1) amort. | O(1) amort. | O(1)       | O(1)       | O(1)     |
| Random Replacement       | O(1)        | O(1)        | O(1) avg.  | O(1)       | O(1)     |
| Weighted Replacement     | O(log n)    | O(log n)    | O(log n)   | O(1)       | O(1)     |

Concurrent locks add minor overhead:
- Reads: O(1) + shared-lock acquisition
- Writes: O(1)/O(log n) + exclusive-lock acquisition

---

## Building

Requirements:
- A C++20-compliant compiler
- CMake >= 3.15
- Threads support (`-pthread`)

```bash
git clone <repo-url>
cd ./CacheLib
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
# On Windows, run 'ctest --test-dir build --output-on-failure -C Debug'
```

---

## References

*   [Cache replacement policies](https://en.wikipedia.org/wiki/Cache_replacement_policies)
*   [LFU Cache](https://leetcode.com/problems/lfu-cache/)
*   [LRU Cache](https://leetcode.com/problems/lru-cache/)
