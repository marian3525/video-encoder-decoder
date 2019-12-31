#include <iostream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include "ImageUtils/Image.hpp"
#include "encodingUtils/Block.hpp"

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

tuple<vector<Block>, vector<Block>, vector<Block>> directTransformAll(tuple<vector<Block>, vector<Block>, vector<Block>> input) {
    // put all blocks into a single vector
    auto Y = get<0>(input);
    auto U = get<1>(input);
    auto V = get<2>(input);

    // apply a forwardDCT to all elements of a list and return the list
    auto transformOperation = [&](const vector<Block>& blocks) {
        vector<Block> transformed;  // vector to write the transformed blocks into

        // map the list of blocks to their transformed value
        for(auto& block: blocks) {
            transformed.push_back(block.forwardDCT());
        }

        return transformed;
    };

    return make_tuple(transformOperation(Y), transformOperation(U), transformOperation(V));
}

tuple<vector<Block>, vector<Block>, vector<Block>> inverseTransformAll(tuple<vector<Block>, vector<Block>, vector<Block>> input) {
    // put all blocks into a single vector
    auto Y = get<0>(input);
    auto U = get<1>(input);
    auto V = get<2>(input);

    // apply a forwardDCT to all elements of a list and return the list
    auto transformOperation = [&](const vector<Block>& blocks) {
        vector<Block> transformed;  // vector to write the transformed blocks into
        transformed.reserve(blocks.size());

        // map the list of blocks to their transformed value
        for(const auto& block: blocks) {
            transformed.push_back(std::move(block.inverseDCT()));
        }

        return transformed;
    };

    return make_tuple(transformOperation(Y), transformOperation(U), transformOperation(V));
}

int main() {
    auto t1 = std::chrono::high_resolution_clock::now();

    auto img = Image{"../image.ppm"};
    //auto converted = img.getYCbCrImage();
    writeImageSample(img, "../blocksOut/original.txt");

    // create a list of 8x8 matrixes containing the 64 values, the type of block (Y) and the position of the block in
    // the original image
    tuple<vector<Block>, vector<Block>, vector<Block>> encoded = img.encode();

    auto directTransformed = directTransformAll(encoded);
    auto inverseTransformed = inverseTransformAll(directTransformed);

    auto b = get<0>(directTransformed)[0];
    // zig zag parsing
    auto x = b.entropy_encode();
    // encoded should be the same as inverseTransformed
//    for(int i=0; i<get<0>(encoded).size(); i++) {
//        auto x = get<0>(encoded)[i];
//        auto inv = get<0>(inverseTransformed)[i];
//        for(int j=0; j<x.values.rows();j++) {
//            for(int k=0; k<x.values.cols(); k++) {
//               if(abs(x.values(j, k) - inv.values(j, k)) > 0.1) {
//                   cout<<x.values(j, k)<<" "<<inv.values(j, k)<<endl;
//               }
//
//            }
//        }
//    }
//
//    // in inverseTransformed U and V are 8x8
//    // compress them back to 4x4 to display an image
//    auto Y = get<0>(inverseTransformed);    // Y stays the same, at 8x8
//
//    auto U = vector<Block>{};
//    U.reserve(Y.size());
//    // transform each U block into a 4x4
//    //transform(get<1>(inverseTransformed).begin(), get<1>(inverseTransformed).end(), U.begin(), [&](const Block& block){return move(block.compressTo4x4());});
//    for(const auto& block : get<1>(inverseTransformed)) {
//        U.push_back(move(block.compressTo4x4()));
//    }
//
//    auto V = vector<Block>{};
//    V.reserve(Y.size());
//    // transform each V block into a 4x4
//    //transform(get<2>(inverseTransformed).begin(), get<2>(inverseTransformed).end(), V.begin(), [&](const Block& block){return block.compressTo4x4();});
//    for(const auto& block : get<2>(inverseTransformed)) {
//        V.push_back(move(block.compressTo4x4()));
//    }
//    auto adjustedInverse = make_tuple(Y, U, V); // has the same structure as encoded, can be used to create an image
//
//    writeBlockToFile("../blocksOut/YBlockOriginal.txt", get<0>(encoded)[0]);
//    writeBlockToFile("../blocksOut/Ydirect.txt", get<0>(directTransformed)[0]);
//    writeBlockToFile("../blocksOut/Yinverse.txt", get<0>(inverseTransformed)[0]);
//
//    writeBlockToFile("../blocksOut/UBlockOriginal.txt", get<1>(encoded)[0]);
//    writeBlockToFile("../blocksOut/UBlockExpanded.txt", get<1>(encoded)[0].expandTo8x8());
//    writeBlockToFile("../blocksOut/Udirect.txt", get<1>(directTransformed)[0]);
//    writeBlockToFile("../blocksOut/Uinverse.txt", get<1>(inverseTransformed)[0]);
//
//    writeBlockToFile("../blocksOut/VBlockOriginal.txt", get<2>(encoded)[0]);
//    writeBlockToFile("../blocksOut/Vdirect.txt", get<2>(directTransformed)[0]);
//    writeBlockToFile("../blocksOut/Vinverse.txt", get<2>(inverseTransformed)[0]);
//    writeBlockToFile("../blocksOut/UBlock.txt", get<1>(encoded)[0]);
//    writeBlockToFile("../blocksOut/VBlock.txt", get<2>(encoded)[0]);

    //Image decoded = Image::decode(encoded);
//    Image decoded = Image::decode(adjustedInverse);
//    writeImageSample(decoded, "../blocksOut/decoded.txt");
//
//    auto t2 = std::chrono::high_resolution_clock::now();
//    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
//    std::cout << duration << "ms"<<endl;
//
//    decoded.write("../decoded.ppm");

    return 0;
}