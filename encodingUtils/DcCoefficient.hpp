#pragma once

#include <cmath>

class DCCoefficient {
public:
    explicit DCCoefficient(std::byte value): size{DCCoefficient::getScale(static_cast<int>(value))}, amplitude{value} {};
    int size;
    std::byte amplitude;
private:
    static int getScale(int value) {
        for(int i=0; i<9; i++) {
            if(value > pow(2, i) && value < pow(2, i+1)) {
                return i;
            }
        }
    }
};