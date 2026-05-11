# Headless CLI benchmark to measure the impact of `next()` micro-optimizations
# in `ankerl::unordered_dense::map`. See AnkerlNextBenchmark.cpp for details.

set(SRC AnkerlNextBenchmark.cpp)

sfml_add_example(ankerl_next_benchmark
                 SOURCES ${SRC}
                 DEPENDS SFML::System)
