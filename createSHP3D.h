#pragma once

#include <memory>
#include <vector>
#include "LatLongElevation.h"



#if defined A_DLL_EXPORT
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif

struct ElevationData{
	float MinLng, MaxLng,  MinLat, MaxLat;
	int NumLngs, NumLats;
	std::shared_ptr<std::vector<float>> Grid;
};

#define SMART_SHAPE_HANDLE(shp_handle, shp_file) \
	SHPInfo* shp_info = SHPOpen(shp_file,"rb");\
	std::shared_ptr<SHPInfo>shp_handle(shp_info, \
		[] (SHPInfo* s){SHPClose(s);});

#define SMART_SHAPE(shp, shp_handle, i) \
	SHPObject* unmanaged = SHPReadObject(shp_handle.get(), i);\
	std::shared_ptr<SHPObject>shp(unmanaged, \
		[] (SHPObject* s){SHPDestroyObject(s);});

#define SMART_ARRAY(array, type)\
	std::vector<type>* vector = new std::vector<type>;\
	std::shared_ptr<std::vector<type>> array(vector);




struct ElevationData readXTR(const char* fName);
DECLDIR void createSHP3D(const char* latlong, const char* shp3d, const char* xtrN);
DECLDIR void createBMP(const char* name, const char* xtrN);
DECLDIR void createNormalTexture(const char* image, const char* xtr);
DECLDIR void createEvenlySpacedSHP(const char* shpName, const char* xtrName);
DECLDIR void createBMPwithSHP(const char* shpFile, const char* bmpFile, const char* xtrN);
DECLDIR void createBMPwithSHP3D(const char* shpFile, const char* bmpFile, int dim);
DECLDIR std::shared_ptr<std::vector<double>> getSHPInfo(const char* shpName);
DECLDIR void createTTTFile(const char* tName, const char* shpName);
