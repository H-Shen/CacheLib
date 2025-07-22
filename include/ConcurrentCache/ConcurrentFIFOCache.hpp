//
// Created by Haohu Shen on 2025-07-20.
//

#ifndef CACHE_CONCURRENTFIFOCACHE_HPP
#define CACHE_CONCURRENTFIFOCACHE_HPP

#include "../Cache/FIFOCache.hpp"
#include "ConcurrentCache.hpp"

template<typename K, typename V>
using ConcurrentFIFOCache = ConcurrentCache<K, V, FIFOCache<K, V>>;

#endif //CACHE_CONCURRENTFIFOCACHE_HPP
