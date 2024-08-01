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
void writeMTX(const std::string &outputFile, const std::vector<uint8_t> &firstImage, const std::vector<uint8_t> &secondImage, int width1, int height1, int channels1, int width2, int height2, int channels2) {
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

    // Write dimensions and channels
    file.write(reinterpret_cast<char*>(&width1), sizeof(width1));
    file.write(reinterpret_cast<char*>(&height1), sizeof(height1));
    file.write(reinterpret_cast<char*>(&channels1), sizeof(channels1));
    file.write(reinterpret_cast<char*>(&width2), sizeof(width2));
    file.write(reinterpret_cast<char*>(&height2), sizeof(height2));
    file.write(reinterpret_cast<char*>(&channels2), sizeof(channels2));

    // Write images
    if (lengthFirst > 0) {
        file.write(reinterpret_cast<const char*>(firstImage.data()), firstImage.size());
    }
    file.write(reinterpret_cast<const char*>(secondImage.data()), secondImage.size());
}

// Function to read MTXv0 file
void readMTX(const std::string &inputFile, std::vector<uint8_t> &firstImage, std::vector<uint8_t> &secondImage, int &width1, int &height1, int &channels1, int &width2, int &height2, int &channels2) {
    std::ifstream file(inputFile, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file " + inputFile);
    }

    uint32_t magic = 0;
    uint32_t lengthFirst, lengthSecond;

    // Read header
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&lengthFirst), sizeof(lengthFirst));
    file.read(reinterpret_cast<char*>(&lengthSecond), sizeof(lengthSecond));

    // Read dimensions and channels
    file.read(reinterpret_cast<char*>(&width1), sizeof(width1));
    file.read(reinterpret_cast<char*>(&height1), sizeof(height1));
    file.read(reinterpret_cast<char*>(&channels1), sizeof(channels1));
    file.read(reinterpret_cast<char*>(&width2), sizeof(width2));
    file.read(reinterpret_cast<char*>(&height2), sizeof(height2));
    file.read(reinterpret_cast<char*>(&channels2), sizeof(channels2));

    // Read images
    firstImage.resize(lengthFirst);
    secondImage.resize(lengthSecond);

    if (lengthFirst > 0) {
        file.read(reinterpret_cast<char*>(firstImage.data()), lengthFirst);
    }
    file.read(reinterpret_cast<char*>(secondImage.data()), lengthSecond);
}

// Function to write a JPEG file using libjpeg
void writeJPEG(const std::string &filename, const std::vector<uint8_t> &imageData, int width, int height, int channels) {
    FILE *file = fopen(filename.c_str(), "wb");
    if (!file) {
        throw std::runtime_error("Could not open file " + filename);
    }

    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, file);

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
    fclose(file);
}

int main() {
    try {
        std::string firstImageFile = "1.jpg";
        std::string secondImageFile = "2.jpg";
        std::string outputFile = "output.mtx";
        std::string mtxFile = "output.mtx";
        std::string firstOutputImageFile = "output1.jpg";
        std::string secondOutputImageFile = "output2.jpg";

        // Read JPEG files
        int width1, height1, channels1;
        std::vector<uint8_t> firstImageRaw = readJPEG(firstImageFile, width1, height1, channels1);
        std::vector<uint8_t> firstImageCompressed = compressJPEG(firstImageRaw, width1, height1, channels1);

        int width2, height2, channels2;
        std::vector<uint8_t> secondImageRaw = readJPEG(secondImageFile, width2, height2, channels2);
        std::vector<uint8_t> secondImageCompressed = compressJPEG(secondImageRaw, width2, height2, channels2);

        // Write MTXv0 file
        writeMTX(outputFile, firstImageCompressed, secondImageCompressed, width1, height1, channels1, width2, height2, channels2);
        std::cout << "MTXv0 file created successfully :3 " << outputFile << std::endl;

        // Read MTXv0 file
        std::vector<uint8_t> firstImageExtracted, secondImageExtracted;
        readMTX(mtxFile, firstImageExtracted, secondImageExtracted, width1, height1, channels1, width2, height2, channels2);

        // Write extracted images to JPEG files
        writeJPEG(firstOutputImageFile, firstImageExtracted, width1, height1, channels1);
        writeJPEG(secondOutputImageFile, secondImageExtracted, width2, height2, channels2);

        std::cout << "JPEG files created successfully :3 " << firstOutputImageFile << ", " << secondOutputImageFile << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}