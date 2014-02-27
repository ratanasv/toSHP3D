#include "stdafx.h"
#include "SHPToObj.h"
#include "objload.h"
using namespace std;
namespace fs = boost::filesystem;


void SHPToObj(const string& shpPathString, const string& objPath) {
	auto shp_handle = initShapeHandle(SHPOpen(shpPathString.c_str(), "rb"));
	int num_shapes;
	double mins[4];
	double maxs[4];
	SHPGetInfo(shp_handle.get(), &num_shapes, NULL, mins, maxs);
	double xcenter = (mins[0] + maxs[0])/2.0;
	double ycenter = (mins[1] + maxs[1])/2.0;
	double zcenter = (mins[2] + maxs[2])/2.0;
	double rad = max(maxs[0]-xcenter,maxs[1]-ycenter);

	vector<float> vertices;
	vector<unsigned> indices;
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

			for (int k=first; k<=last; k++) {
				vertices.push_back((shape->padfX[k] - xcenter)/rad);
				vertices.push_back((shape->padfY[k] - ycenter)/rad);
				vertices.push_back((shape->padfZ[k] - zcenter)/(maxs[2]-zcenter)/10.0);
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

	for (int i=0; i<numTris*3; i++) {
		unsigned int buffer;
		fread(&buffer, sizeof(unsigned int), 1, tttFile.get());
		buffer = buffer + 1; //obj start counting at 1;
		indices.push_back(buffer);
	}

	ofstream outfile(objPath.c_str(), ios::out);
	for (int i=0; i<vertices.size(); i=i+3) {
		outfile << "v" 
			<< " " << vertices[i+0] 
			<< " " << vertices[i+1]
			<< " " << vertices[i+2]
			<< "\n";
	}

	for (int i=0; i<indices.size(); i=i+3) {
		outfile << "f" << " "
			<< " " << indices[i+0] 
			<< " " << indices[i+1]
			<< " " << indices[i+2]
			<< "\n";
	}
}
