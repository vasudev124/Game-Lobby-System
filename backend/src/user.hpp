#pragma once
#include <string>
#include <chrono>

struct User {
    std::string id;
    std::string username;
    std::string currentRoom;
    bool isOnline;
    std::chrono::system_clock::time_point lastActivity;

    User() : isOnline(false) {}
    User(const std::string& userId, const std::string& name) 
        : id(userId), username(name), isOnline(true), 
          lastActivity(std::chrono::system_clock::now()) {}
};
