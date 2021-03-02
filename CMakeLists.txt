cmake_minimum_required(VERSION 3.17)
project(DistributedTemplateLibrary)

set(CMAKE_CXX_STANDARD 20)

file(GLOB tests test/*.cpp)

add_executable(test ${tests})
