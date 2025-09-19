class WebSocketService {
    constructor() {
        this.ws = null;
        this.isConnected = false;
        this.messageHandlers = new Map();
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectInterval = 3000;
    }

    connect(url = 'ws://localhost:9002') {
        try {
            this.ws = new WebSocket(url);
            this.setupEventHandlers();
        } catch (error) {
            console.error('WebSocket connection failed:', error);
            this.attemptReconnect();
        }
    }

    setupEventHandlers() {
        this.ws.onopen = (event) => {
            console.log('WebSocket connected');
            this.isConnected = true;
            this.reconnectAttempts = 0;
            this.triggerHandler('open', event);
        };

        this.ws.onmessage = (event) => {
            try {
                const message = JSON.parse(event.data);
                console.log('WebSocket message received:', message);
                this.triggerHandler('message', message);

                // Handle specific message types
                if (message.type) {
                    this.triggerHandler(message.type, message);
                }
            } catch (error) {
                console.error('Error parsing WebSocket message:', error);
            }
        };

        this.ws.onclose = (event) => {
            console.log('WebSocket connection closed');
            this.isConnected = false;
            this.triggerHandler('close', event);

            if (!event.wasClean) {
                this.attemptReconnect();
            }
        };

        this.ws.onerror = (error) => {
            console.error('WebSocket error:', error);
            this.triggerHandler('error', error);
        };
    }

    send(message) {
        if (this.isConnected && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(JSON.stringify(message));
            return true;
        } else {
            console.warn('WebSocket not connected. Message not sent:', message);
            return false;
        }
    }

    authenticate(userId, username) {
        return this.send({
            type: 'auth',
            data: JSON.stringify({ userId, username })
        });
    }

    createRoom(name, gameType = 'Generic') {
        return this.send({
            type: 'create_room',
            data: JSON.stringify({ name, gameType })
        });
    }

    joinRoom(roomId) {
        return this.send({
            type: 'join_room',
            data: JSON.stringify({ roomId })
        });
    }

    leaveRoom(roomId) {
        return this.send({
            type: 'leave_room',
            data: JSON.stringify({ roomId })
        });
    }

    sendChatMessage(roomId, message) {
        return this.send({
            type: 'chat_message',
            data: JSON.stringify({ roomId, message })
        });
    }

    getRooms() {
        return this.send({
            type: 'get_rooms'
        });
    }

    getUsers() {
        return this.send({
            type: 'get_users'
        });
    }

    on(event, handler) {
        if (!this.messageHandlers.has(event)) {
            this.messageHandlers.set(event, []);
        }
        this.messageHandlers.get(event).push(handler);
    }

    off(event, handler) {
        if (this.messageHandlers.has(event)) {
            const handlers = this.messageHandlers.get(event);
            const index = handlers.indexOf(handler);
            if (index > -1) {
                handlers.splice(index, 1);
            }
        }
    }

    triggerHandler(event, data) {
        if (this.messageHandlers.has(event)) {
            this.messageHandlers.get(event).forEach(handler => {
                try {
                    handler(data);
                } catch (error) {
                    console.error(`Error in ${event} handler:`, error);
                }
            });
        }
    }

    attemptReconnect() {
        if (this.reconnectAttempts < this.maxReconnectAttempts) {
            this.reconnectAttempts++;
            console.log(`Attempting to reconnect (${this.reconnectAttempts}/${this.maxReconnectAttempts})...`);

            setTimeout(() => {
                this.connect();
            }, this.reconnectInterval * this.reconnectAttempts);
        } else {
            console.error('Max reconnection attempts reached');
            this.triggerHandler('max_reconnect_attempts_reached');
        }
    }

    disconnect() {
        if (this.ws) {
            this.ws.close();
            this.ws = null;
            this.isConnected = false;
        }
    }
}

// Export singleton instance
export default new WebSocketService();
