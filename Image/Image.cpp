#include <fstream>
#include "Image.hpp"
#include "PixelConverter.hpp"

using namespace std;
/**
 * Create an Image from the give RGB image
 * @param image
 */
Image::Image(const Matrix<RGBPixel, Dynamic, Dynamic>& image) {
    rgbImage = image;
    rgbLoaded = true;
}

/**
 * Create an Image from the give YCbCr image
 * @param image
 */
Image::Image(const Matrix<YCbCrPixel, Dynamic, Dynamic>& image) {
    yCbCrImage = image;
    yCbCrLoaded = true;
}

/**
 * Create an Image from a file
 * @param filename The filename to read
 */
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

    rgbLoaded = true;
    imageFile.close();
}

Matrix<YCbCrPixel, Dynamic, Dynamic> Image::getYCbCrImage() {
    // check if already converted
    if(not yCbCrLoaded) {
        // not initialized
        yCbCrImage.resize(rgbImage.rows(), rgbImage.cols());

        for(int i=0; i<rgbImage.rows(); i++) {
            for(int j=0; j<rgbImage.cols(); j++) {
                auto yCbCrPixel = PixelConverter::RGBToYCbCr(rgbImage(i, j));
                yCbCrImage(i, j) = yCbCrPixel;
            }
        }
    }

    yCbCrLoaded = true;
    return yCbCrImage;
}


Matrix<RGBPixel, Dynamic, Dynamic> Image::getRGBImage() {
    // check if already converted
    if(not rgbLoaded) {
        // not initialized
        rgbImage.resize(yCbCrImage.rows(), yCbCrImage.cols());

        for(int i=0; i<yCbCrImage.rows(); i++) {
            for(int j=0; j<yCbCrImage.cols(); j++) {
                auto rgbPixel = PixelConverter::YCbCrToRGB(yCbCrImage(i, j));
                rgbImage(i, j) = rgbPixel;
            }
        }
    }
    rgbLoaded = true;
    return rgbImage;
}

void Image::write(const std::string& file) {
    ofstream f(file);

    f<<"P3"<<endl;
    f<<"# CREATOR: GIMP PNM Filter Version 1.1"<<endl;
    f<<"800 600"<<endl<<"255"<<endl;
    unsigned char red, green, blue;
    getRGBImage();


    for(int i=0; i<rgbImage.rows(); i++) {
        for(int j=0; j<rgbImage.cols(); j++) {
            red = rgbImage(i, j).red;
            green = rgbImage(i, j).green;
            blue = rgbImage(i, j).blue;

            f<<to_string(red)<<endl<<to_string(green)<<endl<<to_string(blue)<<endl;
        }
    }
    f.close();
}

Image::~Image() {
    //delete &rgbImage;
    //delete &yCbCrImage;
}
