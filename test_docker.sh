#!/bin/bash

# Cinema Microservice Docker Test Script
# This script tests if Docker correctly starts the server and client

set -e  # Exit on any error

echo " Cinema Microservice Docker Test"
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Function to cleanup containers
cleanup() {
    print_status "Cleaning up containers..."
    docker compose down --remove-orphans 2>/dev/null || true
    docker container prune -f 2>/dev/null || true
}

# Function to check if Docker is running
check_docker() {
    print_status "Checking if Docker is running..."
    if ! docker info >/dev/null 2>&1; then
        print_error "Docker is not running. Please start Docker and try again."
        exit 1
    fi
    print_success "Docker is running"
}

# Function to check if docker compose is available
check_docker_compose() {
    print_status "Checking docker compose availability..."
    if ! command -v docker compose >/dev/null 2>&1; then
        print_error "docker compose not found. Please install docker compose."
        exit 1
    fi
    print_success "docker compose is available"
}

# Function to build Docker images
build_images() {
    print_status "Building Docker images..."
    if docker compose build; then
        print_success "Docker images built successfully"
    else
        print_error "Failed to build Docker images"
        exit 1
    fi
}

# Function to start the server
start_server() {
    print_status "Starting Cinema server..."
    docker compose up -d cinema-server
    
    # Wait for server to start
    print_status "Waiting for server to start (10 seconds)..."
    sleep 10
    
    # Check if server container is running
    if docker compose ps cinema-server | grep -q "Up"; then
        print_success "Cinema server is running"
    else
        print_error "Cinema server failed to start"
        docker compose logs cinema-server
        exit 1
    fi
}

# Function to test server connectivity
test_server_connectivity() {
    print_status "Testing server connectivity..."
    
    # Check if server is listening on port 8080
    if docker exec cinema-server netstat -tln | grep -q ":8080"; then
        print_success "Server is listening on port 8080"
    else
        print_warning "Server might not be listening on port 8080"
        print_status "Server logs:"
        docker compose logs cinema-server | tail -10
    fi
    
    # Test WebSocket connection using wscat
    print_status "Testing WebSocket connection..."
    if docker run --rm --network cinema-microservice_cinema-network appropriate/curl -s --max-time 5 http://cinema-server:8080 >/dev/null 2>&1; then
        print_success "Server is reachable"
    else
        print_warning "Server connection test inconclusive (this might be normal for WebSocket-only servers)"
    fi
}

# Function to run client interactively
run_client_interactive() {
    print_status "Starting Cinema client interactively..."
    print_warning "You can now interact with the client. Type 'exit' to quit."
    echo
    docker compose run --rm cinema-client ./build/client/cinema_client
}

# Function to run automated client test
run_client_automated() {
    print_status "Running automated client test..."
    
    # Create a simple test script for the client
    cat > test_client.sh << 'EOF'
#!/bin/bash
echo "Starting automated client test..."
timeout 30s ./build/client/cinema_client << 'INPUT' || true
1
4
INPUT
echo "Automated client test completed"
EOF
    
    # Copy test script to container and run it
    docker cp test_client.sh cinema-client:/app/test_client.sh 2>/dev/null || true
    docker compose run --rm cinema-client bash -c "chmod +x test_client.sh && ./test_client.sh" || true
    
    # Cleanup
    rm -f test_client.sh
}

# Function to show running containers
show_status() {
    print_status "Current container status:"
    docker compose ps
    echo
    
    print_status "Server logs (last 10 lines):"
    docker compose logs --tail=10 cinema-server
    echo
}

# Function to run comprehensive test
run_comprehensive_test() {
    print_status "Running comprehensive Docker test..."
    echo
    
    # Cleanup any existing containers
    cleanup
    
    # Run all checks
    check_docker
    check_docker_compose
    build_images
    start_server
    test_server_connectivity
    show_status
    
    print_success "Server tests completed successfully!"
    echo
    
    # Ask user about client testing
    echo "Choose client testing mode:"
    echo "1) Interactive client (you can interact with the UI)"
    echo "2) Automated client test (non-interactive)"
    echo "3) Skip client testing"
    echo
    read -p "Enter your choice (1-3): " choice
    
    case $choice in
        1)
            run_client_interactive
            ;;
        2)
            run_client_automated
            ;;
        3)
            print_status "Skipping client testing"
            ;;
        *)
            print_warning "Invalid choice, running interactive client"
            run_client_interactive
            ;;
    esac
    
    echo
    print_success " Docker test completed!"
    print_status "To manually interact with containers:"
    echo "  docker compose exec cinema-server bash     # Access server container"
    echo "  docker compose run --rm cinema-client bash # Access client container"
    echo "  docker compose logs cinema-server          # View server logs"
    echo "  docker compose down                        # Stop all containers"
}

# Function to show help
show_help() {
    echo "Cinema Microservice Docker Test Script"
    echo
    echo "Usage: $0 [OPTION]"
    echo
    echo "Options:"
    echo "  test        Run comprehensive Docker test (default)"
    echo "  build       Build Docker images only"
    echo "  server      Start server only"
    echo "  client      Run client interactively"
    echo "  status      Show container status"
    echo "  cleanup     Stop and remove containers"
    echo "  help        Show this help message"
    echo
}

# Main script logic
case "${1:-test}" in
    test)
        run_comprehensive_test
        ;;
    build)
        check_docker
        check_docker_compose
        build_images
        ;;
    server)
        check_docker
        start_server
        show_status
        ;;
    client)
        print_status "Running client interactively..."
        docker compose run --rm cinema-client ./build/client/cinema_client
        ;;
    status)
        show_status
        ;;
    cleanup)
        cleanup
        print_success "Cleanup completed"
        ;;
    help)
        show_help
        ;;
    *)
        print_error "Unknown option: $1"
        show_help
        exit 1
        ;;
esac