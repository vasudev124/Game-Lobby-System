#pragma once
#include <string>
#include <vector>
#include <chrono>

enum class RoomStatus {
    WAITING,
    IN_PROGRESS,
    FINISHED
};

struct Room {
    std::string id;
    std::string name;
    std::string gameType;
    std::vector<std::string> players;
    std::string createdBy;
    int maxPlayers;
    RoomStatus status;
    std::chrono::system_clock::time_point createdAt;

    Room() : maxPlayers(4), status(RoomStatus::WAITING) {}
    Room(const std::string& roomId, const std::string& roomName, 
         const std::string& creator, const std::string& game = "Generic")
        : id(roomId), name(roomName), createdBy(creator), gameType(game),
          maxPlayers(4), status(RoomStatus::WAITING),
          createdAt(std::chrono::system_clock::now()) {}

    bool isFull() const { return players.size() >= maxPlayers; }
    bool hasPlayer(const std::string& userId) const {
        return std::find(players.begin(), players.end(), userId) != players.end();
    }
};
