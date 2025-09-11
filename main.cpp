#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <algorithm>
#include <random>
#include <cstdint>


class Shows {
public:
	std::string movie;
	std::string dateTime; // e.g., "2025-09-11 19:30"
	std::vector<bool> seats;
	Shows(const std::string& m, const std::string& dt) : movie(m), dateTime(dt) {
		seats.resize(20, false); // 20 seats, all initially available
	}

	std::vector<uint8_t> Seats_Availables() const {
		std::vector<uint8_t> availableSeats;
		for (int i = 0; i < seats.size(); ++i) {
			if (!seats[i]) { // if seat is free (false means free)
				availableSeats.push_back(static_cast<uint8_t>(i + 1)); // return 1-based seat numbers
			}
		}
		return availableSeats;
	}

	bool bookSeats(const std::vector<uint8_t>& seatNumbers) {
		// Check if all requested seats are available
		for (uint8_t seat : seatNumbers) {
			if (seat < 1 || seat > seats.size() || seats[seat - 1]) {
				return false; // Seat number out of range or already booked
			}
		}
		// Book the seats
		for (uint8_t seat : seatNumbers) {
			seats[seat - 1] = true; // Mark seat as booked
		}
		return true;
	}
};

// Theater class
class Theater {
public:
	std::string name;
	std::vector<Shows> shows;
	Theater(const std::string& n) : name(n) {}

	void addShow(const Shows& show) {
		shows.push_back(show);
	}
};



int main() {
	// Hardcoded theaters and movies with random seat availability
	#include <random>

	std::vector<std::string> movieNames = {"Inception", "Interstellar", "Tenet"};
	std::vector<std::string> theaterNames = {"PVR", "IMAX", "Cinepolis"};

	std::random_device rd;
	std::mt19937 gen(rd());
	std::bernoulli_distribution seatDist(0.5); // 50% chance seat is free

	std::vector<Theater> theaters;
	for (const auto& tName : theaterNames) {
		Theater theater(tName);
		for (const auto& mName : movieNames) {
			Shows show(mName, "2025-09-11 19:30");
			for (size_t i = 0; i < show.seats.size(); ++i) {
				show.seats[i] = !seatDist(gen); // true = occupied, false = free
			}
			theater.addShow(show);
		}
		theaters.push_back(theater);
	}

	// Print all theaters, movies, and available seats
	for (const auto& theater : theaters) {
		std::cout << "Theater: " << theater.name << "\n";
		for (const auto& show : theater.shows) {
			std::cout << "  Movie: " << show.movie << "\n";
			std::cout << "    Free seats: ";
			for (auto seatNum : show.Seats_Availables()) {
				std::cout << static_cast<int>(seatNum) << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
	return 0;
}
