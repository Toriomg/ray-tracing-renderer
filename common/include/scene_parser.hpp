#ifndef SCENE_PARSER_HPP
#define SCENE_PARSER_HPP

#include "dataStructs/settings_structs.hpp"
#include <string>

[[nodiscard]] SceneSettings loadSceneFromFile(std::string const & filename);

#endif
