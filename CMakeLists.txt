cmake_minimum_required(VERSION 3.17)
project(DistributedTemplateLibrary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O0")

file(GLOB tests test/*.cpp)

link_libraries(pthread)

add_executable(test ${tests})
