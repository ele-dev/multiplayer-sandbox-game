#pragma once

#include "shared/Config.hpp"
#include <string>

namespace game {

class ClientSettings {
public:
    float mouseSensitivity = 0.0025f;
    float audioVolume = 1.0f;
    bool fullscreen = false;

    void load(const std::string& path);
    void save(const std::string& path) const;
};

} // namespace game
