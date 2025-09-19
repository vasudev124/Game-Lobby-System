#include "room_manager.hpp"
#include <algorithm>
#include <random>
#include <sstream>
#include <iostream>

RoomManager::RoomManager(std::shared_ptr<DatabaseManager> db) : dbManager(db) {
    // Load existing rooms and users from database
    // This would be implemented with proper database queries
}

std::string RoomManager::createRoom(const std::string& name, const std::string& creatorId,
                                   const std::string& gameType) {
    std::lock_guard<std::mutex> lock(roomsMutex);

    std::string roomId = generateRoomId();
    Room room(roomId, name, creatorId, gameType);
    room.players.push_back(creatorId);

    rooms[roomId] = room;

    // Update user's current room
    {
        std::lock_guard<std::mutex> userLock(usersMutex);
        if (users.find(creatorId) != users.end()) {
            users[creatorId].currentRoom = roomId;
            dbManager->updateUser(users[creatorId]);
        }
    }

    // Save to database
    dbManager->insertRoom(room);

    notifyRoomUpdate(roomId);
    return roomId;
}

bool RoomManager::joinRoom(const std::string& roomId, const std::string& userId) {
    std::lock_guard<std::mutex> lock(roomsMutex);

    auto roomIt = rooms.find(roomId);
    if (roomIt == rooms.end()) {
        return false;
    }

    Room& room = roomIt->second;
    if (room.isFull() || room.hasPlayer(userId)) {
        return false;
    }

    room.players.push_back(userId);

    // Update user's current room
    {
        std::lock_guard<std::mutex> userLock(usersMutex);
        if (users.find(userId) != users.end()) {
            users[userId].currentRoom = roomId;
            dbManager->updateUser(users[userId]);
        }
    }

    // Update database
    dbManager->updateRoom(room);

    notifyRoomUpdate(roomId);
    return true;
}

bool RoomManager::leaveRoom(const std::string& roomId, const std::string& userId) {
    std::lock_guard<std::mutex> lock(roomsMutex);

    auto roomIt = rooms.find(roomId);
    if (roomIt == rooms.end()) {
        return false;
    }

    Room& room = roomIt->second;
    auto playerIt = std::find(room.players.begin(), room.players.end(), userId);
    if (playerIt == room.players.end()) {
        return false;
    }

    room.players.erase(playerIt);

    // Update user's current room
    {
        std::lock_guard<std::mutex> userLock(usersMutex);
        if (users.find(userId) != users.end()) {
            users[userId].currentRoom = "";
            dbManager->updateUser(users[userId]);
        }
    }

    // If room is empty, delete it
    if (room.players.empty()) {
        dbManager->deleteRoom(roomId);
        rooms.erase(roomIt);
        return true;
    }

    // Update database
    dbManager->updateRoom(room);

    notifyRoomUpdate(roomId);
    return true;
}

bool RoomManager::addUser(const User& user) {
    std::lock_guard<std::mutex> lock(usersMutex);
    users[user.id] = user;
    dbManager->insertUser(user);
    notifyUserUpdate(user.id);
    return true;
}

bool RoomManager::removeUser(const std::string& userId) {
    // First remove user from any rooms they're in
    {
        std::lock_guard<std::mutex> roomLock(roomsMutex);
        for (auto& [roomId, room] : rooms) {
            auto playerIt = std::find(room.players.begin(), room.players.end(), userId);
            if (playerIt != room.players.end()) {
                room.players.erase(playerIt);
                if (room.players.empty()) {
                    dbManager->deleteRoom(roomId);
                    rooms.erase(roomId);
                    break;
                } else {
                    dbManager->updateRoom(room);
                }
                notifyRoomUpdate(roomId);
            }
        }
    }

    std::lock_guard<std::mutex> lock(usersMutex);
    users.erase(userId);
    dbManager->deleteUser(userId);
    return true;
}

bool RoomManager::sendChatMessage(const std::string& roomId, const std::string& userId,
                                 const std::string& message) {
    std::lock_guard<std::mutex> userLock(usersMutex);
    auto userIt = users.find(userId);
    if (userIt == users.end()) {
        return false;
    }

    const User& user = userIt->second;
    return dbManager->insertChatMessage(roomId, userId, user.username, message);
}

std::vector<Room> RoomManager::getAllRooms() {
    std::lock_guard<std::mutex> lock(roomsMutex);
    std::vector<Room> result;
    for (const auto& [id, room] : rooms) {
        result.push_back(room);
    }
    return result;
}

std::vector<User> RoomManager::getOnlineUsers() {
    std::lock_guard<std::mutex> lock(usersMutex);
    std::vector<User> result;
    for (const auto& [id, user] : users) {
        if (user.isOnline) {
            result.push_back(user);
        }
    }
    return result;
}

std::string RoomManager::generateRoomId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    return "room_" + std::to_string(dis(gen));
}

void RoomManager::notifyRoomUpdate(const std::string& roomId) {
    if (onRoomUpdate) {
        onRoomUpdate(roomId, "room_updated");
    }
}

void RoomManager::notifyUserUpdate(const std::string& userId) {
    if (onUserUpdate) {
        onUserUpdate(userId, "user_updated");
    }
}
