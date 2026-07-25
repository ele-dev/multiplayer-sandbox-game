#pragma once

#include <string>
#include <unordered_map>

namespace game {

class Config {
public:
    bool loadFile(const std::string& path);

    std::string getString(const std::string& key, const std::string& defaultVal) const;
    int getInt(const std::string& key, int defaultVal) const;
    float getFloat(const std::string& key, float defaultVal) const;

    bool contains(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> values_;
};

} // namespace game
