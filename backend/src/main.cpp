#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>
#include "websocket_server.hpp"

std::unique_ptr<WebSocketServer> server;

void signalHandler(int signal) {
    std::cout << "
Shutting down server gracefully..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // Create and start the WebSocket server
        int port = (argc > 1) ? std::atoi(argv[1]) : 9002;
        server = std::make_unique<WebSocketServer>(port);

        std::cout << "Starting Game Lobby Server..." << std::endl;
        std::cout << "WebSocket server listening on port " << port << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;

        server->start();

        // Keep the main thread alive
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
