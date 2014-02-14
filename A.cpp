#include "stdafx.h"
#include "createSHP3D.h"


using boost::filesystem::path;

void main(int argc, char** argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: 2dshpPath outputPath \n");
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
	createSHP3D(argv[1], argv[2], 4096);
// 	createTTTFile("C:/Users/ratanasv/Desktop/tempout/idu3D.ttt", 
// 		"C:/Users/ratanasv/Desktop/tempOut/idu3D.shp");
}


