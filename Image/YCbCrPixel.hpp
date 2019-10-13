#pragma once

#include "RgbPixel.hpp"

class YCbCrPixel {
public:
    YCbCrPixel();
    YCbCrPixel(unsigned char Y, unsigned char Cb, unsigned char Cr): Y{Y}, Cb{Cb}, Cr{Cr} {};

    unsigned char Y;
    unsigned char Cb;
    unsigned char Cr;
};