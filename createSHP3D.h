#pragma once

#include <memory>
#include <vector>
#include "LatLongElevation.h"
#include <cstring>

using std::string;
class boost::filesystem::path;


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


class OGRCoordinateTransformation;

struct ElevationData readXTR(const char* fName);
void createNormalTexture(const char* image, const MikeDEM& dem, 
	const double minX, const double maxX, const double minY, const double maxY, 
	OGRCoordinateTransformation* transformation, const int resolution);

std::shared_ptr<std::vector<double>> getSHPInfo(const char* shpName);
void createTTTFile(const char* tName, const char* shpName);

char* getContent(const boost::filesystem::path& path);
template <class T> shared_ptr<T> initArray(T* data);
void createSHP3D(const char* inSHP, const char* outSHP, const int resolution);
OGRCoordinateTransformation* getTransformation(char* prjWktSource);
std::string stringf(const char* format, ... );
MikeDEM computeMinsMaxs(SHPHandle shpIn, OGRCoordinateTransformation* transformation, 
	double& minLng, double& maxLng, double& minLat, double& maxLat);
void computeCushion(double& minLng, double& maxLng, double& minLat, double& maxLat);