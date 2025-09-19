import React, { useState } from 'react';

function CreateRoom({ onCreateRoom, onCancel }) {
  const [roomData, setRoomData] = useState({
    name: '',
    gameType: 'Generic',
    maxPlayers: 4
  });
  const [errors, setErrors] = useState({});

  const gameTypes = [
    'Generic',
    'Strategy',
    'Action',
    'Puzzle',
    'RPG',
    'Adventure',
    'Racing',
    'Sports',
    'Card Game',
    'Board Game'
  ];

  const handleInputChange = (e) => {
    const { name, value } = e.target;
    setRoomData(prev => ({
      ...prev,
      [name]: value
    }));

    // Clear error when user starts typing
    if (errors[name]) {
      setErrors(prev => ({
        ...prev,
        [name]: ''
      }));
    }
  };

  const validateForm = () => {
    const newErrors = {};

    if (!roomData.name.trim()) {
      newErrors.name = 'Room name is required';
    } else if (roomData.name.length < 3) {
      newErrors.name = 'Room name must be at least 3 characters';
    } else if (roomData.name.length > 30) {
      newErrors.name = 'Room name must be less than 30 characters';
    }

    if (roomData.maxPlayers < 2) {
      newErrors.maxPlayers = 'At least 2 players required';
    } else if (roomData.maxPlayers > 10) {
      newErrors.maxPlayers = 'Maximum 10 players allowed';
    }

    setErrors(newErrors);
    return Object.keys(newErrors).length === 0;
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    if (validateForm()) {
      onCreateRoom(roomData);
    }
  };

  const handleCancel = (e) => {
    e.preventDefault();
    onCancel();
  };

  return (
    <div className="modal-overlay">
      <div className="modal-content create-room-modal">
        <div className="modal-header">
          <h2>Create New Room</h2>
          <button className="close-btn" onClick={handleCancel}>×</button>
        </div>

        <form onSubmit={handleSubmit} className="create-room-form">
          <div className="form-group">
            <label htmlFor="name">Room Name *</label>
            <input
              type="text"
              id="name"
              name="name"
              value={roomData.name}
              onChange={handleInputChange}
              placeholder="Enter room name"
              className={errors.name ? 'error' : ''}
              maxLength="30"
            />
            {errors.name && <span className="error-message">{errors.name}</span>}
            <div className="char-count">{roomData.name.length}/30</div>
          </div>

          <div className="form-group">
            <label htmlFor="gameType">Game Type</label>
            <select
              id="gameType"
              name="gameType"
              value={roomData.gameType}
              onChange={handleInputChange}
            >
              {gameTypes.map(type => (
                <option key={type} value={type}>{type}</option>
              ))}
            </select>
          </div>

          <div className="form-group">
            <label htmlFor="maxPlayers">Maximum Players</label>
            <input
              type="number"
              id="maxPlayers"
              name="maxPlayers"
              value={roomData.maxPlayers}
              onChange={handleInputChange}
              min="2"
              max="10"
              className={errors.maxPlayers ? 'error' : ''}
            />
            {errors.maxPlayers && <span className="error-message">{errors.maxPlayers}</span>}
          </div>

          <div className="form-actions">
            <button type="button" onClick={handleCancel} className="cancel-btn">
              Cancel
            </button>
            <button type="submit" className="create-btn">
              Create Room
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}

export default CreateRoom;
