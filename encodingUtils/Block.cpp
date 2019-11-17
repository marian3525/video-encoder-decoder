#include <iostream>
#include "Block.hpp"

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
            // reverse sub-sampling
            output.values(rowOutput, colOutput) = output.values(rowOutput, colOutput + 1) = output.values(rowOutput + 1,
                                                                                                          colOutput) = output.values(
                    rowOutput + 1, colOutput + 1)
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
    auto inputExpanded = expandTo8x8();
    Block output{*this};

    // subtract 128
    for (auto i = 0; i < inputExpanded.values.rows(); i++) {
        for (auto j = 0; j < inputExpanded.values.cols(); j++) {
            inputExpanded.values(i, j) -= 128;
        }
    }

    // G(u,v)
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            output.values(u, v) = 0.25F * alpha(u) * alpha(v) * sumFDCT(u, v);
        }
    }

    auto Q = Matrix<uint8_t, 8, 8>{};
    Q.row(0) << 6, 4, 4, 6, 10, 16, 20, 24;
    Q.row(1) << 5, 5, 6, 8, 10, 23, 24, 22;
    Q.row(2) << 6, 5, 6, 10, 16, 23, 28, 22;
    Q.row(3) << 6, 7, 9, 12, 20, 35, 32, 25;
    Q.row(4) << 7, 9, 15, 22, 27, 44, 41, 31;
    Q.row(5) << 10, 14, 22, 26, 32, 42, 45, 37;
    Q.row(6) << 20, 26, 31, 35, 41, 48, 48, 40;
    Q.row(7) << 29, 37, 38, 39, 45, 40, 41, 40;


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
        return 0.707;
    } else {
        return 1;
    }
}

Block Block::inverseDCT() const {
    Block output{*this};

    auto Q = Matrix<uint8_t, 8, 8>{};
    Q.row(0) << 6, 4, 4, 6, 10, 16, 20, 24;
    Q.row(1) << 5, 5, 6, 8, 10, 23, 24, 22;
    Q.row(2) << 6, 5, 6, 10, 16, 23, 28, 22;
    Q.row(3) << 6, 7, 9, 12, 20, 35, 32, 25;
    Q.row(4) << 7, 9, 15, 22, 27, 44, 41, 31;
    Q.row(5) << 10, 14, 22, 26, 32, 42, 45, 37;
    Q.row(6) << 20, 26, 31, 35, 41, 48, 48, 40;
    Q.row(7) << 29, 37, 38, 39, 45, 40, 41, 40;


    // divide by Q
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            output.values(u, v) *= Q(u, v);
        }
    }

    // f(u,v)
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            output.values(x, y) = 0.25F * sumIDCT(x, y);
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
            sum += cos(((2 * x + 1) * u * EIGEN_PI) / 16) * cos(((2 * y + 1) * v * EIGEN_PI) / 16);
        }
    }

    return sum;
}

float Block::sumIDCT(const int &x, const int &y) const {
    auto sum = 0.0F;

    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            sum += cos(((2 * x + 1) *u * EIGEN_PI) / 16) * cos(((2 * y + 1) * v * EIGEN_PI) / 16);
        }
    }

    return sum;
}