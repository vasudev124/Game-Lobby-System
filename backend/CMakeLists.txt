cmake_minimum_required(VERSION 3.10)
project(GameLobbyServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# Find MongoDB C++ driver
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# Find JsonCpp
pkg_check_modules(JSONCPP jsoncpp)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/main.cpp
    src/websocket_server.cpp
    src/room_manager.cpp
    src/database_manager.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    ${JSONCPP_LIBRARIES}
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
    pthread
)

# Compiler flags
target_compile_definitions(${PROJECT_NAME} PRIVATE _WEBSOCKETPP_CPP11_STL_)

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
