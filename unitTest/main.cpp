#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "LatLongElevation.h"

using boost::filesystem::path;


BOOST_AUTO_TEST_CASE(TestLatLongElevation) {
	MikeDEM dem(-122.95, -123.0, 40.95, 41.0, 5, 5);
	BOOST_REQUIRE_CLOSE(dem.elevAt(-122.97, 40.96), 2178.54419, 1.0);
	BOOST_CHECK_THROW(dem.elevAt(40.96, -122.97),  out_of_range);
}


