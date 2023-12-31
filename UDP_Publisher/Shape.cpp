#include "Shape.h"

int Shape::idSequence = 0;

void Shape::setLocation() {
	std::random_device rd;
	std::mt19937 gen(rd());
	// Longitude ranges from -180 to +180
	std::uniform_real_distribution<> lonDistribution(-180.0, 180.0);
	location_[0] = lonDistribution(gen);

	// Latitude ranges from -90 to +90
	std::uniform_real_distribution<> latDistribution(-90.0, 90.0);
	location_[1] = latDistribution(gen);

	// Altitude ranges (modify as needed)
	std::uniform_real_distribution<> altDistribution(-1000.0, 8000.0);  // Example: Altitude from -1000m to 8000m
	location_[2] = altDistribution(gen);

}