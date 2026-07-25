#pragma once

#include "client/Scene.hpp"

#include <functional>
#include <string>

namespace game {

class ConnectScene : public Scene {
public:
    using ConnectCallback = std::function<void(const std::string& ip)>;
    using BackCallback = std::function<void()>;

    ConnectScene(ConnectCallback onConnect, BackCallback onBack);

    void onEnter() override;
    void renderImGui() override;

private:
    ConnectCallback onConnect_;
    BackCallback onBack_;
    std::string errorMessage_;
    char ipBuffer_[32] = "127.0.0.1";
};

} // namespace game
