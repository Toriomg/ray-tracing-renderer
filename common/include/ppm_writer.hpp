#pragma once
#include <string>
#include <vector>

class PPMWriter {
public:
    static bool write_ppm(const std::string& filename,
                         const std::vector<uint8_t>& r_channel,
                         const std::vector<uint8_t>& g_channel,
                         const std::vector<uint8_t>& b_channel,
                         size_t width, size_t height);
};