#include "stdafx.h"
#include "SHPToObj.h"
#include "objload.h"
using namespace std;
namespace fs = boost::filesystem;


void SHPToObj(const string& shpPathString, const string& objPath) {
	auto shp_handle = initShapeHandle(SHPOpen(shpPathString.c_str(), "rb"));
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

			for (int k=first; k<last; k++) {
				objModel.vertex.push_back(shape->padfX[k]);
				objModel.vertex.push_back(shape->padfY[k]);
				objModel.vertex.push_back(shape->padfZ[k]);
			}
		}
	}

	const fs::path shpPath(shpPathString);
	fs::path tttPath = shpPath;
	tttPath.replace_extension(".ttt");

	FileGuard tttFile(tttPath.string().c_str(), "rb");
	fseek(tttFile.get(), sizeof(int), SEEK_SET);
	int numTris;
	fread(&numTris, sizeof(int), 1, tttFile.get());

	objModel.faces["foo"] = vector<unsigned short>();
	auto& indices = objModel.faces["foo"];

	for (int i=0; i<numTris; i++) {
		unsigned int buffer;
		fread(&buffer, sizeof(unsigned int), 1, tttFile.get());
		indices.push_back((unsigned short)buffer);
	}

	ofstream outfile(objPath.c_str(),ofstream::binary);
	outfile << objModel;
}
