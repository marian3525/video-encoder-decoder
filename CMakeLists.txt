cmake_minimum_required(VERSION 3.14)
project(video_encoder_decoder)

set(CMAKE_CXX_STANDARD 17)

add_executable(video_encoder_decoder main.cpp Image/Image.cpp Image/Image.hpp Image/RgbPixel.cpp Image/RgbPixel.hpp Image/YCbCrPixel.cpp Image/YCbCrPixel.hpp Image/PixelConverter.cpp Image/PixelConverter.hpp encoding_utils/Block.cpp encoding_utils/Block.hpp)