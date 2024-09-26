# all source files
set(SRC BatchingBenchmark.cpp)

# define the batching_benchmark target
sfml_add_example(batching_benchmark GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics
                 RESOURCES_DIR resources)
