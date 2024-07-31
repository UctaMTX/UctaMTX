#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <jpeglib.h>

// Function to read a JPEG file into a vector of bytes using libjpeg
std::vector<uint8_t> readJPEG(const std::string &filename, int &width, int &height, int &channels) {
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Could not open file " + filename);
    }

    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    width = cinfo.output_width;
    height = cinfo.output_height;
    channels = cinfo.output_components;
    std::vector<uint8_t> buffer(width * height * channels);

    while (cinfo.output_scanline < cinfo.output_height) {
        uint8_t *row_pointer = &buffer[cinfo.output_scanline * width * channels];
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(file);

    return buffer;
}

// Function to compress image data back to JPEG format using libjpeg
std::vector<uint8_t> compressJPEG(const std::vector<uint8_t> &imageData, int width, int height, int channels) {
    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    unsigned long outputSize = 0;
    uint8_t *outputBuffer = nullptr;
    jpeg_mem_dest(&cinfo, &outputBuffer, &outputSize);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = channels;
    cinfo.in_color_space = channels == 3 ? JCS_RGB : JCS_GRAYSCALE;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 75, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        const uint8_t *row_pointer = &imageData[cinfo.next_scanline * width * channels];
        jpeg_write_scanlines(&cinfo, const_cast<JSAMPARRAY>(&row_pointer), 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    std::vector<uint8_t> compressedData(outputBuffer, outputBuffer + outputSize);
    free(outputBuffer);

    return compressedData;
}

// Function to write MTXv0 file
void writeMTX(const std::string &outputFile, const std::vector<uint8_t> &firstImage, const std::vector<uint8_t> &secondImage) {
    std::ofstream file(outputFile, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file " + outputFile);
    }

    uint32_t magic = 0;
    uint32_t lengthFirst = firstImage.size();
    uint32_t lengthSecond = secondImage.size();

    // Write header
    file.write(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<char*>(&lengthFirst), sizeof(lengthFirst));
    file.write(reinterpret_cast<char*>(&lengthSecond), sizeof(lengthSecond));

    // Write images
    if (lengthFirst > 0) {
        file.write(reinterpret_cast<const char*>(firstImage.data()), firstImage.size());
    }
    file.write(reinterpret_cast<const char*>(secondImage.data()), secondImage.size());
}

int main() {
    try {
        std::string firstImageFile = "1.jpg";
        std::string secondImageFile = "2.jpg";
        std::string outputFile = "output.mtx";

        // Read JPEG files
        int width1, height1, channels1;
        std::vector<uint8_t> firstImageRaw = readJPEG(firstImageFile, width1, height1, channels1);
        std::vector<uint8_t> firstImageCompressed = compressJPEG(firstImageRaw, width1, height1, channels1);

        int width2, height2, channels2;
        std::vector<uint8_t> secondImageRaw = readJPEG(secondImageFile, width2, height2, channels2);
        std::vector<uint8_t> secondImageCompressed = compressJPEG(secondImageRaw, width2, height2, channels2);

        // Write MTXv0 file
        writeMTX(outputFile, firstImageCompressed, secondImageCompressed);

        std::cout << "MTXv0 file created successfully :3 " << outputFile << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}