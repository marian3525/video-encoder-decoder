#include <iostream>
#include <chrono>
#include "Image/Image.hpp"

using namespace std;

int main() {
    auto t1 = std::chrono::high_resolution_clock::now();

    auto img = Image{"/home/marian/Documents/CS/PSAV/video-encoder-decoder/image.ppm"};
    //auto converted = img.getYCbCrImage();

    // create a list of 8x8 matrixes containing the 64 values, the type of block (Y) and the position of the block in
    // the original image

    auto encoded = img.encode();
    auto decoded = Image::decode(encoded);

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << duration/1000 << "ms"<<endl;

    decoded.write("f.ppm");

    return 0;
}