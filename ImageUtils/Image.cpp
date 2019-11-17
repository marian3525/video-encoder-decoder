#include <fstream>
#include "Image.hpp"
#include "PixelConverter.hpp"

using namespace std;
/**
 * Create an ImageUtils from the give RGB image
 * @param image
 */
Image::Image(const Matrix<RGBPixel, Dynamic, Dynamic>& image) {
    rgbImage = image;
    rgbLoaded = true;

    encodeInit();
}

/**
 * Create an ImageUtils from the give YCbCr image
 * @param image
 */
Image::Image(const Matrix<YCbCrPixel, Dynamic, Dynamic>& image) {
    yCbCrImage = image;
    yCbCrLoaded = true;

    encodeInit();
}

/**
 * Create an ImageUtils from a file
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
    f<<"# encoded/decoded image"<<endl;
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
    getYCbCrImage();
    auto yBlocks = encodeYComponent();
    auto uBlocks = encodeUComponent();
    auto vBlocks = encodeVComponent();

    return make_tuple(yBlocks, uBlocks, vBlocks);
}

vector<Block> Image::encodeYComponent() {
    vector<Block> blocks;
    Matrix<uint8_t, Dynamic, Dynamic> tmpVals;
    tmpVals.resize(8,8);

    for(const tuple<pair<int, int>, pair<int, int>, pair<int, int>, pair<int, int>>& corners : blockLocations){
        int row=0, col=0;
        // load all Y values in the tmpVals vector, create a block with this vector and add it to the blocks vector
        for(int i=get<0>(corners).first; i < get<2>(corners).first; i++) {

            col=0;
            for(int j=get<0>(corners).second; j < get<1>(corners).second; j++) {
                tmpVals(row, col++) = yCbCrImage(i, j).Y;
            }
            row++;
        }

        blocks.emplace_back(tmpVals, Y, corners);
    }

    return blocks;
}

std::vector<Block> Image::encodeUComponent() {
    vector<Block> blocks;
    Matrix<uint8_t, Dynamic, Dynamic> tmpVals;
    tmpVals.resize(4,4);

    // corners: corner coords of a block
    for(const tuple<pair<int, int>, pair<int, int>, pair<int, int>, pair<int, int>>& corners : blockLocations){
        int row=0, col=0;

        // step 2 for i, j.
        // Take the average in each 2x2 sub-block
        for(int i=get<0>(corners).first; i<get<2>(corners).first-1; i+=2) {
            col=0;

            for(int j=get<0>(corners).second; j<get<1>(corners).second-1; j+=2) {
                // average the 4 values
                int sum = yCbCrImage(i, j).Cb      + yCbCrImage(i, j+1).Cb
                        + yCbCrImage(i+1, j).Cb + yCbCrImage(i+1, j+1).Cb;
                tmpVals(row, col++) = sum/4;
            }
            row++;
        }

        blocks.emplace_back(tmpVals, V, corners);
    }

    return blocks;
}

std::vector<Block> Image::encodeVComponent() {
    vector<Block> blocks;
    Matrix<uint8_t, Dynamic, Dynamic> tmpVals;
    tmpVals.resize(4,4);

    for(const tuple<pair<int, int>, pair<int, int>, pair<int, int>, pair<int, int>>& corners : blockLocations){
        int row=0, col=0;
        // step 2 for i, j.
        // Take the average in each 2x2 sub-block
        for(int i=get<0>(corners).first; i<get<2>(corners).first-1; i+=2) {
            col=0;

            for(int j=get<0>(corners).second; j<get<1>(corners).second-1; j+=2) {
                // average the 4 values
                int sum = yCbCrImage(i, j).Cr       + yCbCrImage(i, j+1).Cr
                          + yCbCrImage(i+1, j).Cr + yCbCrImage(i+1, j+1).Cr;
                tmpVals(row, col++) = sum/4;
            }
            row++;
        }

        blocks.emplace_back(tmpVals, U, corners);
    }

    return blocks;
}

/**
 * Define the block locations as the 4 corners
 */
void Image::encodeInit() {
    Matrix<pair<int, int>, Dynamic, Dynamic> blockLimits;

    blockLimits.resize(getHeight()/8+1, getWidth()/8+1);

    int row=0, col=0, i, j;
    // create the patch edges
    for(i=0; i<=getHeight()/8; i++) {
        for (j=0; j<=getWidth()/8; j++) {
            blockLimits(i, j) = make_pair(i*8, j*8);
        }
    }

    auto p = blockLimits(75, 100);
    // associate the patch edges to a patch. 4 edges per patch
    for(i=0; i<blockLimits.rows()-1; i++) {
        for(j=0; j<blockLimits.cols()-1; j++) {
            auto topL = blockLimits(i, j);
            auto topR = blockLimits(i, j+1);
            auto bottomL = blockLimits(i+1, j);
            auto bottomR = blockLimits(i+1, j+1);

            blockLocations.emplace_back(topL, topR, bottomL, bottomR);
        }
    }
}

Image Image::decode(tuple<vector<Block>, vector<Block>, vector<Block>> uviBlocks) {

    Matrix<YCbCrPixel, Dynamic, Dynamic> imageMatrix;
    imageMatrix.resize(600, 800);

    // place the Y values
    auto YValues = get<0>(uviBlocks);
    for(const Block& block : YValues) {
        auto row_start = get<0>(block.location).first;
        auto row_end = get<2>(block.location).first;
        auto col_start = get<0>(block.location).second;
        auto col_end = get<1>(block.location).second;
        auto row = 0, col = 0;

        for(auto i=row_start; i<row_end; i++) {
            col = 0;
            for(auto j=col_start; j<col_end; j++) {
                imageMatrix(i, j).Y = block.values(row, col);
                col++;
            }
            row++;
        }
    }

    // place the Cb values
    auto CbValues = get<1>(uviBlocks);
    for(const Block& block: CbValues) {
        auto row_start = get<0>(block.location).first;
        auto row_end = get<2>(block.location).first;
        auto col_start = get<0>(block.location).second;
        auto col_end = get<1>(block.location).second;
        auto row = 0, col = 0;

        // for each element in the block's elements, expand each value to a 2x2 matrix and place it in the image matrix
        for(auto i=row_start; i<row_end; i+=2) {
            col = 0;

            for(auto j=col_start; j<col_end; j+=2) {
                auto value = block.values(row, col);

                imageMatrix(i, j).Cb = value;
                imageMatrix(i, j+1).Cb = value;
                imageMatrix(i+1, j).Cb = value;
                imageMatrix(i+1, j+1).Cb = value;

                col++;
            }
            row++;
        }
    }

    // place the Cr values
    auto CrValues = get<2>(uviBlocks);
    for(const Block& block: CrValues) {
        auto row_start = get<0>(block.location).first;
        auto row_end = get<2>(block.location).first;
        auto col_start = get<0>(block.location).second;
        auto col_end = get<1>(block.location).second;
        auto row = 0, col = 0;

        // for each element in the block's elements, expand each value to a 2x2 matrix and place it in the image matrix
        for(auto i=row_start; i<row_end; i+=2) {
            col = 0;
            for(auto j=col_start; j<col_end; j+=2) {
                auto value = block.values(row, col);

                imageMatrix(i, j).Cr = value;
                imageMatrix(i, j+1).Cr = value;
                imageMatrix(i+1, j).Cr = value;
                imageMatrix(i+1, j+1).Cr = value;

                col++;
            }
            row++;
        }
    }

    return Image(imageMatrix);
}

RGBPixel Image::operator()(int row, int col) {
    getRGBImage();
    return rgbImage(row, col);
}
