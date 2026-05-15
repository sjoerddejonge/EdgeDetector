
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
void writeBMP(Matrix<double> input, const std::string& filename_write); // Write Matrix to .bmp image file

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
        writeBMP(deriv_x, filename_write); // Write derivX to .bmp

        /// Blur image with Gaussian derivative of Y (fY):
        Matrix<double> deriv_y = gaussianBlur(input_matrix, sigma, 2);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_fY.bmp"; // Add suffix to the name to indicate it is the derivative of Y
        writeBMP(deriv_y, filename_write); // Write derivY to .bmp

        /// Calculate gradient magnitude:
        std::cout << "Calculating gradient magnitude...\n";
        Matrix<double> grad_mag = gradientMagnitude(deriv_x, deriv_y);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_GradientMagnitude.bmp";
        writeBMP(grad_mag, filename_write);

        /// Non maximum suppression:
        std::cout << "Performing non-maximum suppression..." << std::endl;
        grad_mag = nonMaximumSuppression(grad_mag, deriv_x, deriv_y);
        /// Write result to image:
        filename_write = filename;
        filename_write += "_GradientMagnitudeSuppressed.bmp";
        writeBMP(grad_mag, filename_write);

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
        writeBMP(edge_map, filename_write);

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
void writeBMP(Matrix<double> input, const std::string& filename_write){
    input.matrixToImage(); // Transform the data to be into the 0 - 255 range.
    Image image = Image(input);
    image.reformatOrigin(true);
    // Create a new vector<uint8_t> to store the image data as integers (which is needed for bmp):
    const std::vector<uint8_t> image_data(image.getData().begin(), image.getData().end());
    const int32_t width = image.getWidth();
    const int32_t height = image.getHeight();
    bool has_alpha = true;
    if (image.getLayers() <= 3){
        has_alpha = false;
    }
    BMP imageBMP(width,height,has_alpha);       // Create new BMP similar to image size.
    imageBMP.data = image_data;                 // Set image matrix pixel data as bmp pixel data.
    imageBMP.write(filename_write);
}
