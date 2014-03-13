#include "stdafx.h"
#include "createSHP3D.h"
#include "Bicubic.h"
#include "SHPToObj.h"

#ifndef TEST
using boost::filesystem::path;



void main(int argc, char** argv) {
/*
	if (argc != 4) {
		fprintf(stderr, "usage: 2dshpPath outputPath resolution\n");
		exit(EXIT_FAILURE);
	}
	int resolution = atoi(argv[3]);
	if (resolution == 0) {
		fprintf(stderr, "resolution not a number");
		exit(EXIT_FAILURE);
	}

	createSHP3D(argv[1], argv[2], resolution);*/
//  	createTTTFile("E:/Vault/centralOregon/idu3D.ttt",
//  		"E:/Vault/centralOregon/idu3D.shp");
// 	createTTTFile("C:/Users/ratanasv/Desktop/bicubic/idu3D.ttt", 
// 		"C:/Users/ratanasv/Desktop/bicubic/idu3D.shp");
	createTTTFile("E:/Vault/big_wood_basin/idu3D.ttt",
		"E:/Vault/big_wood_basin/idu3D.shp");
}

#endif