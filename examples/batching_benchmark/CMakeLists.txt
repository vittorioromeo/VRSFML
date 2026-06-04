# all source files
set(SRC BatchingBenchmark.cpp)

# define the batching_benchmark target
zancle_add_example(batching_benchmark GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics)
