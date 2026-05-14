//
// Created by Sjoerd de Jonge on 06/10/2020.
//

#ifndef CANNYEDGEDETECTOR_IMAGE_H
#define CANNYEDGEDETECTOR_IMAGE_H

#include "Matrix.h"

/// An image is a type of Matrix where the values range from 0-255.
class Image : public Matrix<uint8_t> {
private:
    bool grayscale;

public:
    // Class functions:
    Image(int width, int height, int layers, bool grayscale);  // Constructs an image
    explicit Image(const Matrix<double>& input); // Constructs an image using a Matrix<double>
    void convolve(const Matrix<double>& kernel);
    void blurGaussian(double sigma, int kernel_type);
    void rgbToGrayscale(); // Converts RGB/BGR image to grayscale
    void reformatOrigin(bool isTopDown); // Reformats the origin of the image to be top-down or bottom-up
    Image gradientMagnitude(const Image& im_derivX, const Image& im_derivY);
    double getImageAverage() const; // Returns the average value of an image pixel channel

    // Getter for class variable grayscale:
    inline bool isGrayScale() const { return grayscale; } // Returns boolean whether grayscale or not
};


#endif //CANNYEDGEDETECTOR_IMAGE_H
