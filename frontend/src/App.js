import React, { useState, useEffect } from 'react';
import './App.css';
import Lobby from './components/Lobby';
import WebSocketService from './services/WebSocketService';

function App() {
  const [user, setUser] = useState(null);
  const [isAuthenticated, setIsAuthenticated] = useState(false);
  const [connectionStatus, setConnectionStatus] = useState('Disconnected');
  const [loginForm, setLoginForm] = useState({ username: '' });

  useEffect(() => {
    // Initialize WebSocket connection
    WebSocketService.connect();

    // Set up event handlers
    WebSocketService.on('open', () => {
      setConnectionStatus('Connected');
    });

    WebSocketService.on('close', () => {
      setConnectionStatus('Disconnected');
      setIsAuthenticated(false);
      setUser(null);
    });

    WebSocketService.on('auth_success', (data) => {
      setUser(data.user);
      setIsAuthenticated(true);
      console.log('Authentication successful:', data.user);
    });

    WebSocketService.on('error', (error) => {
      console.error('WebSocket error:', error);
      setConnectionStatus('Error');
    });

    WebSocketService.on('max_reconnect_attempts_reached', () => {
      setConnectionStatus('Connection Failed');
    });

    return () => {
      WebSocketService.disconnect();
    };
  }, []);

  const handleLogin = (e) => {
    e.preventDefault();
    if (loginForm.username.trim() && connectionStatus === 'Connected') {
      const userId = 'user_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
      WebSocketService.authenticate(userId, loginForm.username.trim());
    }
  };

  const handleInputChange = (e) => {
    setLoginForm({
      ...loginForm,
      [e.target.name]: e.target.value
    });
  };

  const handleLogout = () => {
    WebSocketService.disconnect();
    setIsAuthenticated(false);
    setUser(null);
    setLoginForm({ username: '' });
    WebSocketService.connect();
  };

  if (!isAuthenticated) {
    return (
      <div className="App">
        <div className="login-container">
          <h1>Game Lobby System</h1>
          <div className="connection-status">
            Status: <span className={`status ${connectionStatus.toLowerCase().replace(' ', '-')}`}>
              {connectionStatus}
            </span>
          </div>

          {connectionStatus === 'Connected' ? (
            <form onSubmit={handleLogin} className="login-form">
              <h2>Join the Lobby</h2>
              <input
                type="text"
                name="username"
                placeholder="Enter your username"
                value={loginForm.username}
                onChange={handleInputChange}
                required
                minLength="3"
                maxLength="20"
              />
              <button type="submit" disabled={!loginForm.username.trim()}>
                Enter Lobby
              </button>
            </form>
          ) : (
            <div className="connection-waiting">
              {connectionStatus === 'Disconnected' && <p>Connecting to server...</p>}
              {connectionStatus === 'Error' && <p>Connection error. Retrying...</p>}
              {connectionStatus === 'Connection Failed' && (
                <div>
                  <p>Failed to connect to server.</p>
                  <button onClick={() => window.location.reload()}>Retry</button>
                </div>
              )}
            </div>
          )}
        </div>
      </div>
    );
  }

  return (
    <div className="App">
      <header className="app-header">
        <h1>Game Lobby System</h1>
        <div className="user-info">
          <span>Welcome, {user.username}!</span>
          <button onClick={handleLogout} className="logout-btn">Logout</button>
        </div>
      </header>

      <main className="app-main">
        <Lobby user={user} />
      </main>
    </div>
  );
}

export default App;
