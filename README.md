 Cinema Microservice

A real-time cinema booking system built with C++ and WebSocket technology.

 Overview

This project implements a cinema booking system with a server-client architecture. The server manages movie shows and seat availability, while the client provides an interactive booking interface. Communication happens in real-time through WebSocket connections.

 Features

- Real-time seat availability updates
- Multiple theater and movie support
- Interactive booking interface
- Thread-safe seat management
- WebSocket-based communication
- Clean architecture with separated business logic

 Architecture

 Server Components
- Main server application with hardcoded movie data
- WebSocket server for real-time communication
- Cinema library with business logic services
- Thread-safe Shows class for seat management

 Client Components
- Interactive command-line interface
- WebSocket client for server communication
- Real-time seat availability display
- Booking confirmation system

 Project Structure

```
Cinema-microservice/
├── server/
│   ├── main.cpp                 # Server entry point
│   └── lib/
│       ├── cinema.cpp          # Business logic implementation
│       ├── cinema.hpp          # Business logic headers
│       ├── websocket_server.cpp # WebSocket networking
│       └── websocket_server.hpp # WebSocket headers
├── client/
│   ├── main.cpp                # Client entry point
│   └── lib/
│       ├── cinema_Client.cpp   # Shows data model
│       ├── cinema_Client.hpp   # Shows data headers
│       ├── websocket_client.cpp # WebSocket client
│       ├── websocket_client.hpp # WebSocket client headers
│       ├── CinemaUI.cpp        # User interface
│       └── CinemaUI.hpp        # User interface headers
├── build/                      # Build output directory
├── CMakeLists.txt             # Main build configuration
└── README.md                  # This file
```

 Requirements

- C++20 compiler
- CMake 3.15 or higher
- Conan package manager
- Boost libraries (system, websocket)
- Linux environment

 Building and Running

 Option 1: Using Docker 

Docker provides the easiest way to build and run the Cinema microservice without dealing with dependencies.

 Prerequisites
- Docker
- Docker Compose

 Quick Start
```bash
 Make the script executable and run everything
chmod +x start_docker.sh
./start-docker.sh
```

 Manual Docker Commands
```bash
 Build containers
docker-compose build

 Start server only
docker-compose up -d cinema-server

 Test with wscat in client container
docker exec -it cinema-client wscat -c ws://cinema-server:8080

 Use C++ client
docker-compose run --rm cinema-client ./build/client/cinema_client

 View server logs
docker-compose logs cinema-server

 Stop everything
docker-compose down
```

## Documentation

The project includes comprehensive Doxygen documentation for all classes and functions.

### Generating Documentation

To generate the API documentation:

```bash
# Make the script executable
chmod +x generate_docs.sh

# Generate documentation
./generate_docs.sh
```

Alternatively, you can run Doxygen directly:
```bash
# Install Doxygen (if not already installed)
sudo apt install doxygen graphviz

# Generate documentation
doxygen
```

### Viewing Documentation

After generating the documentation, you can view it in several ways:

```bash
# Use the viewing script (opens in browser)
chmod +x view_docs.sh
./view_docs.sh

# Or open directly
open docs/html/index.html           # macOS
xdg-open docs/html/index.html       # Linux
```

The documentation includes:
- Class hierarchies and relationships
- Function and method descriptions
- Code examples and usage patterns
- Dependency graphs
- Source code cross-references

 Option 2: Local Build

 Building the Project

1. Install dependencies using Conan:
```bash
conan install . --output-folder=build --build=missing

```

2. Build the project:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake
cmake --build build --config Release
```

 Running the Application

 Start the Server
```bash
./build/cinema
```
The server will start on localhost:8080 and display available shows.

 Start the Client
```bash
./build/client/cinema_client
```

 Usage

 Server
The server automatically starts with predefined movie data:
- 3 theaters: PVR, IMAX, Cinepolis
- 3 movies: Inception, Interstellar, Tenet
- 20 seats per show with different occupancy patterns

 Client Menu Options
1. View Movies - Display all available shows and seats
2. Book Seats - Interactive booking process
3. Help - Booking instructions
4. Exit - Close the application

 Booking Process
1. Select a theater from the available list
2. Choose a movie and showtime
3. View seat layout and availability
4. Select desired seats (comma or space separated)
5. Confirm booking

 WebSocket Testing

You can also interact directly with the server using WebSocket tools to see real-time data and make custom reservations.

 Using wscat for Testing
Install wscat globally:
```
npm install -g wscat
```

Connect to the server:
```
npx wscat -c ws://localhost:8080
```

 Available Commands
- `get_data` - Get current cinema data and seat availability
- `refresh` - Refresh cinema data
- `theater,movie,seat1,seat2,...` - Make a reservation

 Example Reservation
To book seats 3 and 4 for Inception at PVR theater:
```
PVR,Inception,3,4
```

The server will respond with either:
- `SUCCESS: Booked seats 3, 4 for Inception at PVR` (if successful)
- `ERROR: One or more seats are already booked or invalid` (if failed)

After successful bookings, all connected clients receive real-time updates showing the new seat availability.

 Technical Details

 Communication Protocol
- WebSocket-based real-time communication
- JSON-like message format for data exchange
- Automatic seat availability updates
- Broadcast updates to all connected clients

 Seat Management
- Thread-safe operations using shared_mutex
- 20 seats per show (numbered 1-20)
- Real-time availability tracking
- Atomic booking operations

 Data Models
- Shows class: Manages movie, theater, datetime, and seat data
- CinemaService: Formats and processes cinema data
- BookingService: Handles seat reservation logic
- MessageHandler: Processes client-server communication

 Error Handling

The system includes comprehensive error handling for:
- Network connection failures
- Invalid booking requests
- Seat unavailability
- Client disconnections
- Malformed messages

 Development

 Code Organization
- Business logic separated from networking code
- Clean architecture with service classes
- Header-implementation file separation

 Thread Safety
- Shared mutexes for concurrent read access
- Exclusive locks for seat modifications
- Thread-safe message queuing

 Future Enhancements

Potential improvements could include:
- Database integration for persistent data
- User authentication system
- Payment processing integration
- REST API endpoints
- Web-based client interface
- Show scheduling system
- Advanced seat selection features

 License

This project is a coding test implementation.