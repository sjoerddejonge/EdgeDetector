cmake_minimum_required(VERSION 3.15)
project(CannyEdgeDetector
        VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)

add_executable(EdgeDetector main.cpp BMP.h Matrix.cpp Matrix.h Image.cpp Image.h)