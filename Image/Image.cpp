#include <fstream>
#include "Image.hpp"
#include "PixelConverter.hpp"

using namespace std;

Image::Image(const Matrix<RGBPixel, Dynamic, Dynamic>& image) {
    rgbImage = image;
    rgbLoaded = true;
}

Image::Image(const Matrix<YCbCrPixel, Dynamic, Dynamic>& image) {
    yCbCrImage = image;
    yCbCrLoaded = true;
}

Image::Image(const std::string& filename) {
    ifstream imageFile(filename);

    string line;
    getline(imageFile, line); // P3 or P6
    getline(imageFile, line); // comment
    getline(imageFile, line); // width height

    int width = stoi(line.substr(0, 3));
    int height = stoi(line.substr(3, 6));

    rgbImage = *(new Matrix<RGBPixel, Dynamic, Dynamic>());
    rgbImage.resize(height, width);

    getline(imageFile, line);
    int maxVal = stoi(line);

    int i=0;
    RGBPixel pixel{0, 0, 0};

    while(getline(imageFile, line)) {
        pixel.red = static_cast<unsigned char>(stoi(line));

        getline(imageFile, line);
        pixel.green = static_cast<unsigned char>(stoi(line));

        getline(imageFile, line);
        pixel.blue = static_cast<unsigned char>(stoi(line));


        rgbImage(i / width, i % width) = pixel;
        i++;
    }
    imageFile.close();
}
/*
Image::Image(Matrix<YCbCrPixel, Dynamic, Dynamic> image) {

}
*/
Image::~Image() {
    delete &rgbImage;
    //delete &yCbCrImage;
}

Image::Image() {

}

Matrix<YCbCrPixel, Dynamic, Dynamic> Image::getYCbCrImage() {
    // check if already converted
    if(yCbCrLoaded) {
        // not initialized
        yCbCrImage.resize(rgbImage.rows(), rgbImage.cols());

        for(int i=0; i<rgbImage.rows(); i++) {
            for(int j=0; j<rgbImage.cols(); j++) {
                auto yCbCrPixel = PixelConverter::RGBToYCbCr(rgbImage(i, j));
                yCbCrImage(i, j) = yCbCrPixel;
            }
        }
    }

    return yCbCrImage;
}


Matrix<RGBPixel, Dynamic, Dynamic> Image::getRGBImage() {
    // check if already converted
    if(rgbLoaded) {
        // not initialized
        rgbImage.resize(yCbCrImage.rows(), yCbCrImage.cols());

        for(int i=0; i<yCbCrImage.rows(); i++) {
            for(int j=0; j<yCbCrImage.cols(); j++) {
                auto rgbPixel = PixelConverter::YCbCrToRGB(yCbCrImage(i, j));
                rgbImage(i, j) = rgbPixel;
            }
        }
    }

    return rgbImage;
}