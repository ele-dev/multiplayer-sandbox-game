#pragma once

#include "client/ClientSettings.hpp"
#include "client/Layer.hpp"

#include <functional>
#include <string>

namespace game {

class MainMenuLayer : public Layer {
public:
    using ConnectCallback = std::function<void(const std::string& ip)>;
    using ExitCallback = std::function<void()>;

    MainMenuLayer(ConnectCallback onConnect, ExitCallback onExit, ClientSettings& settings);

    void onAttach() override;
    void onRender() override;
    void navigateToSettings();

private:
    enum class Page {
        Start,
        Connect,
        Settings
    };

    void renderStartPage();
    void renderConnectPage();
    void renderSettingsPage();
    void resetConnectPage();

    Page activePage_ = Page::Start;
    ConnectCallback onConnect_;
    ExitCallback onExit_;
    ClientSettings& settings_;
    std::string errorMessage_;
    char ipBuffer_[32] = "127.0.0.1";
};

} // namespace game
