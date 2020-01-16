#pragma once

#include <cmath>

class DCCoefficient {
public:
    explicit DCCoefficient(int value): size{DCCoefficient::getScale(static_cast<int>(value))}, amplitude{value} {};
    int size;
    int amplitude;
private:
    static int getScale(int value) {
        if(value == 0) {
            return 0;
        }
        value = abs(value);
        for(int i=0; i<20; i++) {
            if(value >= pow(2, i) && value < pow(2, i+1)) {
                return i;
            }
        }
        assert(false);
    }
};