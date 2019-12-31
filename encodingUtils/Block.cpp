#include <iostream>
#include "Block.hpp"
#include "DcCoefficient.hpp"
#include "AcCoefficient.hpp"
using namespace std;
#define PI (double)EIGEN_PI

Block Block::expandTo8x8() const {
    Block output{*this};

    // return the input if the type is Y since Y is already 8x8
    if (type == Y) {
        return output;
    }

    // if U or V, expand the 4x4 to 8x8
    // TODO, what happens to the locations?
    output.values.resize(8, 8);

    auto rowOutput = 0, colOutput = 0;
    for (int i = 0; i < 4; i++) {
        rowOutput = 2 * i;
        for (int j = 0; j < 4; j++) {
            colOutput = 2 * j;
            // reverse sub-sampling, place the values(i, j) in 4 spotss
            output.values(rowOutput, colOutput)
            = output.values(rowOutput, colOutput + 1)
            = output.values(rowOutput + 1,colOutput)
            = output.values(rowOutput + 1, colOutput + 1)
            = values(i, j);
        }
    }

    return output;
}

/**
 * Forward DCT of the values for a given block. Type and location remain untouched from the original
 * @return
 */
Block Block::forwardDCT() const {
    // copy constructor
    Block output{*this};

    // expand U and V blocks (4x4) to 8x8. Leave Y unchanged
    auto inputExpanded = output.expandTo8x8();
    output.values.resize(8, 8);
    output.values = inputExpanded.values;

    // subtract 128 from the expanded input block (this)
    for (auto i = 0; i < inputExpanded.values.rows(); i++) {
        for (auto j = 0; j < inputExpanded.values.cols(); j++) {
            output.values(i, j) -= 128;
        }
    }

    // compute G(u,v) = 1/4 * alpha(u) * alpha(v) * sum
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            output.values(u, v) = 0.25F * alpha(u) * alpha(v) * output.sumFDCT(u, v);
        }
    }

    // divide by Q
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            output.values(u, v) /= Q(u, v);
        }
    }

    return output;
}

float Block::alpha(const int &u) const {
    if (u == 0) {
        return 0.707;   // 1 / sqrt(2) ~ 0.707
    } else {
        return 1;
    }
}

Block Block::inverseDCT() const {

    // copy constructor
    Block output{*this};

    // multiply by Q
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            output.values(u, v) *= Q(u, v);
        }
    }

    // f(x,y)
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            output.values(x, y) = 0.25F * output.sumIDCT(x, y);
        }
    }

    // add 128
    for (auto i = 0; i < output.values.rows(); i++) {
        for (auto j = 0; j < output.values.cols(); j++) {
            output.values(i, j) += 128;
        }
    }

    return output;
}

float Block::sumFDCT(const int &u, const int &v) const {
    auto sum = 0.0F;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            sum += values(x, y) * cos(((2 * x + 1) * u * PI) / 16) * cos(((2 * y + 1) * v * PI) / 16);
        }
    }

    return sum;
}

float Block::sumIDCT(const int &x, const int &y) const {
    auto sum = 0.0F;

    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            sum += values(x, y) * cos(((2 * x + 1) * u * PI) / 16) * cos(((2 * y + 1) * v * PI) / 16);
        }
    }

    return sum;
}

/**
 * Compress a 8x8 block to 4x4 by averaging each 2x2 region to a single value
 * @return
 */
Block Block::compressTo4x4() const {
    // copy constructor
    Block output{*this};

    int col=0, row=0;

    // step 2 for i, j.
    // Take the average in each 2x2 sub-block
    for(int i=0; i<7; i+=2) {
        col=0;
        for(int j=0; j<7; j+=2) {
            // average the 4 values
            int sum = values(i, j) + values(i, j+1) + values(i+1, j) + values(i+1, j+1);
            output.values(row, col++) = sum/4;
        }
        row++;
    }

    return output;
}

std::vector<ACCoefficient> Block::entropy_encode() const {
    vector<ACCoefficient> output;
    vector<std::byte> zigZagParsed;

    bool isAtTop;
    bool isInTopTriangle;
    int i=0, j=0;

    while(i < 7 && j < 7) {

        isAtTop = i == 0;
        isInTopTriangle = i + j <= 7;

        zigZagParsed.push_back((byte)values(i, j));

        if(isAtTop && isInTopTriangle) {
            j++;
            while(j != 0) {
                zigZagParsed.push_back((byte)values(i, j));

                j--, i++;
            }
            continue;
        }

        if(!isAtTop && isInTopTriangle) {
            i++;
            while(i != 0) {
                zigZagParsed.push_back((byte)values(i, j));

                j++, i--;
            }
            continue;
        }

        if(isAtTop && !isInTopTriangle) {
            i++;
            while(i < 7) {
                zigZagParsed.push_back((byte)values(i, j));

                i++, j--;
            }
            continue;
        }
        if(!isAtTop && !isInTopTriangle) {
            j++;
            while(j < 7) {
                zigZagParsed.push_back((byte)values(i, j));

                i--, j++;
            }
            continue;
        }

    }
    // zigZagParsed ready

    // start building the output
    auto first = DCCoefficient(zigZagParsed[0]);
    // add the first value to the output as an AcCoef, even if it doesn't have a NoOfZeroes
    output.emplace_back(0, first);

    int noOfZeroes = 0;
    bool endingWithZero = false;
    for(const auto& val : zigZagParsed) {
        if(val == static_cast<std::byte>(0)) {
            // count the zeroes in front of the next non-zero value
           noOfZeroes++;
           endingWithZero = true;
           continue;
        }
        else {
            endingWithZero = false;
        }

        auto coef = ACCoefficient(noOfZeroes, DCCoefficient(val));

        output.push_back(coef);
    }

    if(endingWithZero) {
        // TODO, handle the case where the block ends with a sequence of 0s
    }

    return output;
}

Block Block::entropy_decode(std::vector<ACCoefficient> encodedBlock) {
    return Block();
}
