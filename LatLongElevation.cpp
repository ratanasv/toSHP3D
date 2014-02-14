#include "stdafx.h"

#include "LatLongElevation.h"
#include <curl/curl.h>
#include <cassert>
#include <sstream>


using std::ostringstream;

static const string MIKEDEM_BASE_URL("http://maverick.coas.oregonstate.edu:11300/terrainextraction.ashx?");


typedef struct {
	char* _data;
	size_t _size;
	size_t _capacity;
} CurlData;

size_t write_data(void *ptr, size_t size, size_t count, void *stream) {
	CurlData* curlData = (CurlData*)stream;

	size_t newSize = curlData->_size + size*count;
	if (newSize > curlData->_capacity) {
		size_t toBeAllocated = curlData->_capacity*2;;
		while (toBeAllocated < newSize) {
			toBeAllocated = toBeAllocated*2;
		}
		char* newData = new char[toBeAllocated];

		memcpy(newData, curlData->_data, curlData->_size);

		delete[] curlData->_data;
		curlData->_data = newData;
		curlData->_capacity = toBeAllocated;
	}
	
	memcpy(curlData->_data + curlData->_size, ptr, size*count);
	curlData->_size = newSize;
	return size*count;
}

MikeDEM::MikeDEM(double lngMin, double lngMax, double latMin, double latMax,
	int numLngs, int numLats) : NUM_LNGS(numLngs), NUM_LATS(numLats),
	LNG_MIN(lngMin), LNG_MAX(lngMax), LAT_MIN(latMin), LAT_MAX(latMax) 
{

	shared_ptr<CURL> curl(curl_easy_init(), [](CURL* c) {
		curl_easy_cleanup(c);
	});

	if (!curl) {
		throw CurlInitException("curl_easy_init failed");
	}

	CurlData curlData = {new char[5012], 0, 5012};

	auto queryString = createQueryString(MIKEDEM_BASE_URL);
	curl_easy_setopt(curl.get(), CURLOPT_URL, queryString.c_str());
	curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curlData);
	curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1);


	CURLcode res = curl_easy_perform(curl.get());
	
	
	if (res != CURLE_OK) {
		throw CurlConnectionException(curl_easy_strerror(res));
	}

	initHeightWithXTR(curlData._data);
}

float MikeDEM::elevAt(double longtitude, double latitude) const {
	double latRange = LAT_MAX-LAT_MIN;
	double lngRange = LNG_MAX-LNG_MIN;
	int latI = (int)( ((latitude-LAT_MIN)/latRange)*((double)NUM_LATS-1.0)  );
	int lngI = (int)( ((longtitude-LNG_MIN)/lngRange)*((double)NUM_LNGS-1.0)  );
	return elevAtIndex(lngI, latI);
}

float MikeDEM::elevAtIndex(int lngI, int latI) const {
	if (latI > NUM_LATS || lngI > NUM_LNGS || lngI < 0 || latI < 0 ) {
		throw out_of_range("invalid lat/long");
	}
	return elevationData->at(NUM_LNGS*latI + lngI);
}

string MikeDEM::createQueryString( const string& baseURL ) {
	ostringstream ss;
	ss << MIKEDEM_BASE_URL << "lat1=" << LAT_MIN << "&lat2=" << LAT_MAX <<
		"&lng1=" << LNG_MIN << "&lng2=" << LNG_MAX << "&numlngs=" << NUM_LNGS <<
		"&numlats=" << NUM_LATS;
	return string(ss.str());
}

void MikeDEM::initHeightWithXTR(char* data) {
	const char FORM_FEED =  0x0c;

	size_t iterator = 0;
	while (data[iterator++] != FORM_FEED) {

	}

// 	float minLng, maxLng, minLat, maxLat;
// 	int numLngs, numLats;
// 
// 	memcpy(&minLng, data + iterator, sizeof(float));
// 	iterator = iterator + sizeof(float);
// 	memcpy(&maxLng, data + iterator, sizeof(float));
// 	iterator = iterator + sizeof(float);
// 	memcpy(&numLngs, data + iterator, sizeof(int));
// 	iterator = iterator + sizeof(int);
// 	memcpy(&minLat, data + iterator, sizeof(float));
// 	iterator = iterator + sizeof(float);
// 	memcpy(&maxLat, data + iterator, sizeof(float));
// 	iterator = iterator + sizeof(float);
// 	memcpy(&numLats, data + iterator, sizeof(int));
// 	iterator = iterator + sizeof(int);
// 
// 	assert(numLngs == NUM_LNGS);
// 	assert(numLats == NUM_LATS);

	vector<float>* hVec = new vector<float>(NUM_LNGS * NUM_LATS, 0.0);

	for (int y = 0; y<NUM_LATS; y++) {
		memcpy( &((*hVec)[y*NUM_LNGS]), data + iterator, sizeof(float)*NUM_LNGS);
		iterator = iterator + sizeof(float)*NUM_LNGS;
	}

	elevationData.reset(hVec);
}
