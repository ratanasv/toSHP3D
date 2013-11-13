#include "createSHP3D.h"

void main(int argc, char** argv)
{
 //	createBMP("EasternOregon.bmp", "EasternOregonSnugFit.xtr");
 //	createSHP3D("iduLatLong.shp", "idu3D.shp", "EasternOregonLooseFit.xtr");
//	createNormalTexture("iduNormal.bmp", "EasternOregonSnugFit.xtr");
 	//createEvenlySpacedSHP("iduSimple.shp", 2048,2048,
 	//-123.374912, 43.874952, -122.749929, 44.199222);
//	createBMPwithSHP("iduSimple.shp", "testBmp.bmp", "EugeneTerrainLooseFit.xtr");
//	createBMPwithSHP3D("idu3D.shp", "EasternOregon.bmp");
//	createTTTFile("idu.ttt", "idu.shp");
	MikeDEM(40.95,41.0,-122.95,-123.0,5,5);
}