#include <iostream>
#include <chrono>
#include <fstream>
#include "Image/Image.hpp"

using namespace std;

void writeBlockToFile(const string& filename, const Block& block) {
    auto stream = ofstream(filename);

    stream << "First "<< block.type << " block:" << endl;
    for(auto i=0; i<block.values.rows(); i++) {
        for(auto j=0; j<block.values.cols(); j++) {
            // print the source elements for the first block
            string tmp;
            auto pixel = block.values(i, j);
            tmp.append("[").append(to_string(pixel)).append("]");
            stream << tmp;
        }
        stream << endl;
    }
    stream.close();
}

void writeImageSample(Image& img, const string& filename) {
    auto stream = ofstream(filename);

    stream << "Original data:" << endl;
    for(auto i=0; i<8; i++) {
        for(auto j=0; j<8; j++) {
            // print the source elements for the first block
            string tmp;
            auto pixel = img(i, j);
            tmp.append("[").append(to_string(pixel.red)).append(",").append(to_string(pixel.green)).append(",").append(to_string(pixel.blue)).append("] ");
            stream << tmp;
        }
        stream << endl;
    }
    stream.close();
}

int main() {
    auto t1 = std::chrono::high_resolution_clock::now();

    auto img = Image{"/home/marian/Documents/CS/PSAV/video-encoder-decoder/image.ppm"};
    //auto converted = img.getYCbCrImage();

    // create a list of 8x8 matrixes containing the 64 values, the type of block (Y) and the position of the block in
    // the original image

    writeImageSample(img, "/home/marian/Documents/CS/PSAV/video-encoder-decoder/blocksOut/original.txt");

    auto encoded = img.encode();

    writeBlockToFile("/home/marian/Documents/CS/PSAV/video-encoder-decoder/blocksOut/YBlock.txt", get<0>(encoded)[0]);
    writeBlockToFile("/home/marian/Documents/CS/PSAV/video-encoder-decoder/blocksOut/UBlock.txt", get<1>(encoded)[0]);
    writeBlockToFile("/home/marian/Documents/CS/PSAV/video-encoder-decoder/blocksOut/VBlock.txt", get<2>(encoded)[0]);

    auto decoded = Image::decode(encoded);
    writeImageSample(decoded, "/home/marian/Documents/CS/PSAV/video-encoder-decoder/blocksOut/decoded.txt");

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << duration/1000 << "ms"<<endl;
    //decoded.write("f.ppm");

    return 0;
}