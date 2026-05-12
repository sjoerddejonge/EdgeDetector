//
// Created by Sjoerd de Jonge on 11/05/2026.
//

#ifndef EDGEDETECTOR_GAUSSIAN_H
#define EDGEDETECTOR_GAUSSIAN_H
#include "include/Matrix.h"

// Returns a Gaussian matrix with given sigma
Matrix<double> constructGaussianKernel(int w, int h, double sigma);

// Returns the derivative of a Gaussian matrix
Matrix<double> constructGaussianKernelDerivative(int w, int h, double sigma, bool respectToX = true);

#endif //EDGEDETECTOR_GAUSSIAN_H
