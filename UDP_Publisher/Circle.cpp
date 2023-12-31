#include "Circle.h"

Circle::Circle()
{
	color_ = "green";
	size_ = 2;
	Shape::setLocation();
	id_ = ++idSequence;
}

string Circle::shapeInfo() {
	string s = "Circle, ID = :" + to_string(id_) + " Color: " + color_ + ", Size: " + to_string(size_) + "Longitide: " +
		to_string(location_[0]) + ", Latitude: " + to_string(location_[1]) + ", Altitude: " + to_string(location_[2]);
	return s;
}
