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
	virtual float elevAt(double longtitude, double latitude) const = 0;
};


class MikeDEM : LatLongElevation {
private:
	const int NUM_LNGS;
	const int NUM_LATS;
	const double LNG_MIN;
	const double LNG_MAX;
	const double LAT_MIN;
	const double LAT_MAX;
	shared_ptr<vector<float>> elevationData;
public:
	MikeDEM(double lngMin, double lngMax, double latMin, double latMax, 
		int numLngs, int numLats);
	virtual float elevAt(double longtitude, double latitude) const;
	virtual float elevAtIndex(int lngI, int latI) const;
protected:
	string createQueryString(const string& baseURL);
	void initHeightWithXTR(char* data);
};



