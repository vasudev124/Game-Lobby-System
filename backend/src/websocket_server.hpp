#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include "room_manager.hpp"
#include "database_manager.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;
using websocketpp::connection_hdl;

class WebSocketServer {
private:
    server wsServer;
    std::shared_ptr<RoomManager> roomManager;
    std::shared_ptr<DatabaseManager> dbManager;

    std::unordered_map<connection_hdl, std::string, std::owner_less<connection_hdl>> connections;
    std::unordered_map<std::string, connection_hdl> userConnections;

    mutable std::mutex connectionsMutex;
    std::thread serverThread;
    bool isRunning;

public:
    WebSocketServer(int port = 9002);
    ~WebSocketServer();

    void start();
    void stop();

    // Message broadcasting
    void broadcastToRoom(const std::string& roomId, const std::string& message);
    void sendToUser(const std::string& userId, const std::string& message);
    void broadcastToAll(const std::string& message);

private:
    // WebSocket event handlers
    void onOpen(connection_hdl hdl);
    void onClose(connection_hdl hdl);
    void onMessage(connection_hdl hdl, message_ptr msg);
    bool onValidate(connection_hdl hdl);

    // Message processing
    void processMessage(connection_hdl hdl, const std::string& message);
    void handleUserAuth(connection_hdl hdl, const std::string& data);
    void handleCreateRoom(connection_hdl hdl, const std::string& data);
    void handleJoinRoom(connection_hdl hdl, const std::string& data);
    void handleLeaveRoom(connection_hdl hdl, const std::string& data);
    void handleChatMessage(connection_hdl hdl, const std::string& data);
    void handleGetRooms(connection_hdl hdl);
    void handleGetUsers(connection_hdl hdl);

    // Utility functions
    std::string getUserId(connection_hdl hdl);
    void cleanupConnection(connection_hdl hdl);
    std::string createJsonResponse(const std::string& type, const std::string& data, 
                                   bool success = true, const std::string& error = "");
};
