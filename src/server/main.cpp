#include "server/ServerApplication.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        game::ServerApplication app;
        return app.run();
    } catch (const std::exception& error) {
        std::cerr << "Fatal server error: " << error.what() << '\n';
        return 1;
    }
}
