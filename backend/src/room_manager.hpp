#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <functional>
#include "room.hpp"
#include "user.hpp"
#include "database_manager.hpp"

class RoomManager {
private:
    std::unordered_map<std::string, Room> rooms;
    std::unordered_map<std::string, User> users;
    std::shared_ptr<DatabaseManager> dbManager;
    mutable std::mutex roomsMutex;
    mutable std::mutex usersMutex;

public:
    using MessageCallback = std::function<void(const std::string&, const std::string&)>;
    MessageCallback onRoomUpdate;
    MessageCallback onUserUpdate;

    RoomManager(std::shared_ptr<DatabaseManager> db);

    // Room operations
    std::string createRoom(const std::string& name, const std::string& creatorId, 
                          const std::string& gameType = "Generic");
    bool joinRoom(const std::string& roomId, const std::string& userId);
    bool leaveRoom(const std::string& roomId, const std::string& userId);
    bool deleteRoom(const std::string& roomId);
    Room getRoomById(const std::string& roomId);
    std::vector<Room> getAllRooms();

    // User operations
    bool addUser(const User& user);
    bool removeUser(const std::string& userId);
    bool updateUserActivity(const std::string& userId);
    User getUserById(const std::string& userId);
    std::vector<User> getOnlineUsers();

    // Chat operations
    bool sendChatMessage(const std::string& roomId, const std::string& userId, 
                        const std::string& message);
    std::vector<std::string> getChatHistory(const std::string& roomId);

    // Matchmaking
    std::vector<Room> findAvailableRooms(const std::string& gameType = "");
    std::string findOrCreateRoom(const std::string& userId, const std::string& gameType = "Generic");

private:
    std::string generateRoomId();
    void notifyRoomUpdate(const std::string& roomId);
    void notifyUserUpdate(const std::string& userId);
    void cleanupInactiveUsers();
};
