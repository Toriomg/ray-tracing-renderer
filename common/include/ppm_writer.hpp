#ifndef PPM_WRITER_HPP
#define PPM_WRITER_HPP

#include <cstdint>
#include <string>
#include <vector>

class PPMWriter {
public:
  struct Pixels {
    std::vector<uint8_t> r_channel;
    std::vector<uint8_t> g_channel;
    std::vector<uint8_t> b_channel;

    Pixels(std::vector<uint8_t> const & r, std::vector<uint8_t> const & g,
           std::vector<uint8_t> const & b)
        : r_channel(r), g_channel(g), b_channel(b) { }
  };

  static bool write_ppm(std::string const & filename, Pixels const & pixels, size_t width,
                        size_t height);
};

#endif
