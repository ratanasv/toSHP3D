#include "stdafx.h"
#include "createSHP3D.h"
#include "Bicubic.h"

#ifndef TEST
using boost::filesystem::path;



void main(int argc, char** argv) {
	if (argc != 4) {
		fprintf(stderr, "usage: 2dshpPath outputPath resolution\n");
		exit(EXIT_FAILURE);
	}
	int resolution = atoi(argv[3]);
	if (resolution == 0) {
		fprintf(stderr, "resolution not a number");
		exit(EXIT_FAILURE);
	}
//
// 	path xtrPath(argv[1]);
// 	path bumpmappingPath(argv[2]);
// 	path shp3dPath(argv[3]);
// 	path tttPath(argv[4]);
// 
// 	createNormalTexture(bumpmappingPath.string().c_str(), xtrPath.string().c_str());
// 	createTTTFile(tttPath.string().c_str(), shp3dPath.string().c_str());
	createSHP3D(argv[1], argv[2], resolution);
// 	createTTTFile("C:/Users/ratanasv/Desktop/tempout/idu3D.ttt", 
// 		"C:/Users/ratanasv/Desktop/tempOut/idu3D.shp");

}

#endif