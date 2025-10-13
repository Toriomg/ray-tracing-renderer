#ifndef PPM_WRITER_HPP
#define PPM_WRITER_HPP

#include <cstdint>
#include <string>
#include <vector>

class PPMWriter {
public:
  struct Pixels{
    std::vector<uint8_t> r_channel;
    std::vector<uint8_t> g_channel;
    std::vector<uint8_t> b_channel;

    Pixels(const std::vector<uint8_t>& r, const std::vector<uint8_t>& g, const std::vector<uint8_t>& b)
        : r_channel(r),
          g_channel(g),
          b_channel(b)
    {
    }
  };
  static bool write_ppm(const std::string& filename,
                     const Pixels& pixels,
                     size_t width, size_t height);
};

#endif
