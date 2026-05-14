//
// Created by Sjoerd de Jonge on 11/05/2026.
//

#ifndef CANNYEDGEDETECTOR_GAUSSIAN_H
#define CANNYEDGEDETECTOR_GAUSSIAN_H
#include "Matrix.h"

// Returns a Gaussian matrix with given sigma
Matrix<double> constructGaussianKernel(int w, int h, double sigma);

// Returns the derivative of a Gaussian matrix
Matrix<double> constructGaussianKernelDerivative(int w, int h, double sigma, bool respectToX = true);

#endif //CANNYEDGEDETECTOR_GAUSSIAN_H
