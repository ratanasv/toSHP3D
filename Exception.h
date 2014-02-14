#pragma once

#include <stdexcept>
#include <cstring>

class OGRTransformationError : public std::runtime_error {
public:
	OGRTransformationError(const std::string& what) : std::runtime_error(what) {}
};

class CurlInitException : runtime_error {
public:
	CurlInitException(const std::string& what) : runtime_error(what) {};
};

class CurlConnectionException : runtime_error {
public:
	CurlConnectionException(const std::string& what) : runtime_error(what) {};
};