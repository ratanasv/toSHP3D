#include "stdafx.h"
#include "SHPToObj.h"
#include "objload.h"
using namespace std;


void SHPToObj(const string& shpPath, const string& objPath) {
	auto shp_handle = initShapeHandle(SHPOpen(shpPath.c_str(), "rb"));
	int num_shapes;
	SHPGetInfo(shp_handle.get(), &num_shapes, NULL, NULL, NULL);

	obj::Model objModel;
	for (int i=0; i<num_shapes; i++) {
		auto shape = initShape(SHPReadObject(shp_handle.get(), i));
		for (int j=0; j<shape->nParts; j++) {
			int first = shape->panPartStart[j];
			int last;
			if (j ==shape->nParts-1) {
				last = shape->nVertices-1;
			} else {
				last = shape->panPartStart[j+1]-1;
			}

			for (int i=first; i<last; i++) {
				objModel.vertex.push_back(shape->padfX[i]);
				objModel.vertex.push_back(shape->padfY[i]);
				objModel.vertex.push_back(shape->padfZ[i]);
			}
		}
	}


}
