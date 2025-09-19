#!/bin/bash
# Game Lobby System Startup Script

echo "Starting Game Lobby System..."

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Please install Docker first."
    exit 1
fi

# Check if Docker Compose is installed
if ! command -v docker-compose &> /dev/null; then
    echo "Docker Compose is not installed. Please install Docker Compose first."
    exit 1
fi

# Create data directory
mkdir -p data/mongodb

# Start the services
echo "Starting services with Docker Compose..."
docker-compose up -d

# Wait for services to be ready
echo "Waiting for services to start..."
sleep 10

echo "Game Lobby System is starting up!"
echo "Frontend: http://localhost:3000"
echo "MongoDB Admin: http://localhost:8081 (admin/admin)"
echo ""
echo "To stop the system, run: docker-compose down"
