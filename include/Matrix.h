//
// Created by Sjoerd de Jonge on 21/07/2020.
//

#ifndef EDGEDETECTOR_MATRIX_H
#define EDGEDETECTOR_MATRIX_H

#include <iostream>
#include <vector>

template <typename T>
class Matrix {
private:
    int width;          // Width of the matrix
    int height;         // Height of the matrix
    int layers;         // In case of a 3D matrix
    std::vector<T> data;

public:
    // Class functions:

    /// Construct a matrix with int width, int height and int layers and default values 0 for data.
    Matrix(int width, int height, int layers) {
        // Evaluate input; make sure size > 0:
        if (width <= 0 || height <=0 || layers <=0){
            throw std::invalid_argument("Matrix::Matrix: width, height and/or layers must be larger than 0");
        }
        this->width = width;
        this->height = height;
        this->layers = layers;
        data.resize( width * height * layers, 0);
    }

    /// Transform any matrix into the byte image range (0 to 255). Similar to MATLAB's mat2gray, which source code was
    /// studied to make this function in C++.
    void matrixToImage() {
        // Get the max and min value of the matrix:
        const T max = *std::max_element(std::begin(data), std::end(data));
        // (std::max_element() returns an iterator so the dereference operator is used)
        const T min = *std::min_element(std::begin(data), std::end(data));

        // Delta is the weight factor for each pixel:
        const T delta = 255/(max-min);

        // Range-based for loop through the data of the matrix:
        for (T & i : data){
            i *= delta;
            i += -min*delta;

            // Make sure all values are between 0-255:
            if (i < 0){
                i = 0;
            } else if (i > 255){
                i = 255;
            }
        }
    }

    // Getters, these are constant:
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getLayers() const { return layers; }
    const std::vector<T>& getData() const { return data; } // Returns vector which contains the matrix values
    T getData(int i) const { return data[i]; }

    // Setters:
    /// Change the private integer 'width' of the matrix and resize matrix accordingly.
    void setWidth(const int new_width) {
        if (0 < new_width){
            width = new_width;
            data.resize(width * height);
        } else{
            std::cerr << "Error: Matrix::setWidth. Values equal to or smaller than 0 are not allowed for setting matrix width."
            << std::endl;
        }
    }
    /// Change the private integer 'height' of the matrix and resize matrix accordingly.
    void setHeight(const int new_height) {
        if (0 < new_height){
            height = new_height;
            data.resize(width * height);
        } else{
            std::cerr << "Error: Matrix::setHeight. Values equal to or smaller than 0 are not allowed for setting matrix height."
            << std::endl;
        }
    }
    /// Change the private vector<T> 'data[i]' of the matrix.
    void setData(int i, T new_data) {
        // Set a single point on the data vector using index value
        if( (0 <= i) && (i <= (data.size()-1)) ){
            data[i] = new_data;
        }
        else{
            std::cout << "Error: Matrix::setData. Cannot set matrix data. Index out of range. Max index size is: " << data.size()-1
            << std::endl;
        }
    }
    /// Change the private vector<T> 'data[y * getWidth() + x]' of the matrix.
    void setData(int x, int y, T new_data) {
        // Set a single point on the data vector using matrix' x,y values
        --x; // Decrease x by 1, because vector index starts at 0
        --y; // Decrease y by 1, because vector index starts at 0
        if( 0 <= (y * getWidth() + x) && (y * getWidth() + x) <= (data.size()-1) ){
            data[y * getWidth() + x] = new_data;
        }
        else{
            std::cout << "Error: Matrix::setData. Cannot set matrix data. Index out of range. Max index size is: " << data.size()-1
            << std::endl;
        }
    }
    /// Set a full vector as the data
    void setData(std::vector<T> new_data) {
        if (new_data.size() != width*height*layers){
            std::cout << "Error: Matrix::setData. Cannot set matrix data, vector does not match matrix size." << std::endl;
        }
        else{
            data = new_data;
        }
    }
};


#endif //EDGEDETECTOR_MATRIX_H
