#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "stdafx.h"
#include "Bicubic.h"


BOOST_AUTO_TEST_CASE( my_test ) {
	const int resolution = 256;
	MultiArray<float> bogus(resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			bogus[i][j] = i;
		}
	}
	Bicubic bicubic(bogus.get(), resolution);
	const MultiArray<float> fx(bicubic.initFX(), resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			BOOST_CHECK_EQUAL(fx[i][j], 0.0);
		}
	}

	const MultiArray<float> fy(bicubic.initFY(), resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			BOOST_CHECK_EQUAL(fy[i][j], 1.0);
		}
	}

	const MultiArray<float> fxy(bicubic.initFXY(fx.get(), fy.get()), 
		resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			BOOST_CHECK_EQUAL(fxy[i][j],0.0);
		}
	}



}