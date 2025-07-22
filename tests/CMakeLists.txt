add_executable(CacheTest main.cpp)

target_link_libraries(CacheTest PRIVATE Threads::Threads)

add_test(NAME CacheTest COMMAND CacheTest)
