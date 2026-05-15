cmake_minimum_required(VERSION 3.15)
project(CannyEdgeDetector
        VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

add_executable(CannyEdgeDetector
        main.cpp
        include/BMP.h
        include/Matrix.h
        src/Image.cpp
        include/Image.h
        src/gaussian.cpp
        include/gaussian.h
        src/image_processing.cpp
        include/image_processing.h
        include/constants.h)
# Add ../include as a known include directory so we can use "canny.h" instead of "../include/canny.h"
target_include_directories(${PROJECT_NAME} PRIVATE include)