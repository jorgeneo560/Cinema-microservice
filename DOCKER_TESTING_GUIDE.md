Docker Testing Guide for Cinema Microservice## Manual Testing Steps

### Build and Start Services
```bash
# Build images
docker compose build

# Start server in background
docker compose up -d cinema-server

# Check server status
docker compose ps
```uick Test Commands

**Automated Complete Test**
```bash
./test_docker.sh
```
This runs a comprehensive test that:
1. Checks Docker installation
2. Builds images
3. Starts server
4. Tests connectivity
5. Offers client testing options

Individual Test Commands

```bash
# Build Docker images only
./test_docker.sh build

# Start server only
./test_docker.sh server

# Run client interactively
./test_docker.sh client

# Check container status
./test_docker.sh status

# Clean up containers
./test_docker.sh cleanup
```

Manual Testing Steps

Build and Start Services
```bash
# Build images
docker compose build

# Start server in background
docker compose up -d cinema-server

# Check server status
docker compose ps
```

Test Server Functionality
```bash
# Check server logs
docker compose logs cinema-server

# Test if server is listening on port 8080
docker exec cinema-server netstat -tln | grep 8080

# Access server container
docker compose exec cinema-server bash
```

Test Client Connectivity
```bash
# Run client interactively
docker compose run --rm cinema-client ./build/client/cinema_client

# Or start client container and access it
docker compose run --rm cinema-client bash
# Then inside container: ./build/client/cinema_client
```

Test WebSocket Communication
```bash
# Test with wscat (WebSocket client)
docker compose run --rm cinema-client wscat -c ws://cinema-server:8080

# Or test HTTP connectivity
docker run --rm --network cinema-microservice_cinema-network \
  appropriate/curl -s --max-time 5 http://cinema-server:8080
```

Expected Test Results

Successful Server Start
```
Server logs should show:
- "Starting WebSocket server on 0.0.0.0:8080"
- "WebSocket server started! Connect to ws://localhost:8080"
- Cinema data display with theaters and available seats
```

Successful Client Connection
```
Client should display:
- "Starting Cinema Client..."
- "Fetching initial cinema data..."
- Main menu with options:
  1. View Movies
  2. Book Seats
  3. Help
  4. Exit
```

Working WebSocket Communication
```
- Client can fetch cinema data (option 1)
- Client can attempt booking (option 2)
- Server responds to client requests
- Real-time updates work between multiple clients
```

Troubleshooting Common Issues

Server Won't Start
```bash
# Check server build logs
docker compose build cinema-server --no-cache

# Check server runtime logs
docker compose logs cinema-server

# Access server container for debugging
docker compose run --rm cinema-server bash
```

Client Can't Connect
```bash
# Verify network connectivity
docker compose run --rm cinema-client ping cinema-server

# Check if server port is accessible
docker compose run --rm cinema-client nc -zv cinema-server 8080

# Test WebSocket manually
docker compose run --rm cinema-client wscat -c ws://cinema-server:8080
```

Build Failures
```bash
# Clean build (removes cached layers)
docker compose build --no-cache

# Check disk space
df -h

# Check Docker daemon
docker info
```

Port Conflicts
```bash
# Check if port 8080 is in use
netstat -tln | grep 8080
# or
lsof -i :8080

# Kill processes using port 8080
sudo fuser -k 8080/tcp
```

Advanced Testing

Multi-Client Testing
```bash
# Terminal 1: Start server
docker compose up cinema-server

# Terminal 2: Start first client
docker compose run --rm cinema-client ./build/client/cinema_client

# Terminal 3: Start second client
docker compose run --rm cinema-client ./build/client/cinema_client

# Test that booking in one client updates the other
```

Load Testing
```bash
# Start multiple clients
for i in {1..5}; do
  docker compose run -d cinema-client bash -c "
    echo 'get_data' | ./build/client/cinema_client || true
  "
done
```

Network Testing
```bash
# Test network isolation
docker network ls
docker network inspect cinema-microservice_cinema-network

# Test DNS resolution
docker compose run --rm cinema-client nslookup cinema-server
```

Performance Monitoring

Resource Usage
```bash
# Monitor container resource usage
docker stats

# Check container health
docker compose ps
```

Log Analysis
```bash
# Follow server logs in real-time
docker compose logs -f cinema-server

# Check client logs
docker compose logs cinema-client

# Export logs for analysis
docker compose logs cinema-server > server.log
docker compose logs cinema-client > client.log
```

Cleanup

Full Cleanup
```bash
# Stop and remove containers
docker compose down

# Remove volumes (if any)
docker compose down -v

# Remove images
docker rmi cinema-microservice_cinema-server
docker rmi cinema-microservice_cinema-client

# Clean Docker system
docker system prune -f
```

Success Criteria

Your Docker setup is working correctly if:

1. Server container starts without errors
2. Server listens on port 8080
3. Client container can reach server
4. WebSocket connection establishes successfully
5. Client can fetch cinema data
6. Client can attempt bookings
7. Real-time updates work between clients
8. Containers restart properly
9. Logs show expected behavior
10. No resource leaks or errors

Run `./test_docker.sh` to automatically verify all these criteria! 