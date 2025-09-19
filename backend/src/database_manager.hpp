#pragma once
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <memory>
#include "user.hpp"
#include "room.hpp"

class DatabaseManager {
private:
    mongocxx::instance instance{};
    std::unique_ptr<mongocxx::client> client;
    mongocxx::database db;

public:
    DatabaseManager(const std::string& connectionString = "mongodb://localhost:27017");
    ~DatabaseManager();

    // User operations
    bool insertUser(const User& user);
    bool updateUser(const User& user);
    User getUserById(const std::string& userId);
    std::vector<User> getOnlineUsers();
    bool deleteUser(const std::string& userId);

    // Room operations
    bool insertRoom(const Room& room);
    bool updateRoom(const Room& room);
    Room getRoomById(const std::string& roomId);
    std::vector<Room> getAvailableRooms();
    bool deleteRoom(const std::string& roomId);

    // Chat operations
    bool insertChatMessage(const std::string& roomId, const std::string& userId, 
                          const std::string& username, const std::string& message);
    std::vector<std::string> getChatHistory(const std::string& roomId, int limit = 50);

    bool isConnected() const;
};
