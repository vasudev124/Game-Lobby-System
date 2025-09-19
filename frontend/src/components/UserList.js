import React from 'react';

function UserList({ users, currentUser }) {
  const sortedUsers = [...users].sort((a, b) => {
    // Current user first, then alphabetical
    if (a.id === currentUser.id) return -1;
    if (b.id === currentUser.id) return 1;
    return a.username.localeCompare(b.username);
  });

  return (
    <div className="user-list">
      {sortedUsers.length === 0 ? (
        <div className="user-list-empty">
          <p>No users online</p>
        </div>
      ) : (
        sortedUsers.map((user) => (
          <div 
            key={user.id} 
            className={`user-item ${user.id === currentUser.id ? 'current-user' : ''}`}
          >
            <div className="user-avatar">
              {user.username.charAt(0).toUpperCase()}
            </div>
            <div className="user-info">
              <div className="user-name">
                {user.username}
                {user.id === currentUser.id && <span className="you-label">(You)</span>}
              </div>
              <div className="user-status">
                <span className="status-indicator online"></span>
                <span className="status-text">Online</span>
                {user.currentRoom && (
                  <span className="room-indicator">In Room</span>
                )}
              </div>
            </div>
          </div>
        ))
      )}
    </div>
  );
}

export default UserList;
