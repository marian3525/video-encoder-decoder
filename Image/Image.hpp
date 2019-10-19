#pragma once

#include "../Eigen/Dense"
#include "RgbPixel.hpp"
#include "../Eigen/src/Core/util/Constants.h"
#include "../encoding_utils/Block.hpp"

using Eigen::Matrix;
using Eigen::Dynamic;

class Image {
public:
    explicit Image(const std::string& filename);
    explicit Image(const Matrix<RGBPixel, Dynamic, Dynamic>& image);
    explicit Image(const Matrix<YCbCrPixel, Dynamic, Dynamic>& image);

    Matrix<YCbCrPixel, Dynamic, Dynamic> getYCbCrImage();
    Matrix<RGBPixel, Dynamic, Dynamic> getRGBImage();
    std::tuple<std::vector<Block>, std::vector<Block>, std::vector<Block>> encode();

    void decode(std::tuple<std::vector<Block>, std::vector<Block>, std::vector<Block>>);

    int getWidth() const;
    int getHeight() const;

    void write(const std::string& file);

    ~Image();
private:
    void encodeInit();
    std::vector<Block> encodeYComponent();
    std::vector<Block> encodeUComponent();
    std::vector<Block> encodeVComponent();

    std::vector<std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>>> blockLocations;

    Matrix<RGBPixel, Dynamic, Dynamic> rgbImage;
    bool rgbLoaded = false;

    Matrix<YCbCrPixel, Dynamic, Dynamic> yCbCrImage;
    bool yCbCrLoaded = false;

    int width;
    int height;
};