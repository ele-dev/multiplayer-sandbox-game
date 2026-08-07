#pragma once

#include "client/Layer.hpp"

#include <functional>
#include <string>

namespace game {

class MainMenuLayer : public Layer {
public:
    using ConnectCallback = std::function<void(const std::string& ip)>;
    using ExitCallback = std::function<void()>;

    MainMenuLayer(ConnectCallback onConnect, ExitCallback onExit);

    void onAttach() override;
    void onRender() override;

private:
    enum class Page {
        Start,
        Connect
    };

    void renderStartPage();
    void renderConnectPage();
    void resetConnectPage();

    Page activePage_ = Page::Start;
    ConnectCallback onConnect_;
    ExitCallback onExit_;
    std::string errorMessage_;
    char ipBuffer_[32] = "127.0.0.1";
};

} // namespace game
