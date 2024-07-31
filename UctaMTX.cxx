#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

// Function to convert MTX file to JPEG
bool convertMTXtoJPEG(const std::string& mtxFilePath, const std::string& jpegFilePath) {
    // Read MTX file
    std::ifstream mtxFile(mtxFilePath, std::ios::binary);
    if (!mtxFile) {
        std::cerr << "Failed to open MTX file: " << mtxFilePath << std::endl;
        return false;
    }

    // Read the MTX file into a vector
    std::vector<uint8_t> mtxData((std::istreambuf_iterator<char>(mtxFile)), std::istreambuf_iterator<char>());

    // Check if the file contains JPEG header
    if (mtxData.size() < 2 || mtxData[0] != 0xFF || mtxData[1] != 0xD8) {
        std::cerr << "Invalid MTX file. No JPEG data found." << std::endl;
        return false;
    }

    // Write the extracted JPEG data to a JPEG file
    std::ofstream jpegFile(jpegFilePath, std::ios::binary);
    if (!jpegFile) {
        std::cerr << "Failed to create JPEG file: " << jpegFilePath << std::endl;
        return false;
    }
    jpegFile.write(reinterpret_cast<const char*>(mtxData.data()), mtxData.size());

    std::cout << "MTX file converted to JPEG successfully :)" << std::endl;
    return true;
}

// Function to convert JPEG to MTX file
bool convertJPEGtoMTX(const std::string& jpegFilePath, const std::string& mtxFilePath) {
    // Read JPEG file
    std::ifstream jpegFile(jpegFilePath, std::ios::binary);
    if (!jpegFile) {
        std::cerr << "Failed to open JPEG file: " << jpegFilePath << std::endl;
        return false;
    }

    // Read the JPEG file into a vector
    std::vector<uint8_t> jpegData((std::istreambuf_iterator<char>(jpegFile)), std::istreambuf_iterator<char>());

    // Create the MTX header
    std::vector<uint8_t> mtxHeader = { 'J', 'F', 'I', 'F' };

    // Concatenate the MTX header and JPEG data
    std::vector<uint8_t> mtxData;
    mtxData.reserve(mtxHeader.size() + jpegData.size());
    mtxData.insert(mtxData.end(), mtxHeader.begin(), mtxHeader.end());
    mtxData.insert(mtxData.end(), jpegData.begin(), jpegData.end());

    // Write the MTX data to a file
    std::ofstream mtxFile(mtxFilePath, std::ios::binary);
    if (!mtxFile) {
        std::cerr << "Failed to create MTX file: " << mtxFilePath << std::endl;
        return false;
    }
    mtxFile.write(reinterpret_cast<const char*>(mtxData.data()), mtxData.size());

    std::cout << "JPEG file converted to MTX successfully :)" << std::endl;
    return true;
}

int main() {
    std::string mtxFilePath = "input.mtx";
    std::string jpegFilePath = "output.jpeg";

    // Convert MTX to JPEG
    if (convertMTXtoJPEG(mtxFilePath, jpegFilePath)) {
        std::cout << "MTX to JPEG conversion successful :3" << std::endl;
    } else {
        std::cerr << "MTX to JPEG conversion failed :(" << std::endl;
    }

    // Convert JPEG to MTX
    if (convertJPEGtoMTX(jpegFilePath, mtxFilePath)) {
        std::cout << "JPEG to MTX conversion successful :3" << std::endl;
    } else {
        std::cerr << "JPEG to MTX conversion failed :(" << std::endl;
    }

    return 0;
}