cmake_minimum_required(VERSION 3.10)
project(video_encoder_decoder)

set(CMAKE_CXX_STANDARD 17)

add_executable(video_encoder_decoder main.cpp ImageUtils/Image.cpp ImageUtils/Image.hpp ImageUtils/RgbPixel.cpp ImageUtils/RgbPixel.hpp ImageUtils/YCbCrPixel.cpp ImageUtils/YCbCrPixel.hpp ImageUtils/PixelConverter.cpp ImageUtils/PixelConverter.hpp encodingUtils/Block.cpp encodingUtils/Block.hpp encodingUtils/DcCoefficient.hpp encodingUtils/AcCoefficient.hpp)