#!/bin/bash

echo "Building Docker containers..."
docker compose build

echo "Starting Cinema microservice..."
docker compose up -d cinema-server

echo "Waiting for server to start..."
sleep 5

echo "Server is running! You can now:"
echo "1. Test with wscat:"
echo "   docker exec -it cinema-client wscat -c ws://cinema-server:8080"
echo ""
echo "2. Use the C++ client:"
echo "   docker exec -it cinema-client ./build/client/cinema_client"
echo ""
echo "3. Get server logs:"
echo "   docker compose logs cinema-server"
echo ""
echo "4. Stop everything:"
echo "   docker compose down"
echo ""
echo "Starting interactive client container..."
docker compose run --rm cinema-client ./build/client/cinema_client