cmake_minimum_required(VERSION 3.1)
project(ComputerAlgebraSystem)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(ComputerAlgebraSystem ${SOURCE_FILES} TreeNode.cpp TreeNode.h Operator.cpp Operator.h Function.cpp Function.h cas.h cas.h cas.cpp)