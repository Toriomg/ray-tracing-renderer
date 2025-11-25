#ifndef PAR_INCLUDE_IMAGE_HPP
#define PAR_INCLUDE_IMAGE_HPP

#include <cstdint>
#include <vector>

struct Image {
  std::vector<uint8_t> r;
  std::vector<uint8_t> g;
  std::vector<uint8_t> b;
};

#endif
