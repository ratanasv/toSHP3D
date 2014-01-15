#include "stdafx.h"

#include "LatLongElevation.h"
#include <curl/curl.h>
#include <cassert>
#include <sstream>


using std::ostringstream;

static const string MIKEDEM_BASE_URL("http://maverick.coas.oregonstate.edu:11300/terrainextraction.ashx?");
static const string XTR_FILENAME("temp.xtr");

MikeDEM::MikeDEM(float lngMin, float lngMax, float latMin, float latMax,
	int numLngs, int numLats) : NUM_LNGS(numLngs), NUM_LATS(numLats),
	LNG_MIN(lngMin), LNG_MAX(lngMax), LAT_MIN(latMin), LAT_MAX(latMax) {

	shared_ptr<CURL> curl(curl_easy_init(), [](CURL* c){
		curl_easy_cleanup(c);
	});
	CURLcode res;

	if (!curl) {
		throw CurlInitException("curl_easy_init failed");
	}

	auto queryString = createQueryString(MIKEDEM_BASE_URL);
	curl_easy_setopt(curl.get(), CURLOPT_URL, queryString.c_str());
	curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);

	{
		shared_ptr<FILE> xtrFile(fopen(XTR_FILENAME.c_str(), "wb"), [](FILE* f){
			fclose(f);
		});

		if (!xtrFile) {
			throw runtime_error("can't open XTR file");
		}
		curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, xtrFile.get());
		res = curl_easy_perform(curl.get());
	}
	
	if (res != CURLE_OK) {
		throw CurlConnectionException(curl_easy_strerror(res));
	}

	initHeightWithXTR();
}

float MikeDEM::elevAt( float longtitude, float latitude ) {
	double latRange = LAT_MAX-LAT_MIN;
	double lngRange = LNG_MAX-LNG_MIN;
	int latI = (int)( ((latitude-LAT_MIN)/latRange)*((float)NUM_LATS-1.0)  );
	int lngI = (int)( ((longtitude-LNG_MIN)/lngRange)*((float)NUM_LNGS-1.0)  );
	return elevAtIndex(lngI, latI);
}

float MikeDEM::elevAtIndex(int lngI, int latI) {
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

void MikeDEM::initHeightWithXTR () {
	shared_ptr<FILE> xtrFile(fopen(XTR_FILENAME.c_str(), "rb"), [](FILE* f){
		fclose(f);
		remove(XTR_FILENAME.c_str());
	});
	assert(xtrFile.get() != NULL);

	const char FORM_FEED =  0x0c;

	float minLng, maxLng, minLat, maxLat;
	int numLngs, numLats;

	fscanf( xtrFile.get(), "%f %f %d", &minLng, &maxLng, &numLngs );
	fscanf( xtrFile.get(), "%f %f %d", &minLat, &maxLat, &numLats );
	assert(numLngs == NUM_LNGS);
	assert(numLats == NUM_LATS);

	int c;
	do
	{
		c = fgetc( xtrFile.get() );
	} while( c != FORM_FEED );
	// 	c = fgetc( FpIn );                      // the '\n'
	// 	if( c != '\n' )
	// 		fprintf( stderr, "Should have found a '\\n' -- but found '%c'\n", c );

	vector<float>* hVec = new vector<float>(NUM_LNGS * NUM_LATS, 0.0);

	for( int y = 0; y < NUM_LATS; y++ )
		fread( &((*hVec)[y*NUM_LNGS]), sizeof(float), NUM_LNGS, xtrFile.get() );

	elevationData.reset(hVec);
}
