#ifndef PPM_WRITER_HPP
#define PPM_WRITER_HPP

#include "dataStructs/settings_structs.hpp"
#include <cstdint>
#include <string>
#include <vector>

class PPMWriter {
public:
  struct Pixels {
    std::vector<uint8_t> r_channel;
    std::vector<uint8_t> g_channel;
    std::vector<uint8_t> b_channel;
    size_t width;
    size_t height;

    Pixels(std::vector<uint8_t> const & r, std::vector<uint8_t> const & g,
           std::vector<uint8_t> const & b, size_t w, size_t h)
        : r_channel(r), g_channel(g), b_channel(b), width(w), height(h) { }
  };

  static bool write_ppm(std::string const & filename, Pixels const & pixels,
                        ParallelSettings const * settings = nullptr);
};

#endif
