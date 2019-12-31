#pragma once

#include "DcCoefficient.hpp"

class ACCoefficient {
public:
    ACCoefficient(int runlength, DCCoefficient dcCoefficient): runlength{runlength}, dcCoefficient{dcCoefficient} {};
    int runlength;
    DCCoefficient dcCoefficient;
};