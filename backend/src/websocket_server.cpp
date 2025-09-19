#include "websocket_server.hpp"
#include <iostream>
#include <json/json.h>

WebSocketServer::WebSocketServer(int port) : isRunning(false) {
    // Initialize WebSocket++ server
    wsServer.set_access_channels(websocketpp::log::alevel::all);
    wsServer.clear_access_channels(websocketpp::log::alevel::frame_payload);
    wsServer.init_asio();

    // Set handlers
    wsServer.set_validate_handler(std::bind(&WebSocketServer::onValidate, this, std::placeholders::_1));
    wsServer.set_open_handler(std::bind(&WebSocketServer::onOpen, this, std::placeholders::_1));
    wsServer.set_close_handler(std::bind(&WebSocketServer::onClose, this, std::placeholders::_1));
    wsServer.set_message_handler(std::bind(&WebSocketServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));

    // Initialize managers
    dbManager = std::make_shared<DatabaseManager>();
    roomManager = std::make_shared<RoomManager>(dbManager);

    // Set up room manager callbacks
    roomManager->onRoomUpdate = [this](const std::string& roomId, const std::string& type) {
        Room room = roomManager->getRoomById(roomId);
        Json::Value roomData;
        roomData["type"] = "room_update";
        roomData["room"]["id"] = room.id;
        roomData["room"]["name"] = room.name;
        roomData["room"]["gameType"] = room.gameType;
        roomData["room"]["players"] = Json::Value(Json::arrayValue);
        for (const auto& playerId : room.players) {
            roomData["room"]["players"].append(playerId);
        }
        roomData["room"]["maxPlayers"] = room.maxPlayers;
        roomData["room"]["status"] = static_cast<int>(room.status);

        Json::StreamWriterBuilder builder;
        std::string message = Json::writeString(builder, roomData);
        broadcastToAll(message);
    };

    wsServer.listen(port);
    wsServer.start_accept();

    std::cout << "WebSocket server initialized on port " << port << std::endl;
}

WebSocketServer::~WebSocketServer() {
    stop();
}

void WebSocketServer::start() {
    if (!isRunning) {
        isRunning = true;
        serverThread = std::thread([this]() {
            try {
                wsServer.run();
            } catch (const std::exception& e) {
                std::cerr << "WebSocket server error: " << e.what() << std::endl;
            }
        });
        std::cout << "WebSocket server started" << std::endl;
    }
}

void WebSocketServer::stop() {
    if (isRunning) {
        isRunning = false;
        wsServer.stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
        std::cout << "WebSocket server stopped" << std::endl;
    }
}

void WebSocketServer::onOpen(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connectionsMutex);
    connections[hdl] = ""; // User ID will be set during authentication
    std::cout << "New WebSocket connection opened" << std::endl;
}

void WebSocketServer::onClose(connection_hdl hdl) {
    cleanupConnection(hdl);
    std::cout << "WebSocket connection closed" << std::endl;
}

void WebSocketServer::onMessage(connection_hdl hdl, message_ptr msg) {
    try {
        std::string payload = msg->get_payload();
        processMessage(hdl, payload);
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;

        std::string errorResponse = createJsonResponse("error", "", false, e.what());
        wsServer.send(hdl, errorResponse, websocketpp::frame::opcode::text);
    }
}

bool WebSocketServer::onValidate(connection_hdl hdl) {
    return true; // Accept all connections for now
}

void WebSocketServer::processMessage(connection_hdl hdl, const std::string& message) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    std::string errors;

    if (!reader->parse(message.c_str(), message.c_str() + message.length(), &root, &errors)) {
        delete reader;
        throw std::runtime_error("Invalid JSON format");
    }
    delete reader;

    std::string type = root.get("type", "").asString();
    std::string data = root.get("data", "").asString();

    if (type == "auth") {
        handleUserAuth(hdl, data);
    } else if (type == "create_room") {
        handleCreateRoom(hdl, data);
    } else if (type == "join_room") {
        handleJoinRoom(hdl, data);
    } else if (type == "leave_room") {
        handleLeaveRoom(hdl, data);
    } else if (type == "chat_message") {
        handleChatMessage(hdl, data);
    } else if (type == "get_rooms") {
        handleGetRooms(hdl);
    } else if (type == "get_users") {
        handleGetUsers(hdl);
    } else {
        throw std::runtime_error("Unknown message type: " + type);
    }
}

void WebSocketServer::handleUserAuth(connection_hdl hdl, const std::string& data) {
    Json::Value authData;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    std::string errors;

    if (!reader->parse(data.c_str(), data.c_str() + data.length(), &authData, &errors)) {
        delete reader;
        throw std::runtime_error("Invalid auth data");
    }
    delete reader;

    std::string userId = authData.get("userId", "").asString();
    std::string username = authData.get("username", "").asString();

    if (userId.empty() || username.empty()) {
        throw std::runtime_error("Missing user credentials");
    }

    // Create or update user
    User user(userId, username);
    roomManager->addUser(user);

    // Associate connection with user
    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections[hdl] = userId;
        userConnections[userId] = hdl;
    }

    Json::Value response;
    response["type"] = "auth_success";
    response["user"]["id"] = userId;
    response["user"]["username"] = username;

    Json::StreamWriterBuilder writerBuilder;
    std::string responseStr = Json::writeString(writerBuilder, response);
    wsServer.send(hdl, responseStr, websocketpp::frame::opcode::text);
}

void WebSocketServer::handleCreateRoom(connection_hdl hdl, const std::string& data) {
    std::string userId = getUserId(hdl);
    if (userId.empty()) {
        throw std::runtime_error("User not authenticated");
    }

    Json::Value roomData;
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    std::string errors;

    if (!reader->parse(data.c_str(), data.c_str() + data.length(), &roomData, &errors)) {
        delete reader;
        throw std::runtime_error("Invalid room data");
    }
    delete reader;

    std::string roomName = roomData.get("name", "").asString();
    std::string gameType = roomData.get("gameType", "Generic").asString();

    std::string roomId = roomManager->createRoom(roomName, userId, gameType);

    Json::Value response;
    response["type"] = "room_created";
    response["roomId"] = roomId;

    Json::StreamWriterBuilder writerBuilder;
    std::string responseStr = Json::writeString(writerBuilder, response);
    wsServer.send(hdl, responseStr, websocketpp::frame::opcode::text);
}

void WebSocketServer::broadcastToAll(const std::string& message) {
    std::lock_guard<std::mutex> lock(connectionsMutex);
    for (const auto& [hdl, userId] : connections) {
        try {
            wsServer.send(hdl, message, websocketpp::frame::opcode::text);
        } catch (const std::exception& e) {
            std::cerr << "Error broadcasting message: " << e.what() << std::endl;
        }
    }
}

std::string WebSocketServer::getUserId(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connectionsMutex);
    auto it = connections.find(hdl);
    return (it != connections.end()) ? it->second : "";
}

void WebSocketServer::cleanupConnection(connection_hdl hdl) {
    std::string userId = getUserId(hdl);

    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.erase(hdl);
        if (!userId.empty()) {
            userConnections.erase(userId);
        }
    }

    if (!userId.empty()) {
        roomManager->removeUser(userId);
    }
}

std::string WebSocketServer::createJsonResponse(const std::string& type, const std::string& data,
                                               bool success, const std::string& error) {
    Json::Value response;
    response["type"] = type;
    response["success"] = success;
    if (!data.empty()) {
        response["data"] = data;
    }
    if (!error.empty()) {
        response["error"] = error;
    }

    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, response);
}
