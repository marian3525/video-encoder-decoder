#pragma once
#include "RgbPixel.hpp"

class PixelConverter {
public:
    static RGBPixel YCbCrToRGB(YCbCrPixel source) {
        RGBPixel output;

        int r = (int) (source.Y + 1.40200 * (source.Cr - 0x80));
        output.red = std::max(0, std::min(255, r));

        int g = (int) (source.Y - 0.34414 * (source.Cb - 0x80) - 0.71414 * (source.Cr - 0x80));
        output.green = std::max(0, std::min(255, g));

        int b = (int) (source.Y + 1.77200 * (source.Cb - 0x80));
        output.blue = std::max(0, std::min(255, b));

        return output;
    }

    static YCbCrPixel RGBToYCbCr(RGBPixel source) {
        YCbCrPixel output;

        output.Y = static_cast<unsigned char>(0.299 * source.red + 0.587 * source.green + 0.114 * source.blue);
        output.Cb = static_cast<unsigned char>(128 - 0.1687 * source.red - 0.3312 * source.green + 0.5 * source.blue);
        output.Cr = static_cast<unsigned char>(128 + 0.5 * source.red - 0.4186 * source.green - 0.0813 * source.blue);

        return output;
    }
};