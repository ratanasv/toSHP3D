#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "stdafx.h"
#include "Bicubic.h"


BOOST_AUTO_TEST_CASE( my_test ) {
	const int resolution = 3;
	TwoDArray<float> bogus(resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			bogus[i][j] = i*resolution + j;
		}
	}
	Bicubic bicubic(bogus.get(), resolution);
	const TwoDArray<float> fx(bicubic.initFX(), resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			BOOST_REQUIRE_EQUAL(fx[i][j], 1.0);
		}
	}

	const TwoDArray<float> fy(bicubic.initFY(), resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			BOOST_REQUIRE_EQUAL(fy[i][j], 3.0);
		}
	}

	const TwoDArray<float> fxy(bicubic.initFXY(fx.get(), fy.get()), 
		resolution, resolution);
	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			BOOST_REQUIRE_EQUAL(fxy[i][j],0.0);
		}
	}

	const TwoDArray<float[4][4]> alphas(bicubic._alpha, resolution-1, resolution-1);
	for (int i=0; i<resolution-1; i++) {
		for (int j=0; j<resolution-1; j++) {
			const auto& a = alphas[i][j];
			BOOST_REQUIRE_CLOSE(bogus[i][j], a[0][0], 10.0);
			BOOST_REQUIRE_CLOSE(bogus[i][j+1], a[0][0] + a[1][0] + a[2][0] + a[3][0], 10.0);
			BOOST_REQUIRE_CLOSE(bogus[i+1][j], a[0][0] + a[0][1] + a[0][2] + a[0][3], 10.0);
			BOOST_REQUIRE_CLOSE(bogus[i+1][j+1], a[0][0] + a[0][1] + a[0][2] + a[0][3] +
				a[1][0] + a[1][1] + a[1][2] + a[1][3] +
				a[2][0] + a[2][1] + a[2][2] + a[2][3] +
				a[3][0] + a[3][1] + a[3][2] + a[3][3], 10.0);
		}
	}

	for (int i=0; i<resolution; i++) {
		for (int j=0; j<resolution; j++) {
			BOOST_REQUIRE_CLOSE(bogus[i][j], bicubic.valueAt(i,j), 10.0);
		}
	}

}