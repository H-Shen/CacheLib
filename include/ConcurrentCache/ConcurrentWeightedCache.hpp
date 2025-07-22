#ifndef CACHE_CONCURRENTWEIGHTEDCACHE_HPP
#define CACHE_CONCURRENTWEIGHTEDCACHE_HPP

#include "../Cache/WeightedCache.hpp"
#include "ConcurrentCache.hpp"

template<typename K, typename T, typename W>
using ConcurrentWeightedCache = ConcurrentCache<K, std::pair<T, W>, WeightedCache<K, std::pair<T, W>>>;

#endif //CACHE_CONCURRENTWEIGHTEDCACHE_HPP
