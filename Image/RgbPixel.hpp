#pragma once

#include <algorithm>
#include "YCbCrPixel.hpp"

class RGBPixel {
public:
    RGBPixel();
    RGBPixel(unsigned char red, unsigned char green, unsigned char blue): red{red}, green{green}, blue{blue}{}

    unsigned char red;
    unsigned char green;
    unsigned char blue;
};