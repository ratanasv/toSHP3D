#include "stdafx.h"
#include "ShapeArray.h"
#include <boost/functional/hash.hpp>
#include "Util.h"

using namespace std;


size_t hash_value(const struct shpmainheader& header) {
	size_t seed = 0;
	boost::hash_combine(seed, header.shapetype);
	boost::hash_combine(seed, header.xmin);
	boost::hash_combine(seed, header.ymin);
	boost::hash_combine(seed, header.ymax);
	boost::hash_combine(seed, header.zmin);
	boost::hash_combine(seed, header.zmax);
	return seed;
}

size_t hash_value(const struct point3d& point) {
	size_t seed = 0;
	boost::hash_combine(seed, point.x);
	boost::hash_combine(seed, point.y);
	boost::hash_combine(seed, point.z);
	return seed;
}

size_t hash_value(const struct shpheader& header) {
	size_t seed = 0;
	boost::hash_combine(seed, header.xmin);
	boost::hash_combine(seed, header.ymin);
	boost::hash_combine(seed, header.xmax);
	boost::hash_combine(seed, header.ymax);
	boost::hash_combine(seed, header.zmin);
	boost::hash_combine(seed, header.zmax);
	boost::hash_combine(seed, header.numparts);
	boost::hash_combine(seed, header.numpoints);
	return seed;
}

size_t hash_value(const VI_Shape& shape) {
	size_t seed = 0;
	boost::hash_combine(seed, shape.ShapeType);
	boost::hash_combine(seed, shape.ShapeHeader);
	boost::hash_range(seed, shape.Parts.begin(), shape.Parts.end());
	boost::hash_range(seed, shape.Vertices->begin(), shape.Vertices->end());
	return seed;
}

VI_ShapeArray::VI_ShapeArray(shared_ptr<vector<VI_Shape>> array,
struct shpmainheader header)
{
	i_array = array;
	i_shpHeader = header;
	i_hashCode = ComputeHashCode();
	i_numVertices = ComputeNumVertices();

}

bool	VI_ShapeArray::HasData() const {
	return (bool)i_array;
}

unsigned	VI_ShapeArray::GetNumShapes() const {
	return i_array->size();
}


const VI_Shape&	VI_ShapeArray::GetValue(unsigned index) const {
	return i_array->at(index);
}

VI_Shape&	VI_ShapeArray::GetValue(unsigned index) {
	return i_array->at(index);
}

struct shpmainheader VI_ShapeArray::GetShpMainHeader() const {
	return i_shpHeader;
}

size_t VI_ShapeArray::ComputeHashCode() {
	size_t seed = 0;
	if (HasData()) {
		boost::hash_range(seed, i_array->begin(), i_array->end());
		boost::hash_combine(seed, i_shpHeader);
	}
	return seed;
}

size_t VI_ShapeArray::GetHashCode() const {
	return i_hashCode;
}

unsigned VI_ShapeArray::GetNumVertices() const {
	return i_numVertices;
}

unsigned VI_ShapeArray::ComputeNumVertices() {
	unsigned total = 0;
	if (HasData()) {
		for (const auto& it : *i_array) {
			total += it.ShapeHeader.numpoints;
		}
	}
	return total;
}
VI_ShapeArray ImportFromSHPFile(const std::string& fileName) {
	auto shapes = initVectorArray<VI_Shape>();
	auto shpHandle = SHPOpen(fileName.c_str(), "rb");
	int numShapes;
	int type;
	SHPGetInfo(shpHandle, &numShapes, &type, NULL, NULL);
	
	shared_ptr<vector<VI_Shape>> shapeArray(new vector<VI_Shape>(numShapes));
	for (int i = 0; i < numShapes; i++) {
		shared_ptr<SHPObject> shpObject(SHPReadObject(shpHandle, i), [](SHPObject* obj) {
			SHPDestroyObject(obj);
		});
		shapeArray->at(i).ShapeHeader.numparts = shpObject->nParts;
		shapeArray->at(i).ShapeHeader.numpoints = shpObject->nVertices;
		shapeArray->at(i).ShapeHeader.xmax = shpObject->dfXMax;
		shapeArray->at(i).ShapeHeader.ymax = shpObject->dfYMax;
		shapeArray->at(i).ShapeHeader.zmax = shpObject->dfZMax;
		shapeArray->at(i).ShapeHeader.xmin = shpObject->dfXMin;
		shapeArray->at(i).ShapeHeader.ymin = shpObject->dfYMin;
		shapeArray->at(i).ShapeHeader.zmin = shpObject->dfZMin;
		shapeArray->at(i).ShapeType = type;
		shapeArray->at(i).Parts.insert(shapeArray->at(i).Parts.begin(),
			shpObject->panPartStart, shpObject->panPartStart + shpObject->nParts);
		shared_ptr<vector<struct point3d>> pointArray(new vector<struct point3d>(shpObject->nVertices));
		for (int j = 0; j < shpObject->nVertices; j++) {
			pointArray->at(j).x = shpObject->padfX[j];
			pointArray->at(j).y = shpObject->padfY[j];
			pointArray->at(j).z = shpObject->padfZ[j];
		}
		shapeArray->at(i).Vertices = pointArray;
	}
	
	struct shpmainheader header;
	header.filecode = -1;
	header.filelength = -1;
	header.mmax = -1;
	header.mmin = -1;
	header.pad = -1;
	header.shapetype = type;
	header.unused0 = -1;
	header.unused1 = -1;
	header.unused2 = -1;
	header.unused3 = -1;
	header.unused4 = -1;
	header.version = -1;
	double* MaxBuffer = new double[4];
	double* MinBuffer = new double[4];
	SHPGetInfo(shpHandle, NULL, NULL, MinBuffer, MaxBuffer);
	header.xmin = MinBuffer[0];
	header.ymin = MinBuffer[1];
	header.zmin = MinBuffer[2];
	header.xmax = MaxBuffer[0];
	header.ymax = MaxBuffer[1];
	header.zmax = MaxBuffer[2];
	delete[] MaxBuffer;
	delete[] MinBuffer;

	SHPClose(shpHandle);

	return VI_ShapeArray(shapeArray, header);
}
