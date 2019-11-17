#pragma once
#include <utility>

#include "../Eigen/Dense"
#include "../Eigen/src/Core/util/Constants.h"

using Eigen::Matrix;
using Eigen::Dynamic;

enum BlockType {Y, U, V};

class Block {
public:
    Block(Matrix<uint8_t, Dynamic, Dynamic> values, BlockType type, std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>>  location):
                                            values{std::move(values)}, type{type}, location{std::move(location)} {};
    Block() = delete;
    Block(const Block& toCopy) = default;
    Block(Block&& toMove) = delete;

    Matrix<uint8_t, Dynamic, Dynamic> values;
    BlockType type;
    std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>> location;

    Block forwardDCT() const;
    Block inverseDCT() const;

    Block expandTo8x8() const;

private:
    float alpha(const int& u) const;
    float sumFDCT(const int& u, const int& v) const;
    float sumIDCT(const int& u, const int& v) const;
};