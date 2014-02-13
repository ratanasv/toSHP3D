#include "stdafx.h"
#include "createSHP3D.h"


using boost::filesystem::path;

void main(int argc, char** argv)
{
// 	if (argc != 5) {
// 		fprintf(stderr, "usage: xtrPath bumpMappingPath shp3dPath tttPath \n");
// 		exit(EXIT_FAILURE);
// 	}
// 
// 	path xtrPath(argv[1]);
// 	path bumpmappingPath(argv[2]);
// 	path shp3dPath(argv[3]);
// 	path tttPath(argv[4]);
// 
// 	createNormalTexture(bumpmappingPath.string().c_str(), xtrPath.string().c_str());
// 	createTTTFile(tttPath.string().c_str(), shp3dPath.string().c_str());
	createSHP3D(argv[1], argv[2], 2048);
 //	createBMP("EasternOregon.bmp", "EasternOregonSnugFit.xtr");
 //	createSHP3D("iduLatLong.shp", "idu3D.shp", "EasternOregonLooseFit.xtr");
//	createNormalTexture("iduNormal.bmp", "EasternOregonSnugFit.xtr");
 	//createEvenlySpacedSHP("iduSimple.shp", 2048,2048,
 	//-123.374912, 43.874952, -122.749929, 44.199222);
//	createBMPwithSHP("iduSimple.shp", "testBmp.bmp", "EugeneTerrainLooseFit.xtr");
//	createBMPwithSHP3D("idu3D.shp", "EasternOregon.bmp");
//	createTTTFile("idu.ttt", "idu.shp");
//	MikeDEM(40.95,41.0,-122.95,-123.0,5,5);
}


