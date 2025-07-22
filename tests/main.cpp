#include "../include/ConcurrentCache/ConcurrentFIFOCache.hpp"
#include "../include/ConcurrentCache/ConcurrentLRUCache.hpp"
#include "../include/ConcurrentCache/ConcurrentLFUCache.hpp"
#include "../include/ConcurrentCache/ConcurrentRandomReplacementCache.hpp"
#include "../include/ConcurrentCache/ConcurrentWeightedCache.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <set>

// ===== FIFO Cache Tests =====
void test_fifo_basic() {
    FIFOCache<int, int> cache(3);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);
    cache.put(4, 40);  // evict key=1
    assert(!cache.contains(1) && cache.contains(4));
    std::cout << "[fifo_basic] PASS\n";
}

void test_fifo_concurrent() {
    ConcurrentCache<int, int, FIFOCache<int, int>> cache(1000);
    const int threads = 10;
    const int ops = 100;
    std::vector<std::thread> workers;
    workers.reserve(threads);
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&cache, t]() {
            for (int i = 0; i < ops; ++i) {
                cache.put(t * ops + i, i);
            }
        });
    }
    for (auto &th: workers) th.join();
    assert(cache.size() == threads * ops);
    std::cout << "[fifo_concurrent] PASS\n";
}

// ===== LRU Cache Tests =====
void test_lru_basic() {
    LRUCache<int, int> cache(2);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.get(1);
    cache.put(3, 3);  // evict key=2
    assert(!cache.contains(2) && cache.contains(3));
    std::cout << "[lru_basic] PASS\n";
}

void test_lru_concurrent() {
    ConcurrentCache<int, int, LRUCache<int, int>> cache(50);
    const int threads = 5;
    const int ops = 10;
    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&cache, ops]() {
            for (int i = 0; i < ops; ++i) {
                cache.put(i, i);
            }
        });
    }
    for (auto &th: workers) th.join();
    assert(cache.size() == ops);
    std::cout << "[lru_concurrent] PASS\n";
}

// ===== LFU Cache Tests =====
void test_lfu_basic() {
    LFUCache<int, int> cache(2);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.get(1);
    cache.get(1);
    cache.get(2);
    cache.put(3, 3);  // evict key=2
    assert(!cache.contains(2) && cache.contains(1));
    std::cout << "[lfu_basic] PASS\n";
}

void test_lfu_concurrent() {
    ConcurrentCache<int, int, LFUCache<int, int>> cache(30);
    const int threads = 3;
    const int ops = 10;
    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&cache, ops]() {
            for (int i = 0; i < ops; ++i) {
                cache.put(i, i);
            }
        });
    }
    for (auto &th: workers) th.join();
    assert(cache.size() == ops);
    std::cout << "[lfu_concurrent] PASS\n";
}

// ===== Random Replacement Cache Tests =====
void test_random_basic_ops() {
    RandomReplacementCache<int, int> cache(3);
    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);
    cache.put(2, 250); // update value
    assert(cache.get(2) == 250);
    std::cout << "[random_basic_ops] PASS\n";
}

void test_random_eviction() {
    RandomReplacementCache<int, int> cache(3);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);
    assert(cache.size() == 3);
    // 连续插入新的 key,容量应始终保持 3
    for (int i = 4; i <= 10; ++i) {
        cache.put(i, i);
        assert(cache.size() == 3);
        // 新插入的 i 必定在 cache 中
        assert(cache.contains(i));
    }
    std::cout << "[random_eviction] PASS\n";
}

void test_random_erase() {
    RandomReplacementCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.erase(1);
    assert(!cache.contains(1));
    std::cout << "[random_erase] PASS\n";
}

void test_random_concurrent_put() {
    ConcurrentCache<int, int, RandomReplacementCache<int, int>> cache(50);
    const int threads = 8;
    const int ops = 200;
    std::vector<std::thread> workers;
    workers.reserve(threads);
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&cache, t, ops]() {
            for (int i = 0; i < ops; ++i) cache.put(t * ops + i, i);
        });
    }
    for (auto &th: workers) th.join();
    assert(cache.size() <= 50);
    std::cout << "[random_concurrent_put] PASS\n";
}

void test_random_concurrent_get() {
    ConcurrentCache<int, int, RandomReplacementCache<int, int>> cache(100);
    for (int k = 0; k < 100; ++k) cache.put(k, k + 1000);
    std::vector<std::thread> workers;
    workers.reserve(8);
    for (int t = 0; t < 8; ++t) {
        workers.emplace_back([&cache]() {
            for (int i = 0; i < 1000; ++i) {
                assert(cache.get(i % 100) == (i % 100) + 1000);
            }
        });
    }
    for (auto &th: workers) th.join();
    std::cout << "[random_concurrent_get] PASS\n";
}

void test_random_concurrent_mixed() {
    ConcurrentCache<int, int, RandomReplacementCache<int, int>> cache(100);
    const int ops = 200;
    auto writer = [&](int t) { for (int i = 0; i < ops; ++i) cache.put((t << 16) | i, i); };
    auto reader = [&]() {
        for (int i = 0; i < ops * 2; ++i) {
            int k = i % ops;
            auto val = cache.get(k);
        }
    };
    std::vector<std::thread> workers;
    for (int t = 0; t < 8; ++t) {
        if (t % 2 == 0) workers.emplace_back(writer, t);
        else workers.emplace_back(reader);
    }
    for (auto &th: workers) th.join();
    std::cout << "[random_concurrent_mixed] PASS\n";
}

// ===== Weighted Cache Tests =====
void test_weighted_basic() {
    WeightedCache<int, std::pair<int, int>> cache(3);
    cache.put(1, {100, 10});
    cache.put(2, {200, 20});
    cache.put(3, {300, 30});
    cache.put(4, {400, 5});  // evict weight=10
    assert(!cache.contains(1));
    std::cout << "[weighted_basic] PASS\n";
}

void test_weighted_conflict() {
    WeightedCache<int, std::pair<int, int>> cache(2);
    cache.put(1, {100, 50});
    cache.put(2, {200, 50});  // conflict
    assert(cache.size() == 1 && cache.get(1).has_value() && cache.get(1)->first == 200);
    std::cout << "[weighted_conflict] PASS\n";
}

void test_weighted_uniform() {
    WeightedCache<int, std::pair<int, int>> cache(10);
    cache.put(1, {1, 999});
    for (int v = 2; v < 100; ++v) {
        cache.put(v, {v, 999});
        assert(cache.size() == 1);
    }
    std::cout << "[weighted_uniform] PASS\n";
}

void test_weighted_concurrent_put() {
    ConcurrentCache<int, std::pair<int, int>, WeightedCache<int, std::pair<int, int>>> cache(50);
    const int threads = 8;
    const int ops = 500;
    std::vector<std::thread> workers;
    workers.reserve(threads);
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&cache, t]() {
            for (int i = 0; i < ops; ++i)
                cache.put(t * ops + i, {i, i % 100});
        });
    }
    for (auto &th: workers) th.join();
    assert(cache.size() <= 50);
    std::cout << "[weighted_concurrent_put] PASS\n";
}

void test_weighted_concurrent_get() {
    ConcurrentCache<int, std::pair<int, int>, WeightedCache<int, std::pair<int, int>>> cache(100);
    for (int k = 0; k < 100; ++k)
        cache.put(k, {k, k});
    std::vector<std::thread> workers;
    workers.reserve(8);
    for (int t = 0; t < 8; ++t) {
        workers.emplace_back([&cache]() {
            for (int i = 0; i < 1000; ++i) {
                if (cache.contains(i % 100))
                    cache.get(i % 100);
            }
        });
    }
    for (auto &th: workers) th.join();
    std::cout << "[weighted_concurrent_get] PASS\n";
}

void test_weighted_concurrent_mixed() {
    ConcurrentCache<int, std::pair<int, int>, WeightedCache<int, std::pair<int, int>>> cache(100);
    const int ops = 200;
    auto writer = [&](int t) {
        for (int i = 0; i < ops; ++i)
            cache.put((t << 16) | i, {i, i});
    };
    auto reader = [&]() {
        for (int i = 0; i < ops * 2; ++i) {
            int k = i % ops;
            auto val = cache.get(k);
        }
    };
    std::vector<std::thread> workers;
    for (int t = 0; t < 8; ++t) {
        if (t % 2 == 0) workers.emplace_back(writer, t);
        else workers.emplace_back(reader);
    }
    for (auto &th: workers) th.join();
    std::cout << "[weighted_concurrent_mixed] PASS\n";
}

int main() {
    test_fifo_basic();
    test_fifo_concurrent();
    test_lru_basic();
    test_lru_concurrent();
    test_lfu_basic();
    test_lfu_concurrent();
    test_random_basic_ops();
    test_random_eviction();
    test_random_erase();
    test_random_concurrent_put();
    test_random_concurrent_get();
    test_random_concurrent_mixed();
    test_weighted_basic();
    test_weighted_conflict();
    test_weighted_uniform();
    test_weighted_concurrent_put();
    test_weighted_concurrent_get();
    test_weighted_concurrent_mixed();
    std::cout << "all_tests_passed.\n";
    return 0;
}
