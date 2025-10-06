#pragma once
#include <vector>

struct Pixel{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Image{
    std::vector<Pixel> Pixels;
};