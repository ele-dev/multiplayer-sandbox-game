#pragma once

#include "client/Layer.hpp"

#include <cstdint>
#include <string>

namespace game {

class ConnectingLayer : public Layer {
public:
    ConnectingLayer(const std::string& host, std::uint16_t port);

    void onRender() override;

private:
    std::string host_;
    std::uint16_t port_;
};

} // namespace game
