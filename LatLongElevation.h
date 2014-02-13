#pragma once
#include <stdexcept>
#include <cstring>
#include <memory>
#include <vector>

using std::runtime_error;
using std::string;
using std::shared_ptr;
using std::vector;
using std::out_of_range;

class CurlInitException : runtime_error {
public:
	CurlInitException(const string& what) : runtime_error(what) {};
};

class CurlConnectionException : runtime_error {
public:
	CurlConnectionException(const string& what) : runtime_error(what) {};
};

class LatLongElevation {
public:
	virtual ~LatLongElevation() {};
	virtual float elevAt(float longtitude, float latitude) = 0;
};


class MikeDEM : LatLongElevation {
private:
	const int NUM_LNGS;
	const int NUM_LATS;
	const float LNG_MIN;
	const float LNG_MAX;
	const float LAT_MIN;
	const float LAT_MAX;
	shared_ptr<vector<float>> elevationData;
public:
	MikeDEM(float lngMin, float lngMax, float latMin, float latMax, 
		int numLngs, int numLats);
	virtual float elevAt(float longtitude, float latitude);
	virtual float elevAtIndex(int lngI, int latI);
protected:
	string createQueryString(const string& baseURL);
	void initHeightWithXTR(char* data);

private:
	MikeDEM(const MikeDEM& other);
	const MikeDEM& operator=(const MikeDEM& other);
	
};



