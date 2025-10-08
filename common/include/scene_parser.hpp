#pragma once

#include <string>
#include "dataStructs/settings_structs.hpp"

[[nodiscard]] SceneSettings loadSceneFromFile(const std::string &filename);