#include <iostream>
#include <chrono>
#include "Image/Image.hpp"

using namespace std;

int main() {
    auto t1 = std::chrono::high_resolution_clock::now();

    auto img = Image{"/home/marian/Documents/CS/PSAV/video-encoder-decoder/image.ppm"};
    img.getYCbCrImage();

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << duration/1000 << "ms"<<endl;
    return 0;
}