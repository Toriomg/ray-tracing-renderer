#ifndef PPM_WRITER_HPP
#define PPM_WRITER_HPP

#include <cstdint>
#include <string>
#include <vector>

class PPMWriter {
public:
  static bool write_ppm(std::string const & filename, std::vector<uint8_t> const & r_channel,
                        std::vector<uint8_t> const & g_channel,
                        std::vector<uint8_t> const & b_channel, size_t width, size_t height);
};

#endif
