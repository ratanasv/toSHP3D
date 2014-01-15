#include "stdafx.h"
#include "createSHP3D.h"


using boost::filesystem::path;

void main(int argc, char** argv)
{
// 	if (argc != 2) {
// 		fprintf(stderr, "usage: A $filepath\n");
// 		exit(EXIT_FAILURE);
// 	}

//	path shpPath(argv[1]);
	path shpPath("E:/Vault/WW2100/IDU.shp");
	if (!exists(shpPath)) {
		fprintf(stderr, "%s doesn't exist\n", shpPath.c_str());
		exit(EXIT_FAILURE);
	}

	OGRSpatialReference sourceSRS;
	char* prjWKT = getContent(shpPath.replace_extension(".prj"));
	if (prjWKT == NULL) {
		fprintf(stderr, "reading PRJ file failed");
		exit(EXIT_FAILURE);
	}
	sourceSRS.importFromWkt(&prjWKT);
	OGRSpatialReference targetSRS;
	targetSRS.SetWellKnownGeogCS("WGS84");
	OGRCoordinateTransformation* transformation;

	transformation = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);



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