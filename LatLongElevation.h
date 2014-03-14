#pragma once
#include <cstring>
#include <memory>
#include <vector>

using std::runtime_error;
using std::string;
using std::shared_ptr;
using std::vector;
using std::out_of_range;

class Interpolate;

class LatLongElevation {
public:
	virtual ~LatLongElevation() {};
	virtual float elevAt(double longtitude, double latitude) const = 0;
};

/** 
 * Immutable class, so thread-safe.
 */
class MikeDEM : LatLongElevation {
private:
	const int NUM_LNGS;
	const int NUM_LATS;
	const double LNG_MIN;
	const double LNG_MAX;
	const double LAT_MIN;
	const double LAT_MAX;
	shared_ptr<Interpolate> _interpolate;
public:
	MikeDEM(double lngMin, double lngMax, double latMin, double latMax, 
		int numLngs, int numLats); //throws CurlInitException, CurlConnectionException;
	virtual float elevAt(double longtitude, double latitude) const;
protected:
	string createQueryString(const string& baseURL);
	void initHeightWithXTR(char* data);
	static std::shared_ptr<Interpolate> InterpolateFactory(
		std::shared_ptr<float> data, int numLngs);
};



