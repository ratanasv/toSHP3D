#include "stdafx.h"
#include "createSHP3D.h"
#include "Bicubic.h"
#include "SHPToObj.h"
#include <boost/log/trivial.hpp>

#ifndef TEST
using boost::filesystem::path;



void main(int argc, char** argv) {

	if (argc != 4) {
		BOOST_LOG_TRIVIAL(fatal) << "usage: 2dshpPath outputPath resolution";
		exit(EXIT_FAILURE);
	}
	int resolution = atoi(argv[3]);
	if (resolution == 0) {
		BOOST_LOG_TRIVIAL(fatal) << "resolution not a number";
		exit(EXIT_FAILURE);
	}

	createSHP3D(argv[1], argv[2], resolution);

// 	createTTTFile("C:/Users/ratanasv/Desktop/bicubic/idu3D.ttt", 
// 		"C:/Users/ratanasv/Desktop/bicubic/idu3D.shp");
// 	createTTTFile("E:/Vault/big_wood_basin/idu3D.ttt",
// 		"E:/Vault/big_wood_basin/idu3D.shp");
}

#endif