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
 	createTTTFile("C:/Users/ratanasv/Desktop/alexis/idu3D.ttt",
 		"C:/Users/ratanasv/Desktop/alexis/idu3D.shp");
 	SHPToObj("C:/Users/ratanasv/Desktop/alexis/idu3D.shp", 
 		"C:/Users/ratanasv/Desktop/alexis/idu3D.obj");
}

#endif