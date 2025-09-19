import React, { useState, useEffect, useRef } from 'react';

function Chat({ messages, onSendMessage, currentUser }) {
  const [messageInput, setMessageInput] = useState('');
  const messagesEndRef = useRef(null);
  const chatInputRef = useRef(null);

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  const handleSendMessage = (e) => {
    e.preventDefault();
    if (messageInput.trim()) {
      onSendMessage(messageInput.trim());
      setMessageInput('');
      chatInputRef.current?.focus();
    }
  };

  const handleKeyPress = (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      handleSendMessage(e);
    }
  };

  return (
    <div className="chat">
      <div className="chat-messages">
        {messages.length === 0 ? (
          <div className="chat-empty">
            <p>No messages yet. Start the conversation!</p>
          </div>
        ) : (
          messages.map((message) => (
            <div 
              key={message.id} 
              className={`message ${message.username === currentUser.username ? 'own-message' : ''}`}
            >
              <div className="message-header">
                <span className="message-username">{message.username}</span>
                <span className="message-timestamp">{message.timestamp}</span>
              </div>
              <div className="message-content">{message.message}</div>
            </div>
          ))
        )}
        <div ref={messagesEndRef} />
      </div>

      <form className="chat-input-form" onSubmit={handleSendMessage}>
        <div className="chat-input-container">
          <textarea
            ref={chatInputRef}
            value={messageInput}
            onChange={(e) => setMessageInput(e.target.value)}
            onKeyPress={handleKeyPress}
            placeholder="Type your message..."
            rows="2"
            maxLength="500"
          />
          <button 
            type="submit" 
            disabled={!messageInput.trim()}
            className="send-button"
          >
            Send
          </button>
        </div>
        <div className="chat-input-info">
          <span>{messageInput.length}/500 characters</span>
        </div>
      </form>
    </div>
  );
}

export default Chat;
