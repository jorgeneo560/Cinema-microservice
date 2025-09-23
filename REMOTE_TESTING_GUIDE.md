 Testing Cinema Microservice on Another PC

 Overview

There are several ways to test your Cinema microservice on another PC:

1. Docker Hub Deployment (Recommended - Easiest)
2. Git Clone + Local Build
3. Binary Distribution
4. Network Access to Running Server

 Method 1: Docker Hub Deployment (Recommended) 

Step 1: Push Images to Docker Hub

On your development machine:

```bash
# Tag your images for Docker Hub
docker tag cinema-microservice_cinema-server:latest yourusername/cinema-server:latest
docker tag cinema-microservice_cinema-client:latest yourusername/cinema-client:latest

# Login to Docker Hub
docker login

# Push images
docker push yourusername/cinema-server:latest
docker push yourusername/cinema-client:latest
```

 Step 2: Create Deployment docker compose.yml

Create a simplified docker compose file for deployment:

```yaml
# docker compose.deploy.yml
version: '3.8'

services:
  cinema-server:
    image: yourusername/cinema-server:latest
    ports:
      - "8080:8080"
    container_name: cinema-server
    networks:
      - cinema-network
    restart: unless-stopped

  cinema-client:
    image: yourusername/cinema-client:latest
    depends_on:
      - cinema-server
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
```

### Step 3: Deploy on Target PC

On the target PC (needs Docker installed):

```bash
# Download the deployment file
wget https://raw.githubusercontent.com/yourusername/Cinema-microservice/main/docker compose.deploy.yml

# Or create the file manually and copy the content above

# Pull and start the services
docker compose -f docker compose.deploy.yml pull
docker compose -f docker compose.deploy.yml up -d cinema-server

# Test the client
docker compose -f docker compose.deploy.yml run --rm cinema-client ./build/client/cinema_client
```

 Method 2: Git Clone + Local Build 

 Prerequisites on Target PC

```bash
# Install dependencies
sudo apt update
sudo apt install -y git build-essential cmake python3 python3-pip docker.io docker compose

# Install Conan
pip3 install conan
```

 Clone and Build

```bash
# Clone the repository
git clone https://github.com/yourusername/Cinema-microservice.git
cd Cinema-microservice

# Build with Docker (easiest)
docker compose build

# Or build locally
conan profile detect --force
conan install . --output-folder=build --build=missing
cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake
make -j$(nproc)

# Run the system
docker compose up -d cinema-server
docker compose run --rm cinema-client ./build/client/cinema_client
```

 Method 3: Network Access Testing 

 Server on Development PC, Client on Target PC

On Development PC (Server):

```bash
# Start server accessible from network
docker run -d -p 8080:8080 --name cinema-server cinema-microservice_cinema-server:latest

# Or modify server to bind to all interfaces
# In server/main.cpp, change:
# auto const address = net::ip::make_address("0.0.0.0");  // Already correct!

# Find your IP address
ip addr show | grep inet
```

On Target PC (Client):

```bash
# Option 1: Using Docker
docker run -it --rm \
  -e SERVER_HOST=192.168.1.100 \
  -e SERVER_PORT=8080 \
  yourusername/cinema-client:latest \
  ./build/client/cinema_client

# Option 2: Build client locally and run
git clone https://github.com/yourusername/Cinema-microservice.git
cd Cinema-microservice
# Build client only...
SERVER_HOST=192.168.1.100 SERVER_PORT=8080 ./build/client/cinema_client
```

 Method 4: Complete Binary Distribution 

Create Distribution Package

On development PC:

```bash
# Create distribution directory
mkdir cinema-distribution
cd cinema-distribution

# Copy binaries
cp ../build/cinema ./cinema-server
cp ../build/client/cinema_client ./cinema-client

# Copy required libraries (if any)
ldd cinema-server
ldd cinema-client
# Copy any non-system libraries to lib/ directory

# Create start scripts
cat > start-server.sh << 'EOF'
#!/bin/bash
export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH
./cinema-server
EOF

cat > start-client.sh << 'EOF'
#!/bin/bash
export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH
export SERVER_HOST=${SERVER_HOST:-localhost}
export SERVER_PORT=${SERVER_PORT:-8080}
./cinema-client
EOF

chmod +x start-*.sh

# Create archive
cd ..
tar -czf cinema-microservice-$(date +%Y%m%d).tar.gz cinema-distribution/
```

Deploy on Target PC

```bash
# Extract and run
tar -xzf cinema-microservice-*.tar.gz
cd cinema-distribution

# Start server
./start-server.sh &

# Start client
./start-client.sh
```

Automated Testing Scripts

Create Remote Test Script

```bash
# test-remote.sh
#!/bin/bash

SERVER_HOST=${1:-localhost}
SERVER_PORT=${2:-8080}

echo "Testing Cinema Microservice at $SERVER_HOST:$SERVER_PORT"

# Test server connectivity
if nc -z $SERVER_HOST $SERVER_PORT; then
    echo "Server is reachable"
else
    echo "Server is not reachable"
    exit 1
fi

# Test WebSocket connection (if wscat is available)
if command -v wscat >/dev/null 2>&1; then
    echo "Testing WebSocket connection..."
    timeout 10s wscat -c ws://$SERVER_HOST:$SERVER_PORT << 'EOF'
get_data
EOF
    echo "WebSocket test completed"
fi

# Run automated client test
echo "Running client test..."
timeout 30s docker run --rm \
    -e SERVER_HOST=$SERVER_HOST \
    -e SERVER_PORT=$SERVER_PORT \
    yourusername/cinema-client:latest \
    bash -c "echo '1' | ./build/client/cinema_client"

echo "Remote test completed"
```

Network Configuration Tips

Firewall Configuration

```bash
# On server machine, allow incoming connections
sudo ufw allow 8080/tcp

# Check if port is accessible
netstat -tlnp | grep 8080
```

Testing Network Connectivity

```bash
# Test from client machine
telnet SERVER_IP 8080
# Or
nc -zv SERVER_IP 8080

# Test WebSocket (if wscat installed)
wscat -c ws://SERVER_IP:8080
```

Cloud Deployment Options

#AWS EC2 / DigitalOcean / GCP

```bash
# Install Docker on cloud instance
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER

# Deploy using docker compose
wget https://raw.githubusercontent.com/yourusername/Cinema-microservice/main/docker compose.deploy.yml
docker compose -f docker compose.deploy.yml up -d

# Access via public IP
# Client: docker run -e SERVER_HOST=PUBLIC_IP yourusername/cinema-client
```

Docker Swarm (Multi-node)

```bash
# Initialize swarm
docker swarm init

# Deploy stack
docker stack deploy -c docker compose.deploy.yml cinema

# Scale services
docker service scale cinema_cinema-server=2
```

Troubleshooting Remote Testing

Common Issues

1. Connection Refused
   ```bash
   # Check server binding
   docker logs cinema-server
   
   # Verify port mapping
   docker port cinema-server
   ```

2. WebSocket Handshake Failed
   ```bash
   # Check server logs for detailed error
   docker compose logs cinema-server
   
   # Verify client environment variables
   docker run --rm yourusername/cinema-client env
   ```

3. Network Isolation
   ```bash
   # Test basic connectivity
   ping SERVER_HOST
   telnet SERVER_HOST 8080
   ```

Debug Mode

Add debug flags to client:

```bash
# Run client with verbose output
docker run -it --rm \
    -e SERVER_HOST=remote-server \
    -e SERVER_PORT=8080 \
    -e DEBUG=1 \
    yourusername/cinema-client:latest \
    ./build/client/cinema_client
```

Monitoring and Logs

Server Monitoring

```bash
# Real-time logs
docker compose logs -f cinema-server

# Resource usage
docker stats cinema-server

# Health check
curl -I http://SERVER_IP:8080
```

Performance Testing

```bash
# Load testing with multiple clients
for i in {1..10}; do
    docker run -d --name client-$i \
        -e SERVER_HOST=remote-server \
        yourusername/cinema-client:latest \
        bash -c "echo '1' | ./build/client/cinema_client"
done
```

Summary

Recommended Approach for Different Scenarios:

- Quick Demo: Method 1 (Docker Hub)
- Development Testing: Method 2 (Git Clone)  
- Production: Method 1 + Cloud deployment
- Offline Distribution: Method 4 (Binary package)
- Network Testing: Method 3 (Remote server)

Choose the method that best fits your testing requirements and the target environment capabilities!