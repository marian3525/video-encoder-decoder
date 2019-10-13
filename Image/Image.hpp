#pragma once

#include "../Eigen/Dense"
#include "RgbPixel.hpp"
#include "../Eigen/src/Core/util/Constants.h"

using Eigen::Matrix;
using Eigen::Dynamic;

class Image {
public:
    explicit Image(const std::string& filename);
    explicit Image(const Matrix<RGBPixel, Dynamic, Dynamic>& image);
    explicit Image(const Matrix<YCbCrPixel, Dynamic, Dynamic>& image);

    Matrix<YCbCrPixel, Dynamic, Dynamic> getYCbCrImage();
    Matrix<RGBPixel, Dynamic, Dynamic> getRGBImage();

    void write(const std::string& file);

    ~Image();
private:
    Matrix<RGBPixel, Dynamic, Dynamic> rgbImage;
    bool rgbLoaded = false;

    Matrix<YCbCrPixel, Dynamic, Dynamic> yCbCrImage;
    bool yCbCrLoaded = false;
};