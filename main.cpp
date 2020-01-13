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

    stream << "data:" << endl;
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

        // map the list of blocks to their transformed value
        for(const auto& block: blocks) {
            transformed.push_back(block.inverseDCT());
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



//    auto directTransformed = directTransformAll(encoded);
//    auto inverseTransformed = inverseTransformAll(directTransformed);
//    writeBlockToFile("../blocksOut/beforeDCT", get<0>(encoded)[0]);
//    writeBlockToFile("../blocksOut/afterDCT", get<0>(directTransformed)[0]);
//    writeBlockToFile("../blocksOut/afterIDCT", get<0>(inverseTransformed)[0]);

    vector<ACCoefficient> entropy_encoded;

    int noOfBlocks = get<0>(encoded).size();

    // place all ACCoefficients in the output
    for(size_t i=0; i<noOfBlocks; i++) {
        // get the i-th block of each type
        // skip the DCT part, expand to 8x8

        auto y = get<0>(encoded)[i].expandTo8x8();
        auto cb = get<1>(encoded)[i].expandTo8x8();
        auto cr = get<2>(encoded)[i].expandTo8x8();


        // encode them
        auto encodedY = y->entropy_encode();
        auto encodedCb = cb->entropy_encode();
        auto encodedCr = cr->entropy_encode();

        for(const auto& e : encodedY) {
            entropy_encoded.push_back(e);
        }

        for(const auto& e : encodedCb) {
            entropy_encoded.push_back(e);
        }

        for(const auto& e : encodedCr) {
            entropy_encoded.push_back(e);
        }
    }

    vector<Block*> tmpY;
    tmpY.reserve(7500);

    vector<Block*> tmpU;
    tmpU.reserve(7500);

    vector<Block*> tmpV;
    tmpV.reserve(7500);

    int i=0;
    while(i < (int)entropy_encoded.size()) {

        // read up to the point where the current block is full
        auto yBlock = Block::entropy_decode(entropy_encoded, i);
        yBlock->setType(Y);

        auto cbBlock = Block::entropy_decode(entropy_encoded, i);
        cbBlock->setType(U);

        auto crBlock = Block::entropy_decode(entropy_encoded, i);
        crBlock->setType(V);

        // got the 3 types of blocks for the current image patch
        tmpY.push_back(yBlock);
        tmpU.push_back(cbBlock);
        tmpV.push_back(crBlock);
    }
    // decode the array of ACCoefficients into blocks
    // blocks are then fed into entropy-decoder
    auto decoded_output = make_tuple(tmpY, tmpU, tmpV);

    //auto dequantized = inverseTransformAll(decoded_output);

//    // in inverseTransformed U and V are 8x8
//    // compress them back to 4x4 to display an image
    auto Y = get<0>(decoded_output);    // Y stays the same, at 8x8

    auto U = vector<Block*>{};
    // transform each U block into a 4x4

    for(const auto& block : get<1>(decoded_output)) {
        U.push_back(block->compressTo4x4());
    }

    auto V = vector<Block*>{};
    // transform each V block into a 4x4
    for(const auto& block : get<2>(decoded_output)) {
        V.push_back(block->compressTo4x4());
    }

    auto adjustedInverse = make_tuple(Y, U, V); // has the same structure as encoded, can be used to create an image

    // set the location of the blocks
    for(size_t i=0; i < get<0>(adjustedInverse).size(); i++) {
        get<0>(adjustedInverse)[i]->computeLocation(i);
        get<1>(adjustedInverse)[i]->computeLocation(i);
        get<2>(adjustedInverse)[i]->computeLocation(i);
    }

    //Image decoded = Image::decode(adjustedInverse);
    Image decoded = Image::decode(adjustedInverse, 600, 800);
    writeImageSample(decoded, "../blocksOut/decoded.txt");

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
    std::cout << duration << "ms"<<endl;

    decoded.write("../decoded.ppm");

    return 0;
}