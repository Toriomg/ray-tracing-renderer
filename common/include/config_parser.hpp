#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "dataStructs/settings_structs.hpp"
#include <string>

[[nodiscard]] std::optional<ConfigSettings> loadConfigFromFile(std::string const & filename);

#endif
