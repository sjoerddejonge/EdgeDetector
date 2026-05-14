// IMPORTANT: The code in this file is the only code in the project I did not write myself, though I made some minor
// changes.

/**
 * Created by Paul from solarionprogrammer.com
 * Very slightly modified by Sjoerd de Jonge on 01/07/2020.
 * This whole header file is a result from following a tutorial and is therefore not my own code.
 * I figured creating a bitmap image reader in C++ was somewhat of a project on its own and not
 * what I wanted to create for this project. Therefore I used the tutorial. I learned a lot from it
 * regardless, added comments and made some minor changes.
 * Tutorial taken from: https://solarianprogrammer.com/2018/11/19/cpp-reading-writing-bmp-images/
 * Wikipedia page on bitmaps: https://en.wikipedia.org/wiki/BMP_file_format
 */

#pragma once
#include <fstream>
#include <utility>
#include <vector>
#include <iostream>

#ifndef EDGEDETECTOR_BMP_H
#define EDGEDETECTOR_BMP_H

// Data structure for the BMP header.
// More about the BMP file format: https://en.wikipedia.org/wiki/BMP_file_format
#pragma pack(push, 1)
struct BMP_FileHeader{
    uint16_t signature{0x4D42};             // BMP file signature (file type) is always 'BM' (0x4D42)
    uint32_t file_size{0};                  // Size of the file (in bytes)
    uint16_t reserved1{0};                  // Reserved, see image on the Wikipedia page linked above
    uint16_t reserved2{0};                  // Also reserved
    uint32_t pixel_offset{0};               // Start position of pixel data (bytes from the beginning of the file)
};

// Data structure for the DIB header. Again referring to the Wikipedia article,
// most importantly this image: https://upload.wikimedia.org/wikipedia/commons/c/c4/BMPfileFormat.png
// The tutorial splits this struct in two different headers, I chose to follow the naming in the Wikipedia
// article and named the first DIBHeader and the second ColorHeader.
struct BMP_DIBHeader {
    // General information (40 bytes):
    uint32_t size{0};                    // DIB header size
    int32_t width{0};                    // Image width in pixels    (positive means origin is in lower left corner)
    int32_t height{0};                   // Image height in pixels   (negative means origin is in upper left corner)
    uint16_t planes{1};                  // This is always 1 (number of planes)
    uint16_t bit_count{0};               // Number of bits per pixel
    uint32_t compression{0};             // 0 or 3 - uncompressed. Compressed BMP images are not supported.
    uint32_t size_image{0};              // 0 for uncompressed images
    int32_t x_pixels_per_meter{0};       // Pixels per meter on the x axis
    int32_t y_pixels_per_meter{0};       // Pixels per meter on the y axis
    uint32_t colors_used{0};             // Number of color indexes in the color table. 0 is for maximum number colors
    uint32_t colors_important{0};        // Number of colors used for displaying the BMP. If 0, all colors are required
};
struct BMP_ColorHeader{
    // Color information for images with transparency (84 bytes):
    uint32_t red_mask { 0x00ff0000 };       // Bit mask for the red channel
    uint32_t green_mask { 0x0000ff00 };     // Bit mask for the green channel
    uint32_t blue_mask { 0x000000ff };      // Bit mask for the blue channel
    uint32_t alpha_mask { 0xff000000 };     // Bit mask for the alpha channel
    uint32_t color_space_type { 0x73524742 };   // Default "sRGB" (0x73524742)
    uint32_t unused[16] { 0 };              // Unused data for sRGB color space, not supported by this bmp reader.
    // 32 bit integers have a size of 4 bytes, 16-bit integers have a size of 2 bytes. The '_t' suffix guarantees
    // that it is always exactly the size as declared (int32_t is always 32 bits).
    // The size of this BMP_DIBHeader is therefore 124 bytes (considering the last variable is an array of size 16)
};
#pragma pack(pop)

struct BMP {
    BMP_FileHeader file_header;
    BMP_DIBHeader dib_header;
    BMP_ColorHeader color_header;
    std::vector<uint8_t> data; // The pixel data of the image

    explicit BMP(const std::string& filename){
        //std::cout << "Default size of file header: " << sizeof(BMP_FileHeader) << std::endl;
        //std::cout << "Default size of DIB header: " << sizeof(BMP_DIBHeader) << std::endl;
        //std::cout << "Default size of color header: " << sizeof(BMP_ColorHeader) << std::endl;
        read(filename);
        // std::cout << "Image read successfully. Image size is " << dib_header.width << "x" << dib_header.height << " pixels. ";
        // std::cout << "Bit count is: " << dib_header.bit_count << " bit." << std::endl;
    }

    BMP(const int32_t width, const int32_t height, const bool has_alpha = true){
        if (width <= 0 || height <= 0){
            throw std::runtime_error("The image width and height must be positive numbers.");
        }

        dib_header.width = width;
        dib_header.height = height;
        // 32 Bit bmp with alpha values:
        if (has_alpha){
            dib_header.size = sizeof(BMP_DIBHeader) + sizeof(BMP_ColorHeader);
            file_header.pixel_offset = sizeof(BMP_FileHeader) + sizeof(BMP_DIBHeader) + sizeof(BMP_ColorHeader);

            dib_header.bit_count = 32;
            dib_header.compression = 3;
            row_stride = width * 4;
            data.resize(row_stride * height);
            file_header.file_size = file_header.pixel_offset + data.size();
        }
        // 24 Bit bmp without alpha values:
        else{
            dib_header.size = sizeof(BMP_DIBHeader);
            file_header.pixel_offset = sizeof(BMP_FileHeader) + sizeof(BMP_DIBHeader);

            dib_header.bit_count = 24;
            dib_header.compression = 0;
            row_stride = width * 3;
            data.resize(row_stride * height);

            const uint32_t new_stride = make_stride_aligned(4);
            file_header.file_size = file_header.pixel_offset + data.size() + dib_header.height * (new_stride - row_stride);
        }
    }

    void read(const std::string& filename){
        std::ifstream inp{ filename, std::ios_base::binary }; // Create an if-stream to open the BMP.
        if (inp) {
            // Read and store an amount of characters in the file_header string equal to the size of file_header.
            inp.read(reinterpret_cast<char *>(&file_header), sizeof(file_header));
            // Check if the signature (file format) matches the BMP signature.
            if(file_header.signature != 0x4D42){
                throw std::runtime_error("Error! Unrecognized file format.");
            }
            // Read and store the DIB header of the bitmap.
            inp.read(reinterpret_cast<char *>(&dib_header), sizeof(dib_header));

            // The BMPColorHeader is used only for transparent images
            if (dib_header.bit_count == 32){
                // Check if the file has bit mask color information
                if(dib_header.size >= (sizeof(BMP_DIBHeader) + sizeof(BMP_ColorHeader))){
                    inp.read(reinterpret_cast<char *>(&color_header), sizeof(BMP_ColorHeader));
                    // Check if the pixel data is stored as BGRA and if the color space type is sRGB
                    check_color_header(color_header);
                }
                else {
                    std::cerr << "Warning! The file \"" << filename << "\" does not seem to contain bit mask information\n";
                    throw std::runtime_error("Error! Unrecognized file format.");
                }
            }

            // Jump to the pixel data location
            inp.seekg(file_header.pixel_offset, std::ifstream::beg);

            // Adjust the header fields for output.
            // Some editors will put extra info in the image file, we only save the headers and the data.
            if(dib_header.bit_count == 32){
                dib_header.size = sizeof(BMP_DIBHeader) + sizeof(BMP_ColorHeader);
                file_header.pixel_offset = sizeof(BMP_FileHeader) + sizeof(BMP_DIBHeader) + sizeof(BMP_ColorHeader);
            }
            else {
                dib_header.size = sizeof(BMP_DIBHeader);
                // Update the pixel offset to match new DIB header size:
                file_header.pixel_offset = sizeof(BMP_FileHeader) + sizeof(BMP_DIBHeader);
            }
            file_header.file_size = file_header.pixel_offset; // file_size will be increased later to match actual size.
            // Here the size of the pixeldata is still excluded.
            // Check if origin of the image is in bottom left corner:
            if (dib_header.height < 0){
                throw std::runtime_error("The program can treat only BMP images with the origin in the bottom"
                                         "left corner!");
            }

            // The size of data (which stores pixel data) is resized to match the image.
            data.resize(dib_header.width * dib_header.height * dib_header.bit_count / 8);

            // Here we check if we need to take into account row padding
            // "The BMP image format expects every row of data to be aligned to a four bytes boundary or padded with
            // zeros if this is not the case." In case of a 32 bit per pixel bmp, row padding is not needed.
            if (dib_header.width % 4 == 0){
                // No row padding, read the BMP pixel data directly into our std::vector data.
                inp.read((char*)data.data(), data.size());
                file_header.file_size += static_cast<uint32_t>(data.size()); // file_header.file_size is a uint32_t,
                // but data is a uint8_t vector. Therefore the static_cast converts to
                // a uint32_t type.
            }
            else {
                // With row padding (in case of a 32 bit bmp no row padding is added):
                row_stride = dib_header.width * dib_header.bit_count / 8; // row_stride is the size of a pixel row in
                // bytes
                uint32_t new_stride = make_stride_aligned(4);
                std::vector<uint8_t> padding_row(new_stride - row_stride); // no. of 0s to be added to a row for
                // padding, this becomes 0 for 32 bit bmp.
                // For each row, read the BMP pixel data into our std::vector data and read the row padding data into
                // our std::vector row_padding. When manipulating the image, we don't want to mess with 0s.
                for (int y = 0; y < dib_header.height; ++y){
                    inp.read((char*)(data.data() + row_stride * y), row_stride);
                    inp.read((char*)padding_row.data(), padding_row.size());
                    // The 0s in the padding_row are being overwritten each loop, the size of padding_row is still equal
                    // to the amount of 0s in a single row.
                }
                file_header.file_size += static_cast<uint32_t>(data.size()) + dib_header.height * static_cast<uint32_t>(padding_row.size());
            }
        }
        else {
            throw std::runtime_error("Unable to open the input image file.");
        }
    }

    void write(const std::string& filename){
        std::ofstream of{ filename, std::ios_base::binary };
        if (of) {
            if (dib_header.bit_count == 32) {
                // If bmp is 32 bit per pixel, no padding is needed.
                write_headers_and_data(of);
            }
            else if (dib_header.bit_count == 24){
                if (dib_header.width % 4 == 0){
                    // If the width is divisible by 4 no padding is needed
                    write_headers_and_data(of);
                }
                else {
                    uint32_t new_stride = make_stride_aligned(4);
                    std::vector<uint8_t> padding_row(new_stride - row_stride);

                    write_headers(of);

                    for (int y = 0; y < dib_header.height; ++y) {
                        // Write data first to the stream, row_stride is the lenght of a single pixel row (in bytes):
                        of.write((const char*)(data.data() + row_stride * y), row_stride);
                        // Then write the padding_row data (0s at the end of a row) to the stream:
                        of.write((const char*)padding_row.data(), padding_row.size());
                    }
                }
            }
            else {
                throw std::runtime_error("The program can treat only 24 or 32 bits per pixel");
            }
        }
        else {
            throw std::runtime_error("Unable to open the output image file.");
        }
    }

private:
    uint32_t row_stride{ 0 };

    void write_headers(std::ofstream &of) {
        of.write((const char*) &file_header, sizeof(file_header)); // Write the file header
        of.write((const char*) &dib_header, sizeof(dib_header)); // Write the DIB header
        if(dib_header.bit_count == 32){ // In case of 32 bit image, write the color header too.
            of.write((const char*) &color_header, sizeof(color_header));
        }
    }

    void write_headers_and_data(std::ofstream &of) {
        write_headers(of); // Write the headers.
        of.write((const char*) data.data(), data.size()); // Write the pixel data, the size of data is
        // made equal to the image size during the read() function.
    }

    // Add 1 to the row_stride until it is divisble with align_stride, in this case that is divisible by 4.
    uint32_t make_stride_aligned(uint32_t align_stride){
        uint32_t new_stride = row_stride; // In case of a 32 bit image, row_stride is already divisble by 4.
        while (new_stride % align_stride != 0) {
            new_stride++;
        }
        return new_stride;
    }

    // Check if the pixel data is stored as BGRA and if the color space type is sRGB
    void check_color_header(BMP_ColorHeader &color_header){
        BMP_ColorHeader expected_color_header;
        if(expected_color_header.red_mask != color_header.red_mask ||
           expected_color_header.blue_mask != color_header.blue_mask ||
           expected_color_header.green_mask != color_header.green_mask ||
           expected_color_header.alpha_mask != color_header.alpha_mask){
            throw std::runtime_error("Unexpected color mask format! The program expects the pixel data to be in the BGRA format");
        }
        if(expected_color_header.color_space_type != color_header.color_space_type){
            throw std::runtime_error("Unexpected color space type! The program expects sRGB values");
        }
    }
};

#endif //EDGEDETECTOR_BMP_H