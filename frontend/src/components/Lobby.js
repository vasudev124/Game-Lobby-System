import React, { useState, useEffect } from 'react';
import RoomList from './RoomList';
import UserList from './UserList';
import Chat from './Chat';
import CreateRoom from './CreateRoom';
import WebSocketService from '../services/WebSocketService';

function Lobby({ user }) {
  const [rooms, setRooms] = useState([]);
  const [users, setUsers] = useState([]);
  const [currentRoom, setCurrentRoom] = useState(null);
  const [showCreateRoom, setShowCreateRoom] = useState(false);
  const [chatMessages, setChatMessages] = useState([]);

  useEffect(() => {
    // Request initial data
    WebSocketService.getRooms();
    WebSocketService.getUsers();

    // Set up event handlers for real-time updates
    WebSocketService.on('room_update', handleRoomUpdate);
    WebSocketService.on('user_update', handleUserUpdate);
    WebSocketService.on('chat_message', handleChatMessage);
    WebSocketService.on('room_created', handleRoomCreated);
    WebSocketService.on('room_joined', handleRoomJoined);
    WebSocketService.on('room_left', handleRoomLeft);

    return () => {
      WebSocketService.off('room_update', handleRoomUpdate);
      WebSocketService.off('user_update', handleUserUpdate);
      WebSocketService.off('chat_message', handleChatMessage);
      WebSocketService.off('room_created', handleRoomCreated);
      WebSocketService.off('room_joined', handleRoomJoined);
      WebSocketService.off('room_left', handleRoomLeft);
    };
  }, []);

  const handleRoomUpdate = (data) => {
    if (data.rooms) {
      setRooms(data.rooms);
    } else if (data.room) {
      setRooms(prevRooms => {
        const updatedRooms = [...prevRooms];
        const index = updatedRooms.findIndex(room => room.id === data.room.id);
        if (index >= 0) {
          updatedRooms[index] = data.room;
        } else {
          updatedRooms.push(data.room);
        }
        return updatedRooms;
      });
    }
  };

  const handleUserUpdate = (data) => {
    if (data.users) {
      setUsers(data.users);
    }
  };

  const handleChatMessage = (data) => {
    setChatMessages(prevMessages => [...prevMessages, {
      id: Date.now(),
      username: data.username,
      message: data.message,
      timestamp: new Date().toLocaleTimeString(),
      roomId: data.roomId
    }]);
  };

  const handleRoomCreated = (data) => {
    console.log('Room created:', data.roomId);
    setShowCreateRoom(false);
    WebSocketService.getRooms();
  };

  const handleRoomJoined = (data) => {
    console.log('Joined room:', data.roomId);
    setCurrentRoom(data.roomId);
  };

  const handleRoomLeft = (data) => {
    console.log('Left room:', data.roomId);
    setCurrentRoom(null);
    setChatMessages([]);
  };

  const handleJoinRoom = (roomId) => {
    WebSocketService.joinRoom(roomId);
  };

  const handleLeaveRoom = () => {
    if (currentRoom) {
      WebSocketService.leaveRoom(currentRoom);
    }
  };

  const handleCreateRoom = (roomData) => {
    WebSocketService.createRoom(roomData.name, roomData.gameType);
  };

  const handleSendMessage = (message) => {
    if (currentRoom) {
      WebSocketService.sendChatMessage(currentRoom, message);
    }
  };

  const getCurrentRoomMessages = () => {
    return currentRoom ? 
      chatMessages.filter(msg => msg.roomId === currentRoom) : 
      [];
  };

  return (
    <div className="lobby">
      <div className="lobby-sidebar">
        <div className="sidebar-section">
          <h3>Online Users ({users.length})</h3>
          <UserList users={users} currentUser={user} />
        </div>

        <div className="sidebar-section">
          <div className="rooms-header">
            <h3>Game Rooms ({rooms.length})</h3>
            <button 
              className="create-room-btn"
              onClick={() => setShowCreateRoom(true)}
            >
              Create Room
            </button>
          </div>
          <RoomList 
            rooms={rooms} 
            currentUser={user}
            currentRoom={currentRoom}
            onJoinRoom={handleJoinRoom}
            onLeaveRoom={handleLeaveRoom}
          />
        </div>
      </div>

      <div className="lobby-main">
        {currentRoom ? (
          <div className="room-view">
            <div className="room-header">
              <h2>Room: {rooms.find(r => r.id === currentRoom)?.name || currentRoom}</h2>
              <button className="leave-room-btn" onClick={handleLeaveRoom}>
                Leave Room
              </button>
            </div>
            <Chat 
              messages={getCurrentRoomMessages()}
              onSendMessage={handleSendMessage}
              currentUser={user}
            />
          </div>
        ) : (
          <div className="lobby-welcome">
            <h2>Welcome to the Game Lobby!</h2>
            <p>Join a room to start chatting and playing games.</p>
            <div className="lobby-stats">
              <div className="stat">
                <strong>{users.length}</strong>
                <span>Online Players</span>
              </div>
              <div className="stat">
                <strong>{rooms.length}</strong>
                <span>Active Rooms</span>
              </div>
            </div>
          </div>
        )}
      </div>

      {showCreateRoom && (
        <CreateRoom 
          onCreateRoom={handleCreateRoom}
          onCancel={() => setShowCreateRoom(false)}
        />
      )}
    </div>
  );
}

export default Lobby;
