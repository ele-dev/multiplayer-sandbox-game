#include "shared/Config.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

namespace game {

namespace {

void trim(std::string& s) {
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        s.clear();
        return;
    }
    const auto end = s.find_last_not_of(" \t\r\n");
    s = s.substr(start, end - start + 1);
}

} // namespace

bool Config::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        ++lineNum;

        trim(line);

        if (line.empty() || line[0] == '#' || line.starts_with("//")) {
            continue;
        }

        const auto eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            std::cerr << "Config: " << path << ":" << lineNum << " - missing '='\n";
            continue;
        }

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        trim(key);
        trim(value);

        if (key.empty()) {
            std::cerr << "Config: " << path << ":" << lineNum << " - empty key\n";
            continue;
        }

        values_[key] = value;
    }

    return true;
}

std::string Config::getString(const std::string& key, const std::string& defaultVal) const {
    const auto it = values_.find(key);
    return it != values_.end() ? it->second : defaultVal;
}

int Config::getInt(const std::string& key, int defaultVal) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return defaultVal;
    }
    try {
        return std::stoi(it->second);
    } catch (...) {
        std::cerr << "Config: key '" << key << "' has non-integer value '" << it->second << "'\n";
    }
    return defaultVal;
}

float Config::getFloat(const std::string& key, float defaultVal) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return defaultVal;
    }
    try {
        return std::stof(it->second);
    } catch (...) {
        std::cerr << "Config: key '" << key << "' has non-float value '" << it->second << "'\n";
    }
    return defaultVal;
}

bool Config::contains(const std::string& key) const {
    return values_.find(key) != values_.end();
}

} // namespace game
