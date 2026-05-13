cmake_minimum_required(VERSION 3.15)
project(CannyEdgeDetector
        VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)

add_executable(CannyEdgeDetector main.cpp BMP.h include/Matrix.h Image.cpp include/Image.h
        Gaussian.cpp
        include/gaussian.h)