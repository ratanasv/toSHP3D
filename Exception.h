#pragma once

#include <stdexcept>
#include <cstring>

class OGRTransformationError : public std::runtime_error {
public:
	OGRTransformationError(const std::string& what) : std::runtime_error(what) {}
};

