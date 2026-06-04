cmake_minimum_required(VERSION 3.28)
project(zancle-example CXX)

set(ZA_ROOT "${CMAKE_ANDROID_NDK}/sources/third_party/zancle/lib/${CMAKE_ANDROID_ARCH_ABI}/cmake/Zancle")

# Create the libzancle-example.so library that contains the application's c++ code
add_library(zancle-example SHARED main.cpp)

# Find and link Zancle
find_package(Zancle 3 REQUIRED COMPONENTS Graphics Main)
target_link_libraries(zancle-example PRIVATE Zancle::Graphics android log)

# The ANativeActivity_onCreate function from Zancle::Main needs to be exposed in
# our libzancle-example.so file so that is can be loaded at runtime.
target_link_libraries(zancle-example PUBLIC
  -Wl,--whole-archive
  Zancle::Main
  -Wl,--no-whole-archive
)
