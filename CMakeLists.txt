cmake_minimum_required(VERSION 3.1)

project(pickle++ VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./include)
set(TEST_CODE tests/)

include(./cmake/Testing.cmake)
