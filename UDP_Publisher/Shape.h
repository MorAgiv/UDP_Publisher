#pragma once
#include <string>
#include <random>

#pragma once

using namespace std;

class Shape
{
protected:
	static int idSequence;
	string color_;
	int id_;
	int size_;
	int location_[3];// lat, lon, alt
	void setLocation();

public:
	virtual string shapeInfo() = 0;


};

