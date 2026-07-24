#include "client/ClientApplication.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        game::ClientApplication app;
        return app.run();
    } catch (const std::exception& error) {
        std::cerr << "Fatal client error: " << error.what() << '\n';
        return 1;
    }
}
