#include <iostream>
#include "Block.hpp"
#include "DcCoefficient.hpp"
#include "AcCoefficient.hpp"
using namespace std;
#define PI (double)EIGEN_PI

int Block::END_OF_BLOCK = -2;

Block* Block::expandTo8x8() const {
    auto* output = new Block{*this};

    // return the input if the type is Y since Y is already 8x8
    if (type == Y) {
        return output;
    }

    // if U or V, expand the 4x4 to 8x8
    // TODO, what happens to the locations?
    output->values.resize(8, 8);

    auto rowOutput = 0, colOutput = 0;
    for (int i = 0; i < 4; i++) {
        rowOutput = 2 * i;
        for (int j = 0; j < 4; j++) {
            colOutput = 2 * j;
            // reverse sub-sampling, place the values(i, j) in 4 spotss
            output->values(rowOutput, colOutput)
            = output->values(rowOutput, colOutput + 1)
            = output->values(rowOutput + 1,colOutput)
            = output->values(rowOutput + 1, colOutput + 1)
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
    output.values = inputExpanded->values;

    // subtract 128 from the expanded input block (this)
    for (auto i = 0; i < inputExpanded->values.rows(); i++) {
        for (auto j = 0; j < inputExpanded->values.cols(); j++) {
            output.values(i, j) -= 128;
        }
    }

    // compute G(u,v) = 1/4 * alpha(u) * alpha(v) * sum FOR THE INPUT BLOCK, NOT output
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            output.values(u, v) = 0.25F * alpha(u) * alpha(v) * this->sumFDCT(u, v);
        }
    }

    // divide by Q: quantize the DCT coefs. obtained above
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
            output.values(x, y) = int(0.25F * output.sumIDCT(x, y));
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
    float sum = 0.0F;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            sum += (float)values(x, y) * cos(((2 * x + 1) * u * PI) / 16) * cos(((2 * y + 1) * v * PI) / 16);
        }
    }

    return sum;
}

float Block::sumIDCT(const int &x, const int &y) const {
    auto sum = 0.0F;

    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            sum += alpha(u) * alpha(v) * (float)values(x, y) * cos(((2 * x + 1) * u * PI) / 16) * cos(((2 * y + 1) * v * PI) / 16);
        }
    }

    return sum;
}

/**
 * Compress a 8x8 block to 4x4 by averaging each 2x2 region to a single value
 * @return
 */
Block* Block::compressTo4x4() const {
    // copy constructor
    Block* output = new Block{*this};

    int col=0, row=0;

    // step 2 for i, j.
    // Take the average in each 2x2 sub-block
    for(int i=0; i<7; i+=2) {
        col=0;
        for(int j=0; j<7; j+=2) {
            // average the 4 values
            int sum = values(i, j) + values(i, j+1) + values(i+1, j) + values(i+1, j+1);
            output->values(row, col++) = sum/4;
        }
        row++;
    }

    return output;
}

std::vector<ACCoefficient> Block::entropy_encode() const {
    vector<ACCoefficient> output;

    vector<int> zigZagParsed = zigZagParse(*this);

    // zigZagParsed ready

    // start building the output
    auto first = DCCoefficient(zigZagParsed[0]);
    // add the first value to the output as an AcCoef, even if it doesn't have a NoOfZeroes
    output.emplace_back(0, first);

    int noOfZeroes = 0;
    bool endingWithZero = false;
    for(size_t i=1; i<zigZagParsed.size(); i++) {
        auto val = zigZagParsed[i];
        if(val == 0) {
            // count the zeroes in front of the next non-zero value
           noOfZeroes++;
           endingWithZero = true;
           continue;
        }
        else {
            endingWithZero = false;

            auto coef = ACCoefficient(noOfZeroes, DCCoefficient(val));
            output.push_back(coef);

            noOfZeroes = 0;
        }
    }

    if(endingWithZero) {
        // in case the block ends with a 0
        auto c = ACCoefficient(noOfZeroes, DCCoefficient(END_OF_BLOCK));
        output.push_back(c);
    }

    return output;
}

/**
 * Build a block from the list of coefficients.
 *
 * Strategy:
 * - get the list of (byte) coefficients from the list of AC coefficients
 * @param coefList
 * @param last_idx The index of the last processed coef. from the coef list
 * @return
 */
Block* Block::entropy_decode(std::vector<ACCoefficient> coefList, int& last_idx) {

    DCCoefficient dcCoefficient = coefList[last_idx++].dcCoefficient;
    vector<int> pixelValues;

    // add the first value (dcCoef)
    pixelValues.push_back(dcCoefficient.amplitude);
    // for the AC Coefs. : place <runLength> zeroes THEN place the value

    while(pixelValues.size() < 64) {
        auto acCoef = coefList[last_idx++];
        int zeroesBefore = acCoef.runlength;
        int value = acCoef.dcCoefficient.amplitude;

        // place the zeroes
        while(zeroesBefore) {
            pixelValues.push_back(0);
            zeroesBefore--;
        }

        if(value != END_OF_BLOCK) {
            // place the non-zero amplitude that follows the 0s
            pixelValues.push_back(value);
        }
    }

    assert(pixelValues.size() == 64);
    // build a block matrix from the byte vector (zig-zag parsing in reverse)
    auto blockMatrix = zigZagReverse(pixelValues);

    // build a block from the block matrix
    auto block = new Block(blockMatrix);
    return block;
}

std::vector<int> Block::zigZagParse(const Block &block) {
    vector<int> zigZagParsed;

    bool isAtTop;
    bool isInTopTriangle;
    size_t i=0, j=0;

    while(true) {
        if(i > 7 or j > 7)
            break;
        isAtTop = i == 0 || j == 7;
        isInTopTriangle = i + j < 7;

        zigZagParsed.push_back(block.values(i, j));

        if(isAtTop && isInTopTriangle) {
            j++;
            while(j != 0) {
                zigZagParsed.push_back(block.values(i, j));

                j--, i++;
            }
            continue;
        }

        if(!isAtTop && isInTopTriangle) {
            i++;
            while(i != 0) {
                zigZagParsed.push_back(block.values(i, j));

                j++, i--;
            }
            continue;
        }

        // the case when it is on the right wall of the matrix
        if(j == 7) {
            i++;
            while(i < 7) {
                zigZagParsed.push_back(block.values(i, j));

                i++, j--;
            }
            continue;
        }
        if(!isAtTop) {
            j++;
            while(j < 7) {
                zigZagParsed.push_back(block.values(i, j));

                i--, j++;
            }
            continue;
        }
    }

    return zigZagParsed;
}

Matrix<int, Dynamic, Dynamic> Block::zigZagReverse(const std::vector<int>& pixelValues) {
    Matrix<int, Dynamic, Dynamic> blockMatrix;
    blockMatrix.resize(8, 8);

    bool isAtTop;
    bool isInTopTriangle;
    size_t i=0, j=0, pixelIdx = 0;

    while(true) {
        if(i > 7 or j > 7)
            break;
        isAtTop = i == 0 || j == 7;
        isInTopTriangle = i + j < 7;

        blockMatrix(i, j) = pixelValues[pixelIdx++];

        if(isAtTop && isInTopTriangle) {
            j++;
            while(j != 0) {
                blockMatrix(i, j) = pixelValues[pixelIdx++];

                j--, i++;
            }
            continue;
        }

        if(!isAtTop && isInTopTriangle) {
            i++;
            while(i != 0) {
                blockMatrix(i, j) = pixelValues[pixelIdx++];

                j++, i--;
            }
            continue;
        }

        // the case when it is on the right wall of the matrix
        if(j == 7) {
            i++;
            while(i < 7) {
                blockMatrix(i, j) = pixelValues[pixelIdx++];

                i++, j--;
            }
            continue;
        }
        if(!isAtTop) {
            j++;
            while(j < 7) {
                blockMatrix(i, j) = pixelValues[pixelIdx++];

                i--, j++;
            }
            continue;
        }
    }

    return blockMatrix;
}

/**
 *
 * @param i
 */
void Block::computeLocation(const int& i) {
    // the image is 600 rows by 800 cols
    // that makes it 600/8=75 Y blocks by 100 Y blocks
    // U: 600/4=150 by 200
    // V: 150 by 200

    // compute the coords depending on type
    pair<int, int> topLeft;
    pair<int, int> topRight;
    pair<int, int> bottomLeft;
    pair<int, int> bottomRight;
    int row, col;

    row = i / 100;
    col = i % 100;
    topLeft = make_pair(8 * row, 8 * col);
    topRight = make_pair(8 * row, 8 * col + 8);
    bottomLeft = make_pair(8 * row + 8, 8 * col);
    bottomRight = make_pair(8 * row + 8, 8 * col + 8);
    location = make_tuple(topLeft, topRight, bottomLeft, bottomRight);
}
