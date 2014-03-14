#pragma once
#include <memory>
#include "Interpolate.h"

class Nearest : public Interpolate {
public:
	Nearest(std::shared_ptr<float>& data, const int resolution);
	virtual float valueAt(float, float);
private:
	std::shared_ptr<float> _data;
	const int _resolution;
};