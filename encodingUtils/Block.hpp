#pragma once
#include <utility>

#include "../Eigen/Dense"
#include "../Eigen/src/Core/util/Constants.h"
#include "AcCoefficient.hpp"

using Eigen::Matrix;
using Eigen::Dynamic;

enum BlockType {Y, U, V};

class Block {
public:
    Block(Matrix<int, Dynamic, Dynamic> values, BlockType type, std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>>  location):
                                            values{std::move(values)}, type{type}, location{std::move(location)} {
        Q = Matrix<int, 8, 8>{};
        Q.row(0) << 6, 4, 4, 6, 10, 16, 20, 24;
        Q.row(1) << 5, 5, 6, 8, 10, 23, 24, 22;
        Q.row(2) << 6, 5, 6, 10, 16, 23, 28, 22;
        Q.row(3) << 6, 7, 9, 12, 20, 35, 32, 25;
        Q.row(4) << 7, 9, 15, 22, 27, 44, 41, 31;
        Q.row(5) << 10, 14, 22, 26, 32, 42, 45, 37;
        Q.row(6) << 20, 26, 31, 35, 41, 48, 48, 40;
        Q.row(7) << 29, 37, 38, 39, 45, 40, 41, 40;
    };

    Block() = delete;
    Block(const Block& toCopy) = default;
    Block(Block&& toMove) {
        values = toMove.values;
        type = toMove.type;
        location = toMove.location;
        Q = toMove.Q;
    };

    Matrix<int, Dynamic, Dynamic> values;
    BlockType type;
    std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>> location;

    Block forwardDCT() const;
    Block inverseDCT() const;

    std::vector<ACCoefficient> entropy_encode() const;
    static Block entropy_decode(std::vector<ACCoefficient> encodedBlock);

    Block expandTo8x8() const;
    Block compressTo4x4() const;

private:
    Matrix<int, Dynamic, Dynamic> Q;

    float alpha(const int& u) const;
    float sumFDCT(const int& u, const int& v) const;
    float sumIDCT(const int& u, const int& v) const;
};