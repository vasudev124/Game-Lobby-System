import React from 'react';

function RoomList({ rooms, currentUser, currentRoom, onJoinRoom, onLeaveRoom }) {
  const handleRoomAction = (room) => {
    if (currentRoom === room.id) {
      onLeaveRoom(room.id);
    } else {
      onJoinRoom(room.id);
    }
  };

  const getRoomStatus = (room) => {
    switch (room.status) {
      case 0: return 'Waiting';
      case 1: return 'In Progress';
      case 2: return 'Finished';
      default: return 'Unknown';
    }
  };

  const getStatusClass = (room) => {
    switch (room.status) {
      case 0: return 'waiting';
      case 1: return 'in-progress';
      case 2: return 'finished';
      default: return 'unknown';
    }
  };

  const isUserInRoom = (room) => {
    return room.players && room.players.includes(currentUser.id);
  };

  const canJoinRoom = (room) => {
    return room.status === 0 && room.players.length < room.maxPlayers;
  };

  return (
    <div className="room-list">
      {rooms.length === 0 ? (
        <div className="room-list-empty">
          <p>No active rooms</p>
          <p className="room-list-hint">Create a room to get started!</p>
        </div>
      ) : (
        rooms.map((room) => (
          <div 
            key={room.id} 
            className={`room-item ${currentRoom === room.id ? 'current-room' : ''} ${isUserInRoom(room) ? 'joined' : ''}`}
          >
            <div className="room-header">
              <h4 className="room-name">{room.name}</h4>
              <span className={`room-status ${getStatusClass(room)}`}>
                {getRoomStatus(room)}
              </span>
            </div>

            <div className="room-info">
              <div className="room-details">
                <span className="game-type">Game: {room.gameType}</span>
                <span className="player-count">
                  Players: {room.players ? room.players.length : 0}/{room.maxPlayers}
                </span>
              </div>

              <div className="room-actions">
                {isUserInRoom(room) ? (
                  <button 
                    className="room-action-btn leave-btn"
                    onClick={() => handleRoomAction(room)}
                  >
                    {currentRoom === room.id ? 'Leave' : 'Leave Room'}
                  </button>
                ) : canJoinRoom(room) ? (
                  <button 
                    className="room-action-btn join-btn"
                    onClick={() => handleRoomAction(room)}
                  >
                    Join
                  </button>
                ) : (
                  <button 
                    className="room-action-btn disabled"
                    disabled
                  >
                    {room.players.length >= room.maxPlayers ? 'Full' : 'Unavailable'}
                  </button>
                )}
              </div>
            </div>

            {room.players && room.players.length > 0 && (
              <div className="room-players">
                <div className="players-label">Players:</div>
                <div className="players-list">
                  {room.players.map((playerId, index) => (
                    <span key={playerId} className="player-name">
                      {playerId === currentUser.id ? 'You' : `Player ${index + 1}`}
                    </span>
                  ))}
                </div>
              </div>
            )}
          </div>
        ))
      )}
    </div>
  );
}

export default RoomList;
