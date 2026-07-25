#include "client/GuiLayer.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

namespace game {

GuiLayer::~GuiLayer() {
    shutdown();
}

bool GuiLayer::initialize(SDL_Window* window, SDL_GLContext glContext) {
    if (initialized_) {
        return true;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 1.0f;

    if (!ImGui_ImplSDL3_InitForOpenGL(window, glContext)) {
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    initialized_ = true;
    return true;
}

void GuiLayer::processEvent(const SDL_Event& event) {
    if (!initialized_) {
        return;
    }
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void GuiLayer::beginFrame() {
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void GuiLayer::endFrame() {
    if (!initialized_) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiLayer::shutdown() {
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

} // namespace game
