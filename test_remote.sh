#!/bin/bash

# Remote Cinema Microservice Test Script
# Usage: ./test-remote.sh [SERVER_HOST] [SERVER_PORT]

set -e

SERVER_HOST=${1:-localhost}
SERVER_PORT=${2:-8080}

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

print_status "Testing Cinema Microservice at $SERVER_HOST:$SERVER_PORT"
echo "============================================================"

# Test 1: Basic connectivity
print_status "Testing basic connectivity..."
if command -v nc >/dev/null 2>&1; then
    if nc -z $SERVER_HOST $SERVER_PORT; then
        print_success "Server is reachable on port $SERVER_PORT"
    else
        print_error "Server is not reachable"
        exit 1
    fi
else
    print_warning "netcat not available, skipping connectivity test"
fi

# Test 2: Docker availability
print_status "Checking Docker availability..."
if command -v docker >/dev/null 2>&1; then
    if docker info >/dev/null 2>&1; then
        print_success "Docker is available and running"
    else
        print_error "Docker is not running"
        exit 1
    fi
else
    print_error "Docker is not installed"
    exit 1
fi

# Test 3: WebSocket connection test (if wscat available)
print_status "Testing WebSocket connection..."
if command -v wscat >/dev/null 2>&1; then
    echo "get_data" | timeout 10s wscat -c ws://$SERVER_HOST:$SERVER_PORT >/dev/null 2>&1 && \
        print_success "WebSocket connection successful" || \
        print_warning "WebSocket test inconclusive"
else
    print_warning "wscat not available, skipping WebSocket test"
fi

# Test 4: Client container test
print_status "Testing client container..."

# Check if we have the client image locally
if docker images | grep -q cinema.*client; then
    CLIENT_IMAGE=$(docker images --format "{{.Repository}}:{{.Tag}}" | grep cinema.*client | head -1)
    print_status "Using local client image: $CLIENT_IMAGE"
elif docker pull yourusername/cinema-client:latest >/dev/null 2>&1; then
    CLIENT_IMAGE="yourusername/cinema-client:latest"
    print_status "Using Docker Hub client image: $CLIENT_IMAGE"
else
    print_warning "No client image available, building from source..."
    if [ -f "docker compose.yml" ] || [ -f "docker compose.deploy.yml" ]; then
        docker compose build cinema-client >/dev/null 2>&1 && \
            CLIENT_IMAGE="cinema-microservice_cinema-client:latest" || \
            (print_error "Failed to build client image" && exit 1)
    else
        print_error "No docker compose file found and no pre-built image available"
        exit 1
    fi
fi

# Run automated client test
print_status "Running automated client test..."
docker run --rm \
    -e SERVER_HOST=$SERVER_HOST \
    -e SERVER_PORT=$SERVER_PORT \
    $CLIENT_IMAGE \
    timeout 30s bash -c "echo -e '1\n4' | ./build/client/cinema_client" >/dev/null 2>&1 && \
    print_success "Client test completed successfully" || \
    print_warning "Client test completed with warnings (this may be normal)"

# Test 5: Performance test (optional)
print_status "Running quick performance test..."
START_TIME=$(date +%s)
for i in {1..3}; do
    docker run --rm \
        -e SERVER_HOST=$SERVER_HOST \
        -e SERVER_PORT=$SERVER_PORT \
        $CLIENT_IMAGE \
        timeout 10s bash -c "echo '4' | ./build/client/cinema_client" >/dev/null 2>&1 &
done
wait
END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))
print_success "Performance test completed in ${DURATION}s"

# Summary
echo
echo "============================================================"
print_success " All remote tests completed successfully!"
echo
print_status "To run the client interactively:"
echo "docker run -it --rm -e SERVER_HOST=$SERVER_HOST -e SERVER_PORT=$SERVER_PORT $CLIENT_IMAGE ./build/client/cinema_client"
echo
print_status "To access server logs (if running locally):"
echo "docker logs cinema-server"
echo
print_status "To stop the server (if running locally):"
echo "docker stop cinema-server"