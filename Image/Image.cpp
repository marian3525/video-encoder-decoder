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

    encodeInit();
}

/**
 * Create an Image from the give YCbCr image
 * @param image
 */
Image::Image(const Matrix<YCbCrPixel, Dynamic, Dynamic>& image) {
    yCbCrImage = image;
    yCbCrLoaded = true;

    encodeInit();
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

    width = stoi(line.substr(0, 3));
    height = stoi(line.substr(3, 6));

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

    encodeInit();
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

int Image::getWidth() const {
    if(rgbLoaded) {
        return rgbImage.cols();
    }
    else {
        return yCbCrImage.cols();
    }
}

int Image::getHeight() const {
    if(rgbLoaded) {
        return rgbImage.rows();
    }
    else {
        return yCbCrImage.rows();
    }
}

std::tuple<std::vector<Block>, std::vector<Block>, std::vector<Block>> Image::encode() {
    auto yBlocks = encodeYComponent();
    auto uBlocks = encodeUComponent();
    auto vBlocks = encodeVComponent();

    return make_tuple(yBlocks, uBlocks, vBlocks);
}

vector<Block> Image::encodeYComponent() {
    vector<Block> blocks;
    Matrix<uint8_t, Dynamic, Dynamic> tmpVals;
    tmpVals.resize(8,8);

    for_each(blockLocations.begin(), blockLocations.end(), [&](const std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>>& corners){
        int row=0, col=0;
        for(int i=get<0>(corners).first; i<get<2>(corners).first; i++) {
            col=0;

            for(int j=get<0>(corners).second; j<get<0>(corners).second; j++) {
                tmpVals(row, col++) = yCbCrImage(i, j).Y;
            }
            row++;
        }

        blocks.emplace_back(tmpVals, Y, corners);
    });

    return blocks;
}

std::vector<Block> Image::encodeUComponent() {
    vector<Block> blocks;
    Matrix<uint8_t, Dynamic, Dynamic> tmpVals;
    tmpVals.resize(4,4);

    for_each(blockLocations.begin(), blockLocations.end(), [&](const std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>>& corners){
        int row=0, col=0;
        // step 2 for i, j.
        // Take the average in each 2x2 sub-block
        for(int i=get<0>(corners).first; i<get<2>(corners).first-2; i+=2) {
            col=0;

            for(int j=get<0>(corners).second; j<get<0>(corners).second-2; j+=2) {
                // average the 4 values
                int sum = yCbCrImage(i, j).Cr       + yCbCrImage(i, j+1).Cr
                        + yCbCrImage(i+1, j).Cr + yCbCrImage(i+1, j+1).Cr;
                tmpVals(row, col++) = sum/4;
            }
            row++;
        }

        blocks.emplace_back(tmpVals, V, corners);
    });

    return blocks;
}

std::vector<Block> Image::encodeVComponent() {
    vector<Block> blocks;
    Matrix<uint8_t, Dynamic, Dynamic> tmpVals;
    tmpVals.resize(4,4);

    for_each(blockLocations.begin(), blockLocations.end(), [&](const std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>>& corners){
        int row=0, col=0;
        // step 2 for i, j.
        // Take the average in each 2x2 sub-block
        for(int i=get<0>(corners).first; i<get<2>(corners).first-2; i+=2) {
            col=0;

            for(int j=get<0>(corners).second; j<get<0>(corners).second-2; j+=2) {
                // average the 4 values
                int sum = yCbCrImage(i, j).Cb       + yCbCrImage(i, j+1).Cb
                          + yCbCrImage(i+1, j).Cb + yCbCrImage(i+1, j+1).Cb;
                tmpVals(row, col++) = sum/4;
            }
            row++;
        }

        blocks.emplace_back(tmpVals, U, corners);
    });

    return blocks;
}

/*
 * Define the block locations as the 4 corners
 */
void Image::encodeInit() {
    Matrix<pair<int, int>, Dynamic, Dynamic> blockLimits;

    blockLimits.resize(getHeight()/8, getWidth()/8);

    int row=0, col=0;
    // create the patch edges
    for(auto i=0; i<getHeight(); i+=8) {
        col = 0;
        for (auto j=0; j<getWidth(); j+=8) {
            blockLimits(row, col++) = make_pair(i, j);
        }
        row++;
    }

    // associate the patch edges to a patch. 4 edges per patch
    for(auto i=0; i<blockLimits.rows()-1; i++) {
        for(auto j=0; j<blockLimits.cols()-1; j++) {
            auto topL = blockLimits(i, j);
            auto topR = blockLimits(i, j+1);
            auto bottomL = blockLimits(i+1, j);
            auto bottomR = blockLimits(i+1, j+1);

            blockLocations.emplace_back(topL, topR, bottomL, bottomR);
        }
    }
}
