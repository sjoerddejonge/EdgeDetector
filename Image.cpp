//
// Created by Sjoerd de Jonge on 06/10/2020.
//

#include <iostream>
#include <cmath>
#include "include/Image.h"

#include "include/gaussian.h"

Image::Image(int width, int height, int layers, bool grayscale) : Matrix(width, height, layers) {
    this->grayscale = grayscale;
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
    average /= getData().size(); // Calculate the average by dividing the sum by the amount of elements in the image
    return average;
}

/**
 * void convolve( ... )
 * Function that aligns the center of the kernel matrix with each pixel of the image. The overlapping kernel and image
 * elements are then multiplied and summed. The result will be the new value for the current pixel. A larger kernel
 * results in a larger amount of operations per pixel.
 */
void Image::convolve(const Matrix<double> &kernel) {
    // Copy of this image that will overwrite the current image at the end of the convolution
    Image copy = *this;
    // If the kernel has an even size, there is no centerpoint and convolution cannot be done:
    if (kernel.getWidth() % 2 == 0 || kernel.getHeight() % 2 == 0){
        std::cout << "Unable to convolve image, kernel width and/or height must be an odd number. Kernel width: "<<
                  kernel.getWidth() << ". Kernel height: " << kernel.getHeight() << ". " << std::endl;
        return;
    }
    // Kernel cannot be a 3D Matrix:
    if (kernel.getLayers() > 1){
        std::cout << "Unable to convolve image, kernel cannot be a 3D Matrix." << std::endl;
        return;
    }
    // Creating constant copies of the image parameters:
    const int im_width = getWidth();
    const int im_height = getHeight();
    const int channels = getLayers();  // Either 3 or 4 channels (BGR or BGRA)
    const std::vector<uint8_t>& im_data = getData();
    // Creating constant copies of the kernel parameters:
    const int k_width = kernel.getWidth();
    const int k_height = kernel.getHeight();
    const std::vector<double>& k_data = kernel.getData();

    // Looping through the image:
    for (int y = 0; y < im_height; ++y){
        for (int x = 0; x < im_width; ++x){
            double acc = 0;     // Accumulator for the new pixel value. If image is in color, this represents the blue channel.
            double acc_g = 0;   // Accumulator for the green pixel value if image is in color.
            double acc_r = 0;   // Accumulator for the red pixel value if image is in color.

            // For each pixel in the image, this loops through the kernel elements:
            for (int r = 0; r < k_height; ++r){   // For each kernel row
                for (int c = 0; c < k_width; ++c){   // For each kernel column
                    // The center (origin) of the kernel is placed on the current image pixel. The kernel will overlap
                    // the neighbouring pixels. The X and Y coordinates of these pixels are calculated first.
                    // In c++ positive integers are always truncated towards 0, so using floor() is not required here.
                    int posX = x-k_width/2+c;   // posX = 'position X', the X-coordinate on the image which overlaps with the current point (c,r) on the kernel.
                    int posY = y-k_height/2+r;  // posY = 'position Y', Y-coordinate on the image that is overlapped with (c,r) of the kernel
                    // Here it is verified whether the coordinates lie within the image borders, if not they are
                    // mirrored into the image:
                    if ( (0 > posX) || (posX >= im_width) || (0 > posY) || (posY >= im_height) ){
                        // If negative value, mirror the coordinates into the image:
                        posX = abs(posX);
                        posY = abs(posY);
                        // If coordinates are larger than image width, mirror coordinates into image:
                        posX = (im_width-1) - abs(posX - (im_width-1));
                        posY = (im_height-1) - abs(posY - (im_height-1));
                        //... and no extra if-statements are needed! :)
                        // Best way to visualize this is to calculate an example, say im_width = 240, im_height = 240
                        // and posX = 245 (outside the image): (240-1) - abs(245 - (240-1)) = 239 - 6 = 233, which is
                        // as close to 239 as 245 is.
                        // But if posX = 233 (inside the image), we get: (240-1) - abs(233 - (240-1)) = 239 - 6 = 233,
                        // so nothing changes.
                        // Btw, 'im_width-1' (etc) is used, because, for example, a width of 240 pixels means the maximum
                        // image coordinate is 239, because coordinates starts at 0 (but width/height starts at 1).
                    }
                    // The value of the overlapping kernel and image are multiplied and added to the accumulator.
                    acc += im_data[channels * (posY * im_width + posX) + 0] * k_data[r*k_width + c];
                    // If image is not grayscale, green and red channels need to be calculated too:
                    if (!isGrayScale()) {
                        acc_g += im_data[channels * (posY * im_width + posX) + 1] * k_data[r * k_width + c];
                        acc_r += im_data[channels * (posY * im_width + posX) + 2] * k_data[r * k_width + c];
                    }
                }
            }
            // Write the accumulated value to the BGR channels of the pixels of the output image:
            //output.data[channels * (y * im_width + x) + 0] = acc; <- this was the old 'public-variable' version
            copy.setData(channels * (y * im_width + x) + 0, acc);
            //std::cout << acc << std::endl;
            if(isGrayScale()){
                copy.setData(channels * (y * im_width + x) + 1, acc);
                copy.setData(channels * (y * im_width + x) + 2, acc);
            } else{
                copy.setData(channels * (y * im_width + x) + 1, acc_g);
                copy.setData(channels * (y * im_width + x) + 2, acc_r);
            }
        }
    }
    // Overwrite the current image with the copy that was processed:
    *this = copy;
}

/**
 * void blurGaussian( ... )
 * Constructs a Gaussian kernel and uses the Image::convolve() function for convolution using that kernel.
 * Kernel types are:
 * 0: Normal Gaussian blur
 * 1: Gaussian blur with derivative with respect to X
 * 2: Gaussian blur with derivative with respect to Y
 * 3: Gaussian blur with derivative with respect to XY
 */
void Image::blurGaussian(double sigma, int kernel_type) {
    if (kernel_type <= 0 || kernel_type >= 3){
        std::cout << "ERROR with blurGaussian(): Invalid kernel_type. Using default kernel_type = " << 0 << std::endl;
    }
    int size = 2* static_cast<int>(ceil(sigma * 4))+1;   // Calculate the kernel size based on the sigma
    Matrix<double> g_hor(size, 1, 1);      // Empty horizontal kernel matrix of 1 x size
    Matrix<double> g_ver(1, size, 1);      // Empty vertical  kernel matrix of size x 1
    // Constructing Gaussian kernels:
    switch (kernel_type){
        case 0:     // Normal Gaussian blur
            g_hor = constructGaussianKernel(g_hor.getWidth(), g_hor.getHeight(), sigma);
            g_ver = constructGaussianKernel(g_ver.getWidth(), g_ver.getHeight(), sigma);
            break;
        case 1:     // Derivative of X
            g_hor = constructGaussianKernelDerivative(g_hor.getWidth(), g_hor.getHeight(), sigma);
            g_ver = constructGaussianKernel((g_ver.getWidth()), g_ver.getHeight(), sigma);
            break;
        case 2:     // Derivative of Y
            g_hor = constructGaussianKernel(g_hor.getWidth(), g_hor.getHeight(), sigma);
            g_ver = constructGaussianKernelDerivative((g_ver.getWidth()), g_ver.getHeight(), sigma);
            break;
        case 3:     // Derivative of XY
            g_hor = constructGaussianKernelDerivative(g_hor.getWidth(), g_hor.getHeight(), sigma);
            g_ver = constructGaussianKernelDerivative((g_ver.getWidth()), g_ver.getHeight(), sigma);
            break;
        default:    // Normal Gaussian blur
            g_hor = constructGaussianKernel(g_hor.getWidth(), g_hor.getHeight(), sigma);
            g_ver = constructGaussianKernel(g_ver.getWidth(), g_ver.getHeight(), sigma);
            break;
    }
    // Convolve our image with the two Gaussian matrices, resulting in a new matrix:
    convolve(g_hor);
    convolve(g_ver);
}

/**
 * void gradientMagnitude( ... )
 * Calculate the gradient magnitude using the X and Y derivatives of an image. The X and Y derivatives are
 * obtained by convolving an image with the derivative of a Gaussian with respect to X or Y.
 * The formula for calculating the gradient magnitude is:
 * Gradient magnitude = sqrt(derivX^2 + derivY^2)
 */
Image Image::gradientMagnitude(const Image &im_derivX, const Image &im_derivY) {
    const int width = im_derivX.getWidth();
    const int height = im_derivX.getHeight();
    const int channels = im_derivX.getLayers();
    Image copy = im_derivX;   // Create a copy of im_derivX that will be the output

    // Check if both input matrices are equal in dimensions:
    if (width != im_derivY.getWidth() || height != im_derivY.getHeight()){
        throw std::runtime_error("ERROR: To calculate gradient magnitude both input images should have similar size!");
    }

    // Loop through all the pixels of the output image/matrix:
    for (int y = 0; y < height; ++y){
        for (int x = 0; x < width; ++x){
            // Index for the 'blue' value of the image, of the current pixel:
            const int b_index = (channels*(y * width+ x)+0);  // image is grayscale so green and red values are the same

            // Gradient magnitude value for that pixel, using the pixels of im_derivX and im_derivY:
            // The formula is: Magnitude = sqrt( derivativeX^2 + derivativeY^2 )
            const double grad_value = sqrt(pow(im_derivX.getData()[b_index],2) + pow(im_derivY.getData()[b_index],2));

            // Set the blue pixel value for the output matrix as the calculated gradient:
            copy.setData(b_index,grad_value);

            // Green and red channel are same as blue channel because image is in grayscale:
            copy.setData(b_index+1,grad_value); // green channel = same as blue channel
            copy.setData(b_index+2,grad_value); // red channel = same as blue channel
        }
    }
    // Return the copy as output of this function:
    return copy;
}

