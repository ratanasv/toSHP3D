#include "createSHP3D.h"
#include <cstring>
#include <boost/filesystem.hpp>
#include "proj_api.h"

using boost::filesystem::path;
using std::string;

void main(int argc, char** argv)
{
// 	if (argc != 2) {
// 		fprintf(stderr, "usage: A $filepath\n");
// 		exit(EXIT_FAILURE);
// 	}

//	path shpPath(argv[1]);
	path shpPath("C:/Vault/WW2010/IDU.shp");
	if (!exists(shpPath)) {
		fprintf(stderr, "%s doesn't exist\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	shpPath.replace_extension(".prj");
	FILE* ifp = fopen(shpPath.string().c_str(), "r");
	char parg[4096];
	char* in_args[100];

	if ( ifp ) {
		int i=0;
		while( fscanf( ifp, "%s", parg) != EOF ) {
			in_args[i] = (char*)malloc ( strlen(parg)+1 );
			strcpy ( in_args[i], parg);
			i++;
		}

		fclose (ifp);
	}
	projPJ pj = pj_init(1, in_args); 


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