//
// Created by Sjoerd de Jonge on 06/10/2020.
//

#ifndef CANNYEDGEDETECTOR_IMAGE_H
#define CANNYEDGEDETECTOR_IMAGE_H

#include "Matrix.h"

/**
 * The Image class is the interface between different image file formats and the Matrix class.
 * An Image instance is a special kind of Matrix with uint8_t pixel values (grayscale, RGB, or RGBA).
 *
 * Pipeline:
 * (BMP) file --> Image --> Matrix<double> --> [image_processing] --> Matrix<double> --> Image --> (BMP) file
 */

/// An image is a type of Matrix where the values range from 0-255.
class Image : public Matrix<uint8_t> {
private:
    bool grayscale;

public:
    // Constructors:
    Image(int width, int height, int layers, bool grayscale);  // Constructs an image populated with zeroes
    explicit Image(const Matrix<double>& input); // Constructs an image using a Matrix<double>

    // Transforming methods:
    /**
     * void Image::rgbToGrayscale()
     * Function to convert RGB/BGR images to grayscale images (which follow the rule R == G == B).
     * Any image that is already in grayscale will remain the same.
     */
    void rgbToGrayscale();

    /**
     * void Image::reformatOrigin( ... )
     * Change the formatting of the image pixel array to correspond with top-down (origin is top left) or bottom-up
     * (origin is bottom left) orientation.
     */
    void reformatOrigin(bool isTopDown);

    // Getters:
    /**
     * Image::getImageAverage()
     * Calculate the average pixel value of the whole image.
     */
    [[nodiscard]] double getImageAverage() const; // Returns the average value of an image pixel channel

    // Getter for class variable grayscale:
    [[nodiscard]] bool isGrayScale() const { return grayscale; } // Returns boolean whether grayscale or not
};


#endif //CANNYEDGEDETECTOR_IMAGE_H
