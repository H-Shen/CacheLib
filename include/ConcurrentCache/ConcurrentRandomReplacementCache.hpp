#ifndef CACHE_CONCURRENTRANDOMREPLACEMENTCACHE_HPP
#define CACHE_CONCURRENTRANDOMREPLACEMENTCACHE_HPP

#include "../Cache/RandomReplacementCache.hpp"
#include "ConcurrentCache.hpp"

template<typename K, typename V>
using ConcurrentRandomReplacementCache = ConcurrentCache<K, V, RandomReplacementCache<K, V>>;

#endif //CACHE_CONCURRENTRANDOMREPLACEMENTCACHE_HPP
