#include "database_manager.hpp"
#include <iostream>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/exception.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

DatabaseManager::DatabaseManager(const std::string& connectionString) {
    try {
        client = std::make_unique<mongocxx::client>(mongocxx::uri{connectionString});
        db = client->database("game_lobby");
        std::cout << "Connected to MongoDB successfully" << std::endl;
    } catch (const mongocxx::exception& e) {
        std::cerr << "MongoDB connection error: " << e.what() << std::endl;
        throw;
    }
}

DatabaseManager::~DatabaseManager() = default;

bool DatabaseManager::insertUser(const User& user) {
    try {
        auto collection = db.collection("users");
        auto doc = document{}
            << "id" << user.id
            << "username" << user.username
            << "currentRoom" << user.currentRoom
            << "isOnline" << user.isOnline
            << "lastActivity" << bsoncxx::types::b_date{user.lastActivity}
            << finalize;

        auto result = collection.insert_one(doc.view());
        return result.has_value();
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error inserting user: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::updateUser(const User& user) {
    try {
        auto collection = db.collection("users");
        auto filter = document{} << "id" << user.id << finalize;
        auto update = document{}
            << "$set" << bsoncxx::builder::stream::open_document
            << "username" << user.username
            << "currentRoom" << user.currentRoom
            << "isOnline" << user.isOnline
            << "lastActivity" << bsoncxx::types::b_date{user.lastActivity}
            << bsoncxx::builder::stream::close_document
            << finalize;

        auto result = collection.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error updating user: " << e.what() << std::endl;
        return false;
    }
}

User DatabaseManager::getUserById(const std::string& userId) {
    try {
        auto collection = db.collection("users");
        auto filter = document{} << "id" << userId << finalize;
        auto result = collection.find_one(filter.view());

        if (result) {
            User user;
            auto view = result->view();
            user.id = view["id"].get_utf8().value.to_string();
            user.username = view["username"].get_utf8().value.to_string();
            user.currentRoom = view["currentRoom"].get_utf8().value.to_string();
            user.isOnline = view["isOnline"].get_bool();
            user.lastActivity = view["lastActivity"].get_date();
            return user;
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error getting user: " << e.what() << std::endl;
    }
    return User{};
}

bool DatabaseManager::insertRoom(const Room& room) {
    try {
        auto collection = db.collection("rooms");
        auto doc = document{}
            << "id" << room.id
            << "name" << room.name
            << "gameType" << room.gameType
            << "createdBy" << room.createdBy
            << "maxPlayers" << room.maxPlayers
            << "status" << static_cast<int>(room.status)
            << "createdAt" << bsoncxx::types::b_date{room.createdAt}
            << finalize;

        auto result = collection.insert_one(doc.view());
        return result.has_value();
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error inserting room: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::insertChatMessage(const std::string& roomId, const std::string& userId,
                                       const std::string& username, const std::string& message) {
    try {
        auto collection = db.collection("chat_messages");
        auto doc = document{}
            << "roomId" << roomId
            << "userId" << userId
            << "username" << username
            << "message" << message
            << "timestamp" << bsoncxx::types::b_date{std::chrono::system_clock::now()}
            << finalize;

        auto result = collection.insert_one(doc.view());
        return result.has_value();
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error inserting chat message: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::isConnected() const {
    try {
        client->database("admin").run_command(document{} << "ping" << 1 << finalize);
        return true;
    } catch (const mongocxx::exception&) {
        return false;
    }
}
