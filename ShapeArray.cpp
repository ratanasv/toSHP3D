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
	auto shapes = InitVectorArray<VI_Shape>();
	auto shpHandle = initShapeHandle(SHPOpen(fileName.c_str(), "rb"));
	int numShapes;
	SHPGetInfo(shpHandle.get(), &numShapes, NULL, NULL, NULL);
	
	for (int i=0; i<numShapes; i++) {
		
	}
}
