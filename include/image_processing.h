//
// Created by Sjoerd de Jonge on 13/05/2026.
//

#ifndef CANNYEDGEDETECTOR_IMAGE_PROCESSING_H
#define CANNYEDGEDETECTOR_IMAGE_PROCESSING_H
#include "Matrix.h"

/**
 * Image processing is done on the Matrix<double> class for extended precision (the Image class has a vector<uint8_t>).
 */

double getImageAverage(const Matrix<double>& input);
Matrix<double> convolve(const Matrix<double>& input, const Matrix<double>& kernel); // Convolution
Matrix<double> gaussianBlur(const Matrix<double>& input, double sigma, int kernel_type);
Matrix<double> gradientMagnitude(const Matrix<double>& im_derivX, const Matrix<double>& im_derivY);
Matrix<double> nonMaximumSuppression(const Matrix<double>& gradMag, const Matrix<double>& im_derivX, const Matrix<double>& im_derivY);
void hysteresisThresholding(Matrix<double> &gradMag, double high_threshold, double low_threshold);

#endif //CANNYEDGEDETECTOR_IMAGE_PROCESSING_H
