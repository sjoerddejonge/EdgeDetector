//
// Created by Sjoerd de Jonge on 13/05/2026.
//

#ifndef CANNYEDGEDETECTOR_CANNY_H
#define CANNYEDGEDETECTOR_CANNY_H
#include "Matrix.h"


void blurGaussian(double sigma, int kernel_type);
void reformatOrigin(bool isTopDown); // Reformats the origin of the image to be top-down or bottom-up
Matrix<double> gradientMagnitude(const Matrix<double>& im_derivX, const Matrix<double>& im_derivY);
double getImageAverage() const; // Returns the average value of an image pixel channel


#endif //CANNYEDGEDETECTOR_CANNY_H
