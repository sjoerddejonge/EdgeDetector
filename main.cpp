
/**
 * @author Sjoerd de Jonge | s1738798
 * Edge detector script to use with .bmp type images.
 */

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "include/BMP.h"
#include "include/gaussian.h"
#include "include/image_processing.h"
#include "include/Matrix.h"
#include "include/Image.h"

/// Functions:
// Image input/output:
Image readBMP(const std::string& filename); // Read .bmp image as Image
void writeBMP(Image input, const std::string& filename_write); // Write Image to .bmp image file
// Image manipulation:
Image convolve(Image& image, const Matrix<double>& kernel); // Convolve Image with kernel
Image gaussianBlur(Image& image, double sigma, int kernel_type); // Blur Image with Gaussian or its derivatives
Image gradientMagnitude(const Image& im_derivX, const Image& im_derivY); // Calculate the gradient magnitude using the Gaussian derivative of x and y of an Image.
Image nonMaximumSuppression(const Image& gradMag, const Image& im_derivX, const Image& im_derivY); // Suppress all non-maximum values in the gradient magnitude Image.
void hysteresisThresholding(Image &gradMag, double high_threshold, double low_threshold); // Threshold the found edges using hysteresis thresholding.

/// Variables:
constexpr double pi = 3.1415926535897;  // Global variable pi

// TODO:
// - Add support for more image filetypes!
// - Change console interface to accept user commands, like Matlab (something like: "edgedetection(2,10,0.5)")
// - Add function to overlay the edge map on the original image
// - ...and much more!

int main() {
    // Variables used throughout the process:
    double sigma = 2; // Sigma for Gaussian blur
    double high_threshold = 10; // High threshold for hysteresis thresholding
    double low_threshold = 0.5; // Low threshold for hysteresis thresholding, equal to high threshold at first
    std::string filename;
    std::string filename_write; // String to store the new filename, used for writing .bmp images


    /// Welcome message:
    std::cout << "This program can be used to detect edges on a bitmap (.bmp) image using the Canny edge detector "
                 "method."<< std::endl;
    std::cout << "Only 24 and 32 bits depth images in the BGR and BGRA color space are supported." << std::endl;
    std::cout << "The sigma value, high threshold and low threshold can be determined iteratively." << std::endl;

    /// User input of image file location and reading of image:
    bool endOfLoop = false; // If true, the loop will end
    Image input_image(1,1,1,true); // Placeholder Image for the input image
    do{
        std::cout << "Enter the location path of the bitmap image to read: " << std::endl;
        std::getline(std::cin, filename); // F:\OneDrive\[Master]\Programming in Engineering\EdgeDetector\ang2.bmp
        std::cout << "Reading image..." << std::endl;
        // This uses the error handling that was used in BMP.h, which can throw a runtime_error:
        try{
            input_image = readBMP(filename); // Returns a grayscale image
        }
        catch(std::runtime_error& e){
            // Catch the runtime_error that might be thrown by the read() function in BMP.h
            std::cout << e.what() << std::endl; // Display the runtime error
            continue; // Start loop over, code below this will not be reached
        }
        endOfLoop = true;
    }
    while(!endOfLoop);
    auto input_matrix = Matrix<double>(input_image);
    filename = filename.substr(0,filename.find_last_of('.')); // Remove the .bmp part of the filename

    /// User input of the sigma
    std::cout << "Enter the sigma value:" << std::endl;
    std::cin >> sigma;
    while (!std::cin) {
        std::cin.clear();
        // The following line solves a problem where the re-input was ignored. Solution to this problem was found here:
        // https://stackoverflow.com/questions/35382436/cin-getline-not-waiting-for-input-and-setting-parameters-on-rand
        std::cin.ignore(1, '\n');
        std::cout << "Input was not a number, enter a valid sigma:" << std::endl;
        std::cin >> sigma;
    }

    /// Iteratively determine the right sigma, high threshold and low threshold:
    endOfLoop = false; // If true, the loop will end
    do {
        /// Blur image with Gaussian derivative of X (fX):
        std::cout << "Blurring the image with Gaussian blur and computing derivatives...\n";

        Matrix<double> deriv_x = gaussianBlur(input_matrix, sigma, 1);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_fX.bmp"; // Add suffix to the name to indicate it is the derivative of X
        writeBMP(Image(deriv_x), filename_write); // Write derivX to .bmp

        /// Blur image with Gaussian derivative of Y (fY):
        Matrix<double> deriv_y = gaussianBlur(input_matrix, sigma, 2);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_fY.bmp"; // Add suffix to the name to indicate it is the derivative of Y
        writeBMP(Image(deriv_y), filename_write); // Write derivY to .bmp

        /// Calculate gradient magnitude:
        std::cout << "Calculating gradient magnitude...\n";
        Matrix<double> grad_mag = gradientMagnitude(deriv_x, deriv_y);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_GradientMagnitude.bmp";
        writeBMP(Image(grad_mag), filename_write);

        /// Non maximum suppression:
        std::cout << "Performing non-maximum suppression..." << std::endl;
        grad_mag = nonMaximumSuppression(grad_mag, deriv_x, deriv_y);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_GradientMagnitudeSuppressed.bmp";
        writeBMP(Image(grad_mag), filename_write);

        /// Hysteresis thresholding:
        std::cout << "The following information is useful for determining the high and low threshold:" << std::endl;
        std::cout << "Maximum pixel value is: "
                  << *std::ranges::max_element(grad_mag.getData()) << std::endl;
        std::cout << "Minimum pixel value is: " << *std::ranges::min_element(grad_mag.getData()) << std::endl;
        std::cout << "Average pixel value is: " << getImageAverage(grad_mag) << std::endl;
        std::cout << "Using this information, enter the high threshold value:" << std::endl;
        std::cin >> high_threshold;
        while (!std::cin) {
            std::cin.clear();
            std::cin.ignore(1, '\n');
            std::cout << "Input was not a number, enter a valid high threshold:" << std::endl;
            std::cin >> high_threshold;
        }
        std::cout << "Now enter the low threshold: " << std::endl;
        std::cin >> low_threshold;
        while (!std::cin) {
            std::cin.clear();
            std::cin.ignore(1, '\n');
            std::cout << "Input was not a number, enter a valid low threshold:" << std::endl;
            std::cin >> low_threshold;
        }
        Matrix<double> edge_map = grad_mag;
        std::cout << "Performing hysteresis thresholding..." << std::endl;
        hysteresisThresholding(edge_map, high_threshold, low_threshold);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_EdgeMap.bmp";
        writeBMP(Image(edge_map), filename_write);

        std::cout << std::endl;
        std::cout << "The resulting edgemap is written to: " << filename_write << std::endl;
        std::cout << "Inspect the result." << std::endl;
        std::cout << "  - If satisfactory, press ENTER to exit." << std::endl;
        std::cout << "  - Otherwise, enter a sigma value again: " << std::endl;
        std::string buf = "not empty";  // String buffer for input is not empty

        // The following line solves a problem where the input was ignored. Solution to this problem was found here:
        // https://stackoverflow.com/questions/35382436/cin-getline-not-waiting-for-input-and-setting-parameters-on-rand
        std::cin.ignore(1, '\n');

        std::getline(std::cin, buf); // Extract user input as string
        if (buf.empty()){
            endOfLoop = true; // End loop
        }
        else{
            sigma = std::stod(buf); // Convert string buf to double
            std::cout << "New sigma is " << sigma << "." << std::endl;
        }
    }
    while(!endOfLoop);
    return 0;
}


/**
 * Image readBMP( ... )
 * Reads .bmp file and returns as Image. The image is converted to grayscale and origin is set to be top-left (i.e. top-
 * down format).
 */
Image readBMP(const std::string& filename){
    // Construct BMP with filename:
    BMP input_bmp(filename);
    // Create an empty Image, equal to the size of the BMP, we assume the image is not grayscale:
    Image input_bmp_image(input_bmp.dib_header.width, input_bmp.dib_header.height, input_bmp.dib_header.bit_count / 8, false);
    // Read image pixel data from the bmp as vector<uint8_t>:
    std::vector<uint8_t> im_data(input_bmp.data.begin(), input_bmp.data.end());
    // Fill the empty Image with the image pixel data:
    input_bmp_image.setData(im_data);
    // Convert rgb to grayscale:
    input_bmp_image.rgbToGrayscale();
    // Reformat origin to be top-down (since BMPs in this implementation can only be bottom-up):
    input_bmp_image.reformatOrigin(false);
    return input_bmp_image;
}


/**
 * void writeBMP( ... )
 * Writes the Image into a .bmp image file.
 */
void writeBMP(Image input, const std::string& filename_write){
    input.matrixToImage(); // Transform the data to be into the 0 - 255 range.
    input.reformatOrigin(true);
    // Create a new vector<uint8_t> to store the image data as integers (which is needed for bmp):
    const std::vector<uint8_t> image_data(input.getData().begin(), input.getData().end());
    const int32_t width = input.getWidth();
    const int32_t height = input.getHeight();
    bool has_alpha = true;
    if (input.getLayers() <= 3){
        has_alpha = false;
    }
    BMP imageBMP(width,height,has_alpha);       // Create new BMP similar to image size.
    imageBMP.data = image_data;                 // Set image matrix pixel data as bmp pixel data.
    imageBMP.write(filename_write);
}


// Image convolve(Image& image, const Matrix<double>& kernel){
//     // Output matrix that will be returned by this function starts as a copy of the input image matrix:
//     Image output = image;
//     // If the kernel has an even size, there is no centerpoint and convolution cannot be done:
//     if (kernel.getWidth() % 2 == 0 || kernel.getHeight() % 2 == 0){
//         std::cout << "Unable to convolve image, kernel width and/or height must be an odd number. Kernel width: "<<
//                 kernel.getWidth() << ". Kernel height: " << kernel.getHeight() << ". " << std::endl;
//         return image;
//     }
//     // Kernel cannot be a 3D Matrix:
//     if (kernel.getLayers() > 1){
//         std::cout << "Unable to convolve image, kernel cannot be a 3D Matrix." << std::endl;
//         return image;
//     }
//     // Creating constant copies of the image parameters:
//     const int im_width = image.getWidth();
//     const int im_height = image.getHeight();
//     const int channels = image.getLayers();  // Either 3 or 4 channels (BGR or BGRA)
//     const std::vector<uint8_t>& im_data = image.getData();
//     // Creating constant copies of the kernel parameters:
//     const int k_width = kernel.getWidth();
//     const int k_height = kernel.getHeight();
//     const std::vector<double>& k_data = kernel.getData();
//
//     double acc; // Accumulator for the new pixel value. If image is in color, this represents the blue channel.
//     double acc_g; // Accumulator for the green pixel value if image is in color.
//     double acc_r; // Accumulator for the red pixel value if image is in color.
//
//     // Looping through the image:
//     for (int y = 0; y < im_height; ++y){
//         for (int x = 0; x < im_width; ++x){
//             acc = 0;
//             acc_g = 0;
//             acc_r = 0;
//
//             // For each pixel in the image, this loops through the kernel elements:
//             for (int r = 0; r < k_height; ++r){   // For each kernel row
//                 for (int c = 0; c < k_width; ++c){   // For each kernel column
//                     // The center (origin) of the kernel is placed on the current image pixel. The kernel will overlap
//                     // the neighbouring pixels. The X and Y coordinates of these pixels are calculated first.
//                     // In c++ positive integers are always truncated towards 0, so using floor() is not required here.
//                     int posX = x-k_width/2+c;   // posX = 'position X', the X-coordinate on the image which overlaps with the current point (c,r) on the kernel.
//                     int posY = y-k_height/2+r;  // posY = 'position Y', Y-coordinate on the image that is overlapped with (c,r) of the kernel
//                     // Here it is verified whether the coordinates lie within the image borders, if not they are
//                     // mirrored into the image:
//                     if ( (0 > posX) || (posX >= im_width) || (0 > posY) || (posY >= im_height) ){
//                         // If negative value, mirror the coordinates into the image:
//                         posX = abs(posX);
//                         posY = abs(posY);
//                         // If coordinates are larger than image width, mirror coordinates into image:
//                         posX = (im_width-1) - abs(posX - (im_width-1));
//                         posY = (im_height-1) - abs(posY - (im_height-1));
//                         //... and no extra if-statements are needed! :)
//                         // Best way to visualize this is to calculate an example, say im_width = 240, im_height = 240
//                         // and posX = 245 (outside the image): (240-1) - abs(245 - (240-1)) = 239 - 6 = 233, which is
//                         // as close to 239 as 245 is.
//                         // But if posX = 233 (inside the image), we get: (240-1) - abs(233 - (240-1)) = 239 - 6 = 233,
//                         // so nothing changes.
//                         // Btw, 'im_width-1' (etc) is used, because, for example, a width of 240 pixels means the maximum
//                         // image coordinate is 239, because coordinates starts at 0 (but width/height starts at 1).
//                     }
//                     // The value of the overlapping kernel and image are multiplied and added to the accumulator.
//                     acc += im_data[channels * (posY * im_width + posX) + 0] * k_data[r*k_width + c];
//                     // If image is not grayscale, green and red channels need to be calculated too:
//                     if (!image.isGrayScale()) {
//                         acc_g += im_data[channels * (posY * im_width + posX) + 1] * k_data[r * k_width + c];
//                         acc_r += im_data[channels * (posY * im_width + posX) + 2] * k_data[r * k_width + c];
//                     }
//                 }
//             }
//             // Write the accumulated value to the BGR channels of the pixels of the output image:
//             //output.data[channels * (y * im_width + x) + 0] = acc; <- this was the old 'public-variable' version
//             output.setData(channels * (y * im_width + x) + 0, acc);
//             //std::cout << acc << std::endl;
//             if(image.isGrayScale()){
//                 output.setData(channels * (y * im_width + x) + 1, acc);
//                 output.setData(channels * (y * im_width + x) + 2, acc);
//             } else{
//                 output.setData(channels * (y * im_width + x) + 1, acc_g);
//                 output.setData(channels * (y * im_width + x) + 2, acc_r);
//             }
//
//         }
//     }
//     return output;
// }

// Image gaussianBlur(Image& image, double sigma, int kernel_type){
//     if (kernel_type <= 0 || kernel_type >= 3){
//         std::cerr << "ERROR with gaussianBlur(): Invalid kernel_type. Using default kernel_type = " << 0 << std::endl;
//     }
//     int size = 2* static_cast<int>(ceil(sigma * 4))+1;   // Calculate the kernel size based on the sigma
//     Matrix<double> g_hor(size, 1, 1);      // Empty horizontal kernel matrix of 1 x size
//     Matrix<double> g_ver(1, size, 1);      // Empty vertical  kernel matrix of size x 1
//     // Constructing Gaussian kernels:
//     switch (kernel_type){
//         case 0:     // Normal Gaussian blur
//             g_hor = constructGaussianKernel(g_hor.getWidth(), g_hor.getHeight(), sigma);
//             g_ver = constructGaussianKernel(g_ver.getWidth(), g_ver.getHeight(), sigma);
//             break;
//         case 1:     // Derivative of X
//             g_hor = constructGaussianKernelDerivative(g_hor.getWidth(), g_hor.getHeight(), sigma);
//             g_ver = constructGaussianKernel((g_ver.getWidth()), g_ver.getHeight(), sigma);
//             break;
//         case 2:     // Derivative of Y
//             g_hor = constructGaussianKernel(g_hor.getWidth(), g_hor.getHeight(), sigma);
//             g_ver = constructGaussianKernelDerivative((g_ver.getWidth()), g_ver.getHeight(), sigma);
//             break;
//         case 3:     // Derivative of XY
//             g_hor = constructGaussianKernelDerivative(g_hor.getWidth(), g_hor.getHeight(), sigma);
//             g_ver = constructGaussianKernelDerivative((g_ver.getWidth()), g_ver.getHeight(), sigma);
//             break;
//         default:    // Normal Gaussian blur
//             g_hor = constructGaussianKernel(g_hor.getWidth(), g_hor.getHeight(), sigma);
//             g_ver = constructGaussianKernel(g_ver.getWidth(), g_ver.getHeight(), sigma);
//             break;
//     }
//     // Convolve our image with the two Gaussian matrices, resulting in a new matrix:
//     Image output = convolve(image, g_hor);
//     output = convolve(output, g_ver);
//     return output;
// }

// Image gradientMagnitude(const Image& im_derivX, const Image& im_derivY){
//     const int width = im_derivX.getWidth();
//     const int height = im_derivX.getHeight();
//     const int channels = im_derivX.getLayers();
//     Image output = im_derivX;   // Create a copy of im_derivX that will be the output
//
//     // Check if both input matrices are equal in dimensions:
//     if (width != im_derivY.getWidth() || height != im_derivY.getHeight()){
//         std::cout << "ERROR: To calculate gradient magnitude both input images should have similar size!" << std::endl;
//         return Image(1,1,1,true);
//     }
//
//     // Loop through all the pixels of the output image/matrix:
//     for (int y = 0; y < height; ++y){
//         for (int x = 0; x < width; ++x){
//             // Index for the 'blue' value of the image, of the current pixel:
//             int b_index = (channels*(y * width+ x)+0);  // image is grayscale so green and red values are the same
//
//             // Gradient magnitude value for that pixel, using the pixels of im_derivX and im_derivY:
//             // The formula is: Magnitude = sqrt( derivativeX^2 + derivativeY^2 )
//             double grad_value = sqrt(pow(im_derivX.getData()[b_index],2) + pow(im_derivY.getData()[b_index],2));
//
//             // Set the blue pixel value for the output matrix as the calculated gradient:
//             output.setData(b_index,grad_value);
//
//             // Green and red channel are same as blue channel because image is in grayscale:
//             output.setData(b_index+1,grad_value); // green channel = same as blue channel
//             output.setData(b_index+2,grad_value); // red channel = same as blue channel
//
//         }
//     }
//     return output;
// }

// Image nonMaximumSuppression(const Image& gradMag, const Image& im_derivX, const Image& im_derivY){
//     Image output = gradMag;
//     const int channels = gradMag.getLayers();
//     const int height = gradMag.getHeight();
//     const int width = gradMag.getWidth();
//
//     for (int y = 0; y < height; ++y){
//         for (int x = 0; x < width; ++x){
//             int index = (channels*(y * width+ x)+0);
//             // Formula for direction: direction = arctan(derivY/derivX)
//             double direction = std::atan(im_derivY.getData()[index]/im_derivX.getData()[index]);
//             direction = direction * 180/pi; // Convert direction to degrees
//             if (direction < 0){ // if direction is negative degrees
//                 direction = 360 + direction; // find positive equivalent
//             }
//
//             // Rounding the direction to be 0, 45, 90, 135, 180, 225, 270 or 315:
//             // Creating an array with absolute difference between the direction and the given angles:
//             double difference[8] { abs(direction - 0), abs(direction - 45), abs(direction - 90),
//                                    abs(direction - 135), abs(direction - 180), abs(direction - 225),
//                                    abs(direction - 270), abs(direction - 315) };
//             // The smallest difference then gives which angle is closest to the direction, which will be used onwards.
//             // Here is a list of which array indices correspond with which angle:
//             // index 0  =   angle 0
//             // index 1  =   angle 45
//             // index 2  =   angle 90
//             // index 3  =   angle 135
//             // index 4  =   angle 180
//             // index 5  =   angle 225
//             // index 6  =   angle 270
//             // index 7  =   angle 315
//             // Pre-determined angles:
//             int angles[8] { 0, 45, 90, 135, 180, 225, 270, 315};
//             // Finding the index of the smallest difference:
//             int min_element_index = std::distance(&difference[0], std::min_element(difference,difference+8));
//             // Set the direction as one of the pre-determined angles:
//             direction = angles[min_element_index];
//
//             // Horizontal direction:
//             if (direction == 0 || direction == 180){
//                 // Look at pixels to the left and right:
//                 if (x-1 >= 0){
//                     // If the pixel to the left has a larger value, suppress the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*(y * width+ x-1)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//                 if (x+1 < width){
//                     // If the pixel to the right has a larger value, suppress the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*(y * width+ x+1)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//             }
//
//             // Diagonal direction towards up-left
//             if (direction == 45 || direction == 225){
//                 // If the pixel diagonally up-left exists:
//                 if (x-1 >=0 && y-1 >= 0){
//                     // If the pixel diagonally up-left has a larger value, suppress the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*((y-1) * width+ x-1)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//                 // If the pixel diagonally down-right exists:
//                 if (x+1 < width && y+1 < height ){
//                     // If the pixel diagonally down-right has a larger value, suppress the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*((y+1) * width+ x+1)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//             }
//
//             // Vertical direction:
//             if (direction == 90 || direction == 270){
//                 // If the pixel above exists:
//                 if (y-1 >= 0){
//                     // If the pixel above has a larger value, suppresss the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*((y-1) * width+ x)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//                 // If the pixel below exists:
//                 if (y+1 < height){
//                     // If the pixel below has a larger value, suppress the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*((y+1) * width+ x)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//             }
//
//             // Diagonal direction towards up-right:
//             if (direction == 135 || direction == 315){
//                 // If the pixel diagonally up-right exists:
//                 if (x+1 < width && y-1 >= 0){
//                     // If the pixel diagonally up-right has a larger value, suppress the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*((y-1) * width+ x+1)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//                 // If the pixel diagonally down-left exists:
//                 if (x-1 >= 0 && y+1 < height ){
//                     // If the pixel diagonally down-left has a larger value, suppress the value in the current pixel:
//                     if (gradMag.getData()[(channels*(y * width+ x)+0)] < gradMag.getData()[(channels*((y+1) * width+ x-1)+0)]){
//                         output.setData((channels*(y * width+ x)+0), 0); // B value
//                         output.setData((channels*(y * width+ x)+1), 0); // G value
//                         output.setData((channels*(y * width+ x)+2), 0); // R value
//                     }
//                 }
//             }
//         }
//     }
//     return output;
//
// }

// void hysteresisThresholding(Image &gradMag, double high_threshold, double low_threshold){
//     const int channels = gradMag.getLayers();
//     const int height = gradMag.getHeight();
//     const int width = gradMag.getWidth();
//     int count_edge = 0; // Counts the primary edge pixels (above high threshold), useful info for debugging
//     int count_edge_sec = 0; // Counts the secondary edge pixels (between low and high threshold)
//
//     // First loop, gather all edge pixels:
//     for (int y = 0; y < height; ++y) {
//         for (int x = 0; x < width; ++x) {
//             // If pixel value is above HIGH threshold, set its value to max (edge pixel)
//             if (gradMag.getData()[channels*(y * width+x)+0] >= high_threshold){
//                 gradMag.setData(channels*(y * width+x)+0, 255);
//                 gradMag.setData(channels*(y * width+x)+1, 255);
//                 gradMag.setData(channels*(y * width+x)+2, 255);
//                 count_edge++;
//
//                 // Then look at neighbour pixels, if those are above LOW threshold, set their values to max as well.
//                 // The neighbours of pixel 'X' are:
//                 //   ___________
//                 //  | 1 | 2 | 3 |
//                 //  | 4 | X | 5 |
//                 //  | 6 | 7 | 8 |
//                 //   ‾‾‾‾‾‾‾‾‾‾‾
//                 // The neighbouring pixels are checked in this order.
//                 // If 1, 2, 3 or 4 is added to the edge pixels, the loop will go back to that pixel before advancing.
//                 // This is to make sure no additional edge pixels are missed.
//
//                 // 1. Pixel above-left:
//                 if (x-1 >= 0){
//                     if (y-1 >= 0){
//                         if ((gradMag.getData()[channels*((y-1) * width+x-1)+0] >= low_threshold) &&
//                             (gradMag.getData()[channels*((y-1) * width+x-1)+0] < high_threshold) ){
//                             gradMag.setData(channels*((y-1) * width+x-1)+0, 255);
//                             gradMag.setData(channels*((y-1) * width+x-1)+1, 255);
//                             gradMag.setData(channels*((y-1) * width+x-1)+2, 255);
//                             count_edge_sec++;
//                             // The next pixel that will be looked at in the loop is (x-1, y-1):
//                             x = (x-1)-1;    // x is decreased by 2 because as soons as the loop ends, it will increase by 1
//                             y = y-1;
//                             continue;
//                         }
//                     }
//                 }
//
//                 // 2. Pixel above:
//                 if (y-1 >= 0){
//                     if ((gradMag.getData()[channels*((y-1) * width+x)+0] >= low_threshold) &&
//                         (gradMag.getData()[channels*((y-1) * width+x)+0] < high_threshold)){
//                         gradMag.setData(channels*((y-1) * width+x)+0, 255);
//                         gradMag.setData(channels*((y-1) * width+x)+1, 255);
//                         gradMag.setData(channels*((y-1) * width+x)+2, 255);
//                         count_edge_sec++;
//                         // The next pixel that will be looked at in the loop is (x, y-1):
//                         x = (x)-1;
//                         y = y-1;
//                         continue;
//                     }
//                 }
//
//                 // 3. Pixel above-right:
//                 if (x+1 < width){
//                     if (y-1 >= 0){
//                         if ((gradMag.getData()[channels*((y-1) * width+x+1)+0] >= low_threshold) &&
//                             (gradMag.getData()[channels*((y-1) * width+x+1)+0] < high_threshold)){
//                             gradMag.setData(channels*((y-1) * width+x+1)+0, 255);
//                             gradMag.setData(channels*((y-1) * width+x+1)+1, 255);
//                             gradMag.setData(channels*((y-1) * width+x+1)+2, 255);
//                             count_edge_sec++;
//                             // The next pixel that will be looked at in the loop is (x+1, y-1):
//                             x = (x+1)-1;
//                             y = y-1;
//                             continue;
//                         }
//                     }
//                 }
//
//                 // 4. Pixel to the left:
//                 if (x-1 >= 0){
//                     if ((gradMag.getData()[channels*(y * width+x-1)+0] >= low_threshold) &&
//                         (gradMag.getData()[channels*(y * width+x-1)+0] < high_threshold)){
//                         gradMag.setData(channels*(y * width+x-1)+0, 255);
//                         gradMag.setData(channels*(y * width+x-1)+1, 255);
//                         gradMag.setData(channels*(y * width+x-1)+2, 255);
//                         count_edge_sec++;
//                         // The next pixel that will be looked at in the loop is (x-1, y):
//                         x = (x-1)-1;
//                         y = y;
//                         continue;
//                     }
//                 }
//
//                 // 5. Pixel to the right:
//                 if (x+1 < width){
//                     if (gradMag.getData()[channels*(y * width+x+1)+0] >= low_threshold){
//                         gradMag.setData(channels*(y * width+x+1)+0, 255);
//                         gradMag.setData(channels*(y * width+x+1)+1, 255);
//                         gradMag.setData(channels*(y * width+x+1)+2, 255);
//                         count_edge_sec++;
//                     }
//                 }
//
//                 // 6. Pixel below-left:
//                 if (x-1 >= 0){
//                     if (y+1 < height){
//                         if (gradMag.getData()[channels*((y+1) * width+x-1)+0] >= low_threshold){
//                             gradMag.setData(channels*((y+1) * width+x-1)+0, 255);
//                             gradMag.setData(channels*((y+1) * width+x-1)+1, 255);
//                             gradMag.setData(channels*((y+1) * width+x-1)+2, 255);
//                             count_edge_sec++;
//                         }
//                     }
//                 }
//
//                 // 7. Pixel below:
//                 if (y+1 < height){
//                     if (gradMag.getData()[channels*((y+1) * width+x)+0] >= low_threshold){
//                         gradMag.setData(channels*((y+1) * width+x)+0, 255);
//                         gradMag.setData(channels*((y+1) * width+x)+1, 255);
//                         gradMag.setData(channels*((y+1) * width+x)+2, 255);
//                         count_edge_sec++;
//                     }
//                 }
//
//                 // 8. Pixel below-right:
//                 if (x+1 < width){
//                     if (y+1 < height){
//                         if (gradMag.getData()[channels*((y+1) * width+x+1)+0] >= low_threshold){
//                             gradMag.setData(channels*((y+1) * width+x+1)+0, 255);
//                             gradMag.setData(channels*((y+1) * width+x+1)+1, 255);
//                             gradMag.setData(channels*((y+1) * width+x+1)+2, 255);
//                             count_edge_sec++;
//                         }
//                     }
//                 }
//             }
//             // If pixel value is below LOW threshold, discard it
//             else if (gradMag.getData()[channels*(y * width+ x)+0] < low_threshold){
//                 gradMag.setData(channels*(y * width+ x)+0, 0);
//                 gradMag.setData(channels*(y * width+ x)+1, 0);
//                 gradMag.setData(channels*(y * width+ x)+2, 0);
//             }
//         }
//     }
//     // Second loop, remove all pixels between low and high that are not connected:
//     for (int y = 0; y < height; ++y) {
//         for (int x = 0; x < width; ++x) {
//             if (gradMag.getData()[channels*(y * width+x)+0] < high_threshold){
//                 gradMag.setData(channels*(y * width+x)+0, 0);
//                 gradMag.setData(channels*(y * width+x)+1, 0);
//                 gradMag.setData(channels*(y * width+x)+2, 0);
//             }
//         }
//     }
//     //std::cout << "Primary edge pixels: " << count_edge << std::endl;
//     //std:: cout << "Secondary edge pixels: " << count_edge_sec << std::endl;
//     // Function is of type void so no return.
// }
