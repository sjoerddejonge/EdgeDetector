//
// Created by Sjoerd de Jonge on 06/10/2020.
//

#include <iostream>
#include <cmath>
#include "../include/Image.h"

#include "../include/gaussian.h"

Image::Image(const int width, const int height, const int layers, bool grayscale) : Matrix(width, height, layers) {
    this->grayscale = grayscale;
}

// TODO: Make input Matrix templated?
Image::Image(const Matrix<double>& input) : Matrix(input) {
    grayscale = getLayers() == 1;
}

/**
 * void Image::rgbToGrayscale()
 * Function to convert RGB/BGR images to grayscale images (which follow the rule R == G == B).
 * Any image that is already in grayscale will remain the same.
 */
void Image::rgbToGrayscale() {
    // Getting the private, inherited Matrix variables:
    const int channels = getLayers();  // Either 3 or 4 channels (BGR or BGRA)
    const int im_width = getWidth();
    const int im_height = getHeight();
    const std::vector<uint8_t> im_data = getData();
    // Cycle through every pixel of the image:
    for (int y = 0; y < im_height; ++y){
        for (int x = 0; x < im_width; ++x){
            // (Bitmaps are actually in BGR format, but that does not matter for the outcome of this function)
            const int R = im_data[channels * (y * im_width + x) + 0]; // Red value of pixel
            const int G = im_data[channels * (y * im_width + x) + 1]; // Green value of pixel
            const int B = im_data[channels * (y * im_width + x) + 2]; // Blue value of pixel
            // Calculate the intensity (grayscale value) of the pixel:
            const int intensity = static_cast<int>(round((R + G + B) / 3));
            // Rewrite the BGR channels of the pixels to match grayscale intensity:
            setData(channels * (y * im_width + x) + 0, intensity);    // B
            setData(channels * (y * im_width + x) + 1, intensity);    // G
            setData(channels * (y * im_width + x) + 2, intensity);    // R
            // Alpha values remain unchanged
        }
    }
    grayscale = true;
}

/**
 * void Image::reformatOrigin( ... )
 * Change the formatting of the image pixel array to correspond with top-down (origin is top left) or bottom-up (origin
 * is bottom left) orientation.
 */
void Image::reformatOrigin(const bool isTopDown) {
    Image output = *this; // Create a copy of this image to write the new pixel structure to.
    // Getting the private, inherited Matrix variables:
    const int width = getWidth();
    const int height = getHeight();
    const int layers = getLayers();
    if (isTopDown){
        // Origin is top-down, i.e. in the top left corner.
        // Reformat so origin is bottom-up, so in the bottom left corner:
        for (int y = 0; y < height; ++y){
            for (int x = 0; x < width; ++x){
                output.setData( (layers*(y*width + x) + 0),
                                getData((layers*(((height-1)-y)*width + x) + 0)));
                output.setData( (layers*(y*width + x) + 1),
                                getData((layers*(((height-1)-y)*width + x) + 1)));
                output.setData( (layers*(y*width + x) + 2),
                                getData((layers*(((height-1)-y)*width + x) + 2)));
                if (layers == 4){
                    output.setData( (layers*(y*width + x) + 3),
                                    getData((layers*(((height-1)-y)*width + x) + 3)));
                }
            }
        }
    }
    else {
        // Origin is bottom-up, i.e. in the bottom left corner.
        // Reformat so origin is top-down, so in the top left corner:
        for (int y = 0; y < height; ++y){
            for (int x = 0; x < width; ++x){
                output.setData( (layers*(((height-1)-y)*width + x) + 0),
                                getData((layers*(y*width + x) + 0)));
                output.setData( (layers*(((height-1)-y)*width + x) + 1),
                                getData((layers*(y*width + x) + 1)));
                output.setData( (layers*(((height-1)-y)*width + x) + 2),
                                getData((layers*(y*width + x) + 2)));
                if (layers == 4){
                    output.setData( (layers*(((height-1)-y)*width + x) + 3),
                                    getData((layers*(y*width + x) + 3)));
                }
            }
        }
    }
    *this = output; // Copy the newly formatted 'output' to this Image.
}

/**
 * Image::getImageAverage()
 * Calculate the average pixel value of the whole image.
 */
double Image::getImageAverage() const {
    double average = 0;
    // Loop through the image to compute the sum of all elements (stored inside the double 'average'):
    for (const double i : getData()){
        average += i;
    }
    average /= static_cast<double>(getData().size()); // Calculate the average by dividing the sum by the amount of elements in the image
    return average;
}

