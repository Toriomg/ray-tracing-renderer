#ifndef IMAGE_SOA_HPP
#define IMAGE_SOA_HPP

#include <cstdint>
#include <vector>

struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct Image {
  std::vector<Pixel> Pixels;
};

#endif
