# Multi-stage Dockerfile for C++ Game Lobby Backend
FROM ubuntu:22.04 as builder

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV CMAKE_VERSION=3.25.0

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    curl \
    git \
    pkg-config \
    libssl-dev \
    libboost-all-dev \
    libmongocxx-dev \
    libbsoncxx-dev \
    libjsoncpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source files
COPY src/ ./src/
COPY external/ ./external/
COPY CMakeLists.txt ./

# Build the application
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04 as runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libmongocxx3.6 \
    libbsoncxx3.6 \
    libjsoncpp25 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1001 gamelobby

# Set working directory
WORKDIR /app

# Copy built application from builder stage
COPY --from=builder /app/build/GameLobbyServer ./

# Change ownership
RUN chown -R gamelobby:gamelobby /app

# Switch to non-root user
USER gamelobby

# Expose port
EXPOSE 9002

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:9002/health || exit 1

# Start the application
CMD ["./GameLobbyServer", "9002"]
