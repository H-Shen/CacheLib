#ifndef CACHE_CONCURRENTLFUCACHE_HPP
#define CACHE_CONCURRENTLFUCACHE_HPP

#include "../Cache/LFUCache.hpp"
#include "ConcurrentCache.hpp"

template<typename K, typename V>
using ConcurrentLFUCache = ConcurrentCache<K, V, LFUCache<K, V>>;

#endif //CACHE_CONCURRENTLFUCACHE_HPP
