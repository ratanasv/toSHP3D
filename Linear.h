#pragma once

#include <memory>
#include "Nearest.h"

class Linear : public Nearest {
public:
	Linear(std::shared_ptr<float>& data, const int resolution);
	virtual float valueAt(float, float);

};