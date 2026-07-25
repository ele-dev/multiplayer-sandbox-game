#pragma once

#include "client/Layer.hpp"

#include <functional>
#include <string>

namespace game {

class ConnectLayer : public Layer {
public:
    using ConnectCallback = std::function<void(const std::string& ip)>;
    using BackCallback = std::function<void()>;

    ConnectLayer(ConnectCallback onConnect, BackCallback onBack);

    void onAttach() override;
    void onRender() override;

private:
    ConnectCallback onConnect_;
    BackCallback onBack_;
    std::string errorMessage_;
    char ipBuffer_[32] = "127.0.0.1";
};

} // namespace game
