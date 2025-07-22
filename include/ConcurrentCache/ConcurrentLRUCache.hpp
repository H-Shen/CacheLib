#ifndef CACHE_CONCURRENTLRUCACHE_HPP
#define CACHE_CONCURRENTLRUCACHE_HPP

#include "../Cache/LRUCache.hpp"
#include "ConcurrentCache.hpp"

template<typename K, typename V>
using ConcurrentLRUCache = ConcurrentCache<K, V, LRUCache<K, V>>;

#endif //CACHE_CONCURRENTLRUCACHE_HPP
