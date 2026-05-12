//
// Created by Sjoerd de Jonge on 11/05/2026.
//

#include "gaussian.h"

#include <iostream>

constexpr double pi = 3.1415926535897;

/// Returns a Gaussian kernel (=a matrix) of w x h with sigma. Width and height must always be odd.
/// Matrix can be 1D (1xN or Nx1) or 2D (MxN or MxM).
Matrix<double> constructGaussianKernel(const int w, const int h, const double sigma) {
    // Validate input; make sure kernel has odd size and size > 0:
    if(w % 2 == 0){
        throw std::invalid_argument("constructGaussianKernel: width of the Gaussian matrix must be odd");
        // Old solution where int w is not const:
        // ++w;
        // std::cout << "Warning: Width of the Gaussian matrix must be odd. Using width = " << w << " instead."
        //          << std::endl;
    }
    if(h % 2 == 0){
        throw std::invalid_argument("constructGaussianKernel: height of the Gaussian matrix must be odd");
        // Old solution where int h is not const:
        // ++h;
        // std::cout << "Warning: Height of the Gaussian matrix must be odd. Using height = " << h << " instead."
        //           << std::endl;
    }
    if(w <= 0 || h <= 0){
        throw std::invalid_argument("constructGaussianKernel: Width and/or height of the Gaussian matrix must be "
                                    "larger than 0");
        // std::cout << "Error: Matrix::constructGaussianKernel. Width and/or height of the Gaussian matrix must be larger "
        //              "than 0. Returning empty 1x1 matrix." << std::endl;
        // return Matrix<double>(1, 1, 1);
    }
    Matrix<double> g(w, h, 1);         // The matrix that will be returned
    const int x_min = static_cast<int>(-floor(w / 2));     // The minimum x value, for a kernel width of 5 we would get -2
    const int y_min = static_cast<int>(-floor(h / 2));     // The minimum y value
    double sum = 0;         // The sum of all elements in the matrix
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // For every element of the matrix:
            const int xval = x_min + x;
            const int yval = y_min + y;

            // Determine whether matrix is 1D-horizontal, 1D-vertical or 2D and construct kernel:
            if (w == 1) {
                // 1D-vertical Gaussian:
                g.setData(y * w + x, (1 / sqrt(2 * pi * pow(sigma, 2))) * exp(-(pow(yval, 2)) / (2 * pow(sigma, 2))));
                sum += g.getData(y * w + x);
            } else if (h == 1) {
                // 1D-horizontal Gaussian:
                g.setData(y * w + x, (1 / sqrt(2 * pi * pow(sigma, 2))) * exp(-(pow(xval, 2)) / (2 * pow(sigma, 2))));
                sum += g.getData(y * w + x);
            } else {
                // 2D Gaussian:
                // Calculating the value for the Gaussian, using 1/(2*pi*sigma^2) * e^((x^2 + y^2)/2*sigma^2):
                g.setData(y * w + x, (1 / (2 * pi * pow(sigma, 2.0))) *
                                      exp(-(pow(xval, 2.0) + pow(yval, 2.0)) / (2 * pow(sigma, 2.0))));
                sum += g.getData(y * w + x);
            }
        }
    }
    // Normalizing the created matrix so the sum of all elements is 1. This is to ensure the image that will be convolved
    // by this kernel will retain its average brightness.
    // See also: https://en.wikipedia.org/wiki/Kernel_(image_processing)#Normalization
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            g.setData(y*w+x, g.getData(y*w+x)/sum); // Dividing each element of the matrix by the sum of all elements
        }
    }
    return g;
}


/// Returns the derivative of a Gaussian kernel (matrix) of w x h with sigma. Width and height must always be odd.
/// Matrix can be 1D (1xN or Nx1) or 2D (MxN or MxM).
/// In case of a 2D matrix, the bool respectToX determines whether the derivative with respect to X is or with respect
/// to Y is taken.
/// Bool respectToX is true by default!
Matrix<double> constructGaussianKernelDerivative(const int w, const int h, const double sigma, const bool respectToX) {
    // Evaluate input; make sure kernel has odd size and size > 0:
    if(w % 2 == 0){
        throw std::invalid_argument("constructGaussianKernelDerivative: width of the Gaussian matrix must be odd");
    }
    if(h % 2 == 0){
        throw std::invalid_argument("constructGaussianKernelDerivative: height of the Gaussian matrix must be odd");
    }
    if(w <= 0 || h <= 0){
        throw std::invalid_argument("constructGaussianKernelDerivative: Width and/or height of the Gaussian matrix "
                                    "must be larger than 0");
    }
    Matrix<double> g(w, h, 1);         // The matrix that will be returned
    int x_min = static_cast<int>(-floor(w / 2));     // The minimum x value, a kernel width of 5 gives -2 as minimum
    int y_min = static_cast<int>(-floor(h / 2));     // The minimum y value
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // For every element of the matrix:
            const int xval = x_min + x;
            const int yval = y_min + y;

            // Determine whether matrix is 1D-horizontal, 1D-vertical or 2D and construct kernel:
            if (w == 1) {
                // 1D-vertical Gaussian:
                // Formula used is: (y / (sigma^3*sqrt(2*pi)) * exp((-y^2)/(2*sigma^2))
                // This formula, and the other derivative formulas below, should start with a negative sign, but due to the
                // different way coordinates are handled in images vs graphs, this is omitted to compensate for the
                // difference.
                g.setData(y*w+x, (yval)/(pow(sigma, 3)*sqrt(2*pi))*exp(-(pow(yval, 2))/(2*pow(sigma, 2))));
            } else if (h == 1) {
                // 1D-horizontal Gaussian:
                // Formula used is: (x / (sigma^3*sqrt(2*pi)) * exp((-x^2)/(2*sigma^2))
                g.setData(y*w+x, (xval)/(pow(sigma, 3)*sqrt(2*pi))*exp(-(pow(xval, 2))/(2*pow(sigma, 2))));
            } else {
                // 2D Gaussian:
                // Calculating the value for the derivative of a Gaussian
                if (respectToX){
                    // Formula used is: (x)/(2*pi*sigma^4) * exp(-(x^2+y^2)/(2*sigma^2))
                    g.setData(y*w+x, (xval)/(2*pi*pow(sigma,4))*exp(-(pow(xval,2)+pow(yval,2))/(2*pow(sigma,2))));
                } else {
                    // Formula used is: -(y)/(2*pi*sigma^4) * exp(-(x^2+y^2)/(2*sigma^2))
                    g.setData(y*w+x, (yval)/(2*pi*pow(sigma,4))*exp(-(pow(xval,2)+pow(yval,2))/(2*pow(sigma,2))));
                }
            }
        }
    }

    // Normalizing is not needed for the derivative of a Gaussian, since Matrix::matrixToImage() will be used for this
    // purpose.
    return g;
}
