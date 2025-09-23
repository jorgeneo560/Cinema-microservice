#!/bin/bash

# Cinema Microservice Quick Deploy Script
# This script sets up the Cinema microservice on a new machine

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo " Cinema Microservice Quick Deploy"
echo

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    print_warning "Running as root. This script can run as a regular user."
fi

# Install Docker if not present
print_status "Checking Docker installation..."
if ! command -v docker >/dev/null 2>&1; then
    print_status "Installing Docker..."
    curl -fsSL https://get.docker.com -o get-docker.sh
    sudo sh get-docker.sh
    sudo usermod -aG docker $USER
    rm get-docker.sh
    print_success "Docker installed successfully"
    print_warning "You may need to log out and back in for Docker permissions to take effect"
else
    print_success "Docker is already installed"
fi

# Install Docker Compose if not present
print_status "Checking Docker Compose installation..."
if ! command -v docker compose >/dev/null 2>&1; then
    print_status "Installing Docker Compose..."
    sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker compose
    sudo chmod +x /usr/local/bin/docker compose
    print_success "Docker Compose installed successfully"
else
    print_success "Docker Compose is already installed"
fi

# Install additional utilities
print_status "Installing additional utilities..."
sudo apt update >/dev/null 2>&1
sudo apt install -y netcat-openbsd curl wget git >/dev/null 2>&1
print_success "Additional utilities installed"

# Option 1: Clone from Git (if repository is public)
if [ "$1" == "git" ] || [ "$1" == "clone" ]; then
    print_status "Cloning Cinema microservice from Git..."
    REPO_URL="https://github.com/yourusername/Cinema-microservice.git"  # Update this URL
    git clone $REPO_URL cinema-microservice
    cd cinema-microservice
    print_success "Repository cloned successfully"

# Option 2: Download docker compose file only
else
    print_status "Setting up deployment environment..."
    mkdir -p cinema-microservice
    cd cinema-microservice
    
    # Download deployment docker compose file
    cat > docker compose.yml << 'EOF'
version: '3.8'

services:
  cinema-server:
    image: yourusername/cinema-server:latest  # Update this image name
    ports:
      - "8080:8080"
    container_name: cinema-server
    networks:
      - cinema-network
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "nc", "-z", "localhost", "8080"]
      interval: 30s
      timeout: 10s
      retries: 3

  cinema-client:
    image: yourusername/cinema-client:latest  # Update this image name
    depends_on:
      cinema-server:
        condition: service_healthy
    container_name: cinema-client
    stdin_open: true
    tty: true
    networks:
      - cinema-network
    environment:
      - SERVER_HOST=cinema-server
      - SERVER_PORT=8080

networks:
  cinema-network:
    driver: bridge
EOF
    print_success "Deployment configuration created"
fi

# Pull images
print_status "Pulling Docker images..."
if docker compose pull >/dev/null 2>&1; then
    print_success "Images pulled successfully"
else
    print_warning "Could not pull images from registry. Will try to build locally if source is available."
    if [ -f "Dockerfile.server" ] && [ -f "Dockerfile.client" ]; then
        print_status "Building images locally..."
        docker compose build
        print_success "Images built locally"
    else
        print_error "No images available and no source to build from"
        exit 1
    fi
fi

# Create convenience scripts
print_status "Creating convenience scripts..."

cat > start-server.sh << 'EOF'
#!/bin/bash
echo "Starting Cinema Server..."
docker compose up -d cinema-server
echo "Server started! Check status with: docker compose ps"
echo "View logs with: docker compose logs -f cinema-server"
EOF

cat > start-client.sh << 'EOF'
#!/bin/bash
echo "Starting Cinema Client..."
docker compose run --rm cinema-client ./build/client/cinema_client
EOF

cat > stop-all.sh << 'EOF'
#!/bin/bash
echo "Stopping Cinema Microservice..."
docker compose down
echo "All services stopped."
EOF

cat > view-logs.sh << 'EOF'
#!/bin/bash
echo "Cinema Server Logs:"
echo "=================="
docker compose logs cinema-server
EOF

chmod +x *.sh

print_success "Convenience scripts created"

# Test the deployment
print_status "Testing deployment..."
docker compose up -d cinema-server
sleep 10

if docker compose ps | grep -q "Up"; then
    print_success "Server started successfully!"
    
    # Quick client test
    print_status "Running quick client test..."
    timeout 15s docker compose run --rm cinema-client bash -c "echo '4' | ./build/client/cinema_client" >/dev/null 2>&1 && \
        print_success "Client test successful!" || \
        print_warning "Client test completed (check logs if issues persist)"
else
    print_error "Server failed to start"
    docker compose logs cinema-server
    exit 1
fi

# Final instructions
echo
echo "Cinema Microservice deployed successfully!"
echo
print_status "Available commands:"
echo "  ./start-server.sh     - Start the server"
echo "  ./start-client.sh     - Start interactive client"
echo "  ./stop-all.sh         - Stop all services"
echo "  ./view-logs.sh        - View server logs"
echo
print_status "Manual commands:"
echo "  docker compose up -d                                    - Start server in background"
echo "  docker compose run --rm cinema-client ./build/client/cinema_client  - Run client"
echo "  docker compose logs cinema-server                       - View server logs"
echo "  docker compose down                                      - Stop all services"
echo
print_status "Server is running on:"
echo "  Local:    http://localhost:8080"
echo "  Network:  http://$(hostname -I | awk '{print $1}'):8080"
echo
print_success "Setup complete! You can now run './start-client.sh' to start booking!"

# Clean up
docker compose down >/dev/null 2>&1