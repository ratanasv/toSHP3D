#include "LatLongElevation.h"
#include <curl/curl.h>
#include <stdexcept>
#include <cassert>
#include <memory>
#include <sstream>

using std::shared_ptr;
using std::ostringstream;

const string MIKEDEM_BASE_URL("http://maverick.coas.oregonstate.edu:11300/terrainextraction.ashx?");

MikeDEM::MikeDEM(float latMin, float latMax,float lngMin, float lngMax, 
	int numLngs, int numLats) : NUM_LNGS(numLngs), NUM_LATS(numLats),
	LNG_MIN(lngMin), LNG_MAX(lngMax), LAT_MIN(latMin), LAT_MAX(latMax) {
	CURL* curl = curl_easy_init();
	CURLcode res;

	if (!curl) {
		throw CurlInitException("curl_easy_init failed");
	}

	auto queryString = createQueryString(MIKEDEM_BASE_URL);
	curl_easy_setopt(curl, CURLOPT_URL, queryString.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	FILE* xtrFile = fopen("./test.xtr", "wb");
	assert(xtrFile != NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, xtrFile);

// 
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "%s\n", curl_easy_strerror(res));
		throw CurlConnectionException(curl_easy_strerror(res));
	}
	fclose(xtrFile);

	curl_easy_cleanup(curl);
}

float MikeDEM::elevAt( float longtitude, float latitude )
{
	return 0.0;
}

string MikeDEM::createQueryString( const string& baseURL ) {
	ostringstream ss;
	ss << MIKEDEM_BASE_URL << "lat1=" << LAT_MIN << "&lat2=" << LAT_MAX <<
		"&lng1=" << LNG_MIN << "&lng2=" << LNG_MAX << "&numlngs=" << NUM_LNGS <<
		"&numlats=" << NUM_LATS;
	return string(ss.str());
}
