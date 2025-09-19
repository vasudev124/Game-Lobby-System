# Multi-stage Dockerfile for React Frontend
FROM node:18-alpine as builder

# Set working directory
WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm ci --only=production

# Copy source code
COPY src/ ./src/
COPY public/ ./public/

# Build the application
RUN npm run build

# Runtime stage with nginx
FROM nginx:1.25-alpine as runtime

# Copy custom nginx configuration
COPY nginx.conf /etc/nginx/nginx.conf

# Copy built application from builder stage
COPY --from=builder /app/build /usr/share/nginx/html

# Create non-root user for nginx
RUN addgroup -g 1001 -S gamelobby && \
    adduser -S -D -H -u 1001 -h /var/cache/nginx -s /sbin/nologin -G gamelobby -g gamelobby gamelobby

# Change ownership of nginx files
RUN chown -R gamelobby:gamelobby /var/cache/nginx && \
    chown -R gamelobby:gamelobby /var/log/nginx && \
    chown -R gamelobby:gamelobby /etc/nginx/conf.d

# Use non-root user
USER gamelobby

# Expose port
EXPOSE 3000

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:3000 || exit 1

# Start nginx
CMD ["nginx", "-g", "daemon off;"]
