#include "stdafx.h"
#include "createSHP3D.h"
#include "Exception.h"
#include "ShapeArray.h"
#include "TTTSerializer.h"
#include <boost/log/trivial.hpp>

using namespace vir;
using namespace std;
namespace fs = boost::filesystem;
//global variables
std::shared_ptr<std::vector<double>> giveMeHeights(SHPObject* shpObjIn, struct ElevationData);


void HSVtoRGB(float hsv[3], float rgb[3])
{
	float tmp1 = hsv[2] * (1-hsv[1]);
	float tmp2 = hsv[2] * (1-hsv[1] * (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) ));
	float tmp3 = hsv[2] * (1-hsv[1] * (1 - (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) )));
	switch((int)(hsv[0] / 60)) {
	case 0:
		rgb[0] = hsv[2] ;
		rgb[1] = tmp3 ;
		rgb[2] = tmp1 ;
		break;
	case 1:
		rgb[0] = tmp2 ;
		rgb[1] = hsv[2] ;
		rgb[2] = tmp1 ;
		break;
	case 2:
		rgb[0] = tmp1 ;
		rgb[1] = hsv[2] ;
		rgb[2] = tmp3 ;
		break;
	case 3:
		rgb[0] = tmp1 ;
		rgb[1] = tmp2 ;
		rgb[2] = hsv[2] ;
		break;
	case 4:
		rgb[0] = tmp3 ;
		rgb[1] = tmp1 ;
		rgb[2] = hsv[2] ;
		break;
	case 5:
		rgb[0] = hsv[2] ;
		rgb[1] = tmp1 ;
		rgb[2] = tmp2 ;
		break;
	default:
		assert(1 == 0);
		break;
	}

}

struct ElevationData readXTR(const char* fName)
{
	struct ElevationData i_elevData;
	FILE* FpIn = fopen(fName,"rb");
	assert(FpIn!=NULL);
	const char FORM_FEED =  0x0c;


	fscanf( FpIn, "%f %f %d", &i_elevData.MinLng, &i_elevData.MaxLng, &i_elevData.NumLngs );
	fscanf( FpIn, "%f %f %d", &i_elevData.MinLat, &i_elevData.MaxLat, &i_elevData.NumLats );

	int c;
	do
	{
		c = fgetc( FpIn );
	} while( c != FORM_FEED );
// 	c = fgetc( FpIn );                      // the '\n'
// 	if( c != '\n' )
// 		fprintf( stderr, "Should have found a '\\n' -- but found '%c'\n", c );

	vector<float>* hVec = new vector<float>(i_elevData.NumLngs * i_elevData.NumLats,3.14);


	for( int y = 0; y < i_elevData.NumLats; y++ )
		fread( &((*hVec)[y*i_elevData.NumLngs]), sizeof(float), i_elevData.NumLngs, FpIn );
	
	fclose(FpIn);
	i_elevData.Grid = shared_ptr<vector<float>>(hVec);
	return i_elevData;
}


void createNormalTexture(const char* image, const MikeDEM& dem, 
	const double minX, const double maxX, const double minY, const double maxY, 
	OGRCoordinateTransformation* transformation, const int resolution)
{
	using Angel::vec3;

	shared_ptr<vector<unsigned char>> normals(
		new vector<unsigned char>(resolution*resolution*3, (unsigned char)0));

	double dx = (maxX-minX)/(double)resolution;
	double dy = (maxY-minY)/(double)resolution;

	for (int i=1; i<resolution-1; i++) {
		for (int j=1; j<resolution-1; j++) {
			double centerY = minY + i*dy;
			double centerX = minX + j*dx;

			double up = centerY + dy;
			double right = centerX + dx;
			double down = centerY - dy;
			double left = centerX - dx;

			transformation->Transform(1, &centerX, &centerY, NULL);
			transformation->Transform(1, &right, &up, NULL);
			transformation->Transform(1, &left, &down, NULL);

			vec3 horizontal(2.0, 0.0, dem.elevAt(right, centerY) - 
				dem.elevAt(left, centerY));
			vec3 vertical(0.0, 2.0, dem.elevAt(centerX, up) - 
				dem.elevAt(centerX, down));
			vec3 c = normalize(cross(horizontal,vertical));
			normals->at(resolution*3*i+3*j+0) = 255.0*(0.5*c.x+0.5);
			normals->at(resolution*3*i+3*j+1) = 255.0*(0.5*c.y+0.5);
			normals->at(resolution*3*i+3*j+2) = 255.0*(0.5*c.z+0.5);
		}
	}

	normals = flipVertically(normals,resolution,3);

	int result = SOIL_save_image
		(
		image,
		SOIL_SAVE_TYPE_BMP,
		resolution, resolution, 3,
		normals->data()
		);
	assert(result == 1);
}


shared_ptr<vector<double>> getSHPInfo(const char* shpName)
{
	FILE* fp = fopen(shpName, "rb");
	fseek(fp, 32, SEEK_SET);
	int shapeType;
	double bounds[6];
	int count = fread(&shapeType, sizeof(int), 1, fp);
	count = fread(bounds, sizeof(double), 6, fp);
	fclose(fp);
	
	

	shared_ptr<vector<double>> MinsMaxs(new vector<double>);
	SHPHandle shpIn = SHPOpen(shpName,"rb");
	assert(shpIn != NULL);
	double mins[4], maxs[4];
	int numShapes, shpType;
	SHPGetInfo(shpIn,&numShapes,&shpType,mins,maxs);
	for(int i= 0 ;i<4; i++){
		assert(i<4 && i>=0);
		MinsMaxs->push_back(mins[i]);
		MinsMaxs->push_back(maxs[i]);
	}
	MinsMaxs->push_back((double)numShapes);
	MinsMaxs->push_back((double)shpType);

	for (int i =0; i<numShapes;i++) {
		SHPObject* handle = SHPReadObject(shpIn, i);
		fprintf(stderr, "%lf\n", handle->dfZMin);

	}
	SHPClose(shpIn);



	
	return MinsMaxs;
}

static unsigned GetIndexFromPoint(TPPLPoint& Point, VI_Shape& shp) {
	float minDiff = FLT_MAX;
	unsigned whichVertex = 0;

	for (int i = 0; i < shp.ShapeHeader.numpoints; i++) {
		const float error = pow(Point.x - shp.Vertices->at(i).x, 2) +
			pow(Point.y - shp.Vertices->at(i).y, 2);
		if (error < minDiff) {
			minDiff = error;
			whichVertex = (unsigned)i;
		}
	}
	return whichVertex;
}

static unsigned getIndexFromListPoly(TPPLPoint& Point,
	VI_Shape& shp)
{
	float EPS = 0.000000000000001;
	for (int i = 0; i < shp.ShapeHeader.numpoints; i++)//search thru to find what we want
	if (abs(Point.x - shp.Vertices->at(i).x) < EPS)
		if (abs(Point.y - shp.Vertices->at(i).y) < EPS)
			return (unsigned)i;
	return 0;
}

void createTTTFile(const char* tName, const char* shpName)
{
	auto shapeArray = ImportFromSHPFile(shpName);
	const int num_shapes = shapeArray.GetNumShapes();
	auto buffer = initVectorArray<unsigned>();
	
	unsigned pC = 0;//accumulative pointer counter
	for(int i=0; i<num_shapes;i++) {
		auto& shape = shapeArray.GetValue(i);
		auto result = initVectorArray<unsigned>();
		list<TPPLPoly> listPoly, listResult;
		for(int j = 0; j<shape.Parts.size();j++){
			int first = shape.Parts[j];
			int last;
			if (j == shape.Parts.size()-1)
				last = shape.Vertices->size() -1;
			else
				last = shape.Parts[j+1]-1;
			TPPLPoly poly;
			poly.Init(last-first+1);
			unsigned c=0;
			assert(shape.Vertices->at(first).x == shape.Vertices->at(last).x);
			for (int k = first; k<=last; k++) {
				poly[c].x = shape.Vertices->at(k).x;
				poly[c].y = shape.Vertices->at(k).y;
				c++;
			}
			if (poly.GetOrientation() == TPPL_CW) {
				poly.SetHole(false);
			} else {
				poly.SetHole(true);
			}
			poly.SetOrientation(TPPL_CCW);
			listPoly.push_back(poly);	
		}
		TPPLPartition pp;
		pp.Triangulate_EC(&listPoly, &listResult);
		for(auto iter = listResult.begin(); iter!=listResult.end(); iter++){
			unsigned index;
			index = getIndexFromListPoly((*iter)[0],shape);//v1
			buffer->push_back(pC+index);
			index = getIndexFromListPoly((*iter)[1],shape);//v2
			buffer->push_back(pC+index);
			index = getIndexFromListPoly((*iter)[2],shape);//v3
			buffer->push_back(pC+index);
		}
		pC += shape.ShapeHeader.numpoints;
	}
	unsigned accumulative = 0;
	int total_tris = 0;
	fprintf(stderr, "%u\n", (unsigned)buffer->size()/3);
	ofstream tttFile(tName, ios_base::trunc | ios_base::binary);
	boost::archive::binary_oarchive oa(tttFile);
	oa << TTTSerializer(shapeArray.GetHashCode(), buffer);
}

shared_ptr<char> getContent(const fs::path& path) {
	shared_ptr<FILE> file (fopen(path.string().c_str(), "rb"), [](FILE* fp) {
		fclose(fp);
	});

	if (file.get() == NULL) {
		return NULL;
	}

	fseek(file.get(), 0L, SEEK_END);
	long size = ftell(file.get());

	fseek(file.get(), 0L, SEEK_SET);
	auto buffer = initCStyleArray(new char[size+1]);
	fread(buffer.get(), 1, size, file.get());

	(buffer.get())[size] = '\0';

	return buffer;
}

void copyFile(const fs::path& in, const fs::path& out) {
	if (!exists(in)) {
		stringstream ss;
		ss << in.string().c_str() << " doesn't exist";
		throw FileNotFoundException(ss.str());
	}

	ifstream source(in.string().c_str(), ios::binary);
	ofstream dest(out.string().c_str(), ios::binary);

	dest << source.rdbuf();

	source.close();
	dest.close();
}

string stringf(const char* format, ... ) {
	va_list	args;
	va_start(args, format);
	int	size = vsnprintf(NULL, 0, format, args);
	unique_ptr<char> buffer(new char[size + 1]);
	vsnprintf(buffer.get(), size + 1, format, args);
	va_end(args);
	string result(buffer.get());
	return result;
}

OGRCoordinateTransformation* getTransformation(shared_ptr<char> prjWktSource) {

	OGRSpatialReference sourceSRS;
	char* rawPtr = prjWktSource.get();
	
	OGRErr result = sourceSRS.importFromWkt(&rawPtr);
	if (result != OGRERR_NONE) {
		throw OGRTransformationError("unable to parse WKT");
	}
	OGRSpatialReference targetSRS;
	result = targetSRS.SetWellKnownGeogCS("WGS84");
	if (result != OGRERR_NONE) {
		throw OGRTransformationError("unable to obtain WGS84 projection");
	}
	

	auto transformation = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);

	if (transformation == NULL) {
		throw OGRTransformationError("unable to determine the transformation");
	}

	return transformation;
}

void computeMinsMaxs(SHPHandle shpIn, OGRCoordinateTransformation* transformation,
	double& minLng, double& maxLng, double& minLat, double& maxLat) 
{
	double mins[4], maxs[4];
	int numShapes;
	SHPGetInfo(shpIn,&numShapes,NULL,mins,maxs);

	minLat = FLT_MAX; minLng = FLT_MAX;
	maxLat = -999999999.0; maxLng = -999999999.0;

	for (int i=0; i<numShapes; i++) {//for each shape
		SHPObject* shpObjIn = SHPReadObject(shpIn, i);
		for (int j = 0; j<shpObjIn->nVertices; j++) {
			double x = shpObjIn->padfX[j];
			double y = shpObjIn->padfY[j];

			bool isSuccessful = transformation->Transform(1, &x, &y);

			if (!isSuccessful) {
				throw runtime_error(stringf("failed to project shp no. %d point no. %d\n", i, j));
			}

			if (x < minLng) {
				minLng = x;
			}
			if (x > maxLng) {
				maxLng = x;
			}
			if (y < minLat) {
				minLat = y;
			}
			if (y > maxLat) {
				maxLat = y;
			}
		}
		SHPDestroyObject(shpObjIn);
	}

	double xmin = mins[0];
	double ymin = mins[1];
	double xmax = maxs[0];
	double ymax = maxs[1];
	transformation->Transform(1, &xmin, &ymin);
	transformation->Transform(1, &xmax, &ymax);
	/*if (xmin < minLng) {
		minLng = xmin;
	}
	if (xmax > maxLng) {
		maxLng = xmax;
	}
	if (ymin < minLat) {
		minLat = ymin;
	}
	if (ymax > maxLat) {
		maxLat = ymax;
	}*/

}

void computeCushion(double& minLng, double& maxLng, double& minLat, double& maxLat) {
	double cushionX = (maxLng-minLng)*0.1; 
	double cushionY = (maxLat-minLat)*0.1;
	minLng = minLng - cushionX;
	maxLng = maxLng + cushionX;
	minLat = minLat - cushionY;
	maxLat = maxLat + cushionY;
}

void createSHP3D(const char* inSHP, const char* outSHP, const int resolution) {
	fs::path inputShpPath(inSHP);
	if (!exists(inputShpPath)) {
		throw FileNotFoundException(inputShpPath.string() + "doesn't exist");
	}
	bool isValidSHPFile = validateSHP(inSHP);
	if (!isValidSHPFile) {
		BOOST_LOG_TRIVIAL(fatal) << inSHP << " is not valid.";
		exit(EXIT_FAILURE);
	}

	fs::path inPrjPath = inputShpPath;
	inPrjPath.replace_extension(".prj");
	auto prjWKTBuffer = getContent(inPrjPath);

	BOOST_LOG_TRIVIAL(info) << "determining transformation....\n";
	OGRCoordinateTransformation* transformation = getTransformation(prjWKTBuffer);

	SHPHandle shpIn = SHPOpen(inputShpPath.string().c_str(),"rb");
	double minLng, maxLng, minLat, maxLat;
	computeMinsMaxs(shpIn, transformation, minLng, maxLng, minLat, maxLat);
	computeCushion(minLng, maxLng, minLat, maxLat);
	MikeDEM demHeightField(minLng, maxLng, minLat, maxLat, resolution, resolution);

	

	BOOST_LOG_TRIVIAL(info) << "creating 3D shapefile....";
	SHPHandle shpOut = SHPCreate(outSHP,SHPT_POLYGONZ);

	double xMin = FLT_MAX; double xMax = -FLT_MAX;
	double yMin = FLT_MAX; double yMax = -FLT_MAX;
	double mins[4], maxs[4];
	int numShapes;
	SHPGetInfo(shpIn, &numShapes, NULL, mins, maxs);

	for (int i=0; i<numShapes; i++) {
		SHPObject* shpObjIn = SHPReadObject(shpIn, i);

		const int numVertices = shpObjIn->nVertices;
		double* xVertices = new double[numVertices];
		double* yVertices = new double[numVertices];
		memcpy(xVertices, shpObjIn->padfX, sizeof(double)*numVertices);
		memcpy(yVertices, shpObjIn->padfY, sizeof(double)*numVertices);

		for (int j=0; j<numVertices; j++) {
			if (xVertices[j] < xMin) {
				xMin = xVertices[j];
			}
			if (xVertices[j] > xMax) {
				xMax = xVertices[j];
			}
			if (yVertices[j] < yMin) {
				yMin = yVertices[j];
			}
			if (yVertices[j] > yMax) {
				yMax = yVertices[j];
			}
		}

		bool isSuccessful = transformation->Transform(
			numVertices, xVertices, yVertices);

		if (!isSuccessful) {
			BOOST_LOG_TRIVIAL(error) << "failed to project shp no. " << i << 
				" unable to proceed.";
			exit(EXIT_FAILURE);
		}

		double* heights = new double[shpObjIn->nVertices];

		for (int j=0; j<shpObjIn->nVertices; j++) {
			heights[j] = demHeightField.elevAt(xVertices[j], yVertices[j]);
		}

		delete[] xVertices;
		delete[] yVertices;

		SHPObject* shpObjOut = SHPCreateObject(
			SHPT_POLYGONZ,
			shpObjIn->nShapeId,
			shpObjIn->nParts,
			shpObjIn->panPartStart,
			shpObjIn->panPartType,
			shpObjIn->nVertices,
			shpObjIn->padfX,
			shpObjIn->padfY,
			heights,
			NULL);

		delete[] heights;

		int entityNumber = SHPWriteObject(shpOut,-1, shpObjOut);
		SHPDestroyObject(shpObjOut);
		SHPDestroyObject(shpObjIn);
	}

	SHPClose(shpIn);
	SHPClose(shpOut);

	if (xMin > mins[0]) {
		BOOST_LOG_TRIVIAL(warning) << "xmin in shpheader is too small";
	}
	if (xMax < maxs[0]) {
		BOOST_LOG_TRIVIAL(warning) << "xmax in shpheader is too big";
	}
	if (yMin > mins[1]) {
		BOOST_LOG_TRIVIAL(warning) << "ymin in shpheader is too small";
	}
	if (yMax < maxs[1]) {
		BOOST_LOG_TRIVIAL(warning) << "ymax in shpheader is too big";
	}

	fs::path outPrjPath(outSHP);
	outPrjPath.replace_extension(".prj");
	if (exists(inPrjPath)) {
		BOOST_LOG_TRIVIAL(info) << "copying " << inPrjPath;
		copyFile(inPrjPath, outPrjPath);
	} else {
		BOOST_LOG_TRIVIAL(warning) << stringf("%s doesn't exist", 
			inPrjPath.string().c_str());
	}
	

	fs::path outDbfPath(outSHP);
	outDbfPath.replace_extension(".dbf");
	fs::path inDbfPath(inSHP);
	inDbfPath.replace_extension(".dbf");
	if (exists(inDbfPath)) {
		BOOST_LOG_TRIVIAL(info) << "copying " << inDbfPath;
		copyFile(inDbfPath, outDbfPath);
	} else {
		BOOST_LOG_TRIVIAL(warning) << stringf("%s doesn't exist", 
			inDbfPath.string().c_str());
	}

	fs::path outXmlPath(outSHP);
	outXmlPath.replace_extension(".xml");
	fs::path inXmlPath(inSHP);
	inXmlPath.replace_extension(".xml");
	if (exists(inXmlPath)) {
		BOOST_LOG_TRIVIAL(info) << "copying " << inXmlPath;
		copyFile(inXmlPath, outXmlPath);
	} else {
		BOOST_LOG_TRIVIAL(warning) << stringf("%s doesn't exist", 
			inXmlPath.string().c_str());
	}

	BOOST_LOG_TRIVIAL(info) << "triangulating ";
	fs::path outTttPath(outSHP);
	outTttPath.replace_extension(".ttt");
	createTTTFile(outTttPath.string().c_str(), inSHP);

	string normalsPathString(outSHP);
	auto loc0 = normalsPathString.find(".");
	normalsPathString.replace(loc0, string::npos, "Normals.bmp");
	
	BOOST_LOG_TRIVIAL(info) << "creating bumpmapping ";
	createNormalTexture(normalsPathString.c_str(), demHeightField, xMin, xMax,
		yMin, yMax, transformation, resolution);
}

bool validateSHP(const string& shpIn) {
	fs::path inputShpPath(shpIn);
	if (!exists(inputShpPath)) {
		throw FileNotFoundException(inputShpPath.string() + " doesn't exist");
	}
	shared_ptr<SHPInfo> shpHandleSmartRef(SHPOpen(inputShpPath.string().c_str(),"rb"),
		[](SHPHandle handle) {
			SHPClose(handle);
		}
	);
	auto shpHandle = shpHandleSmartRef.get();

	double mins[4], maxs[4];
	int numShapes;
	SHPGetInfo(shpHandle, &numShapes, NULL, mins, maxs);

	for (int i=0; i<numShapes; i++) {
		shared_ptr<SHPObject> shpObjHandleSmartRef(SHPReadObject(shpHandle, i), 
			[](SHPObject* obj){ 
				SHPDestroyObject(obj);
			}
		);
		auto shpObjHandle = shpObjHandleSmartRef.get();
		for (int j=0; j<shpObjHandle->nVertices; j++) {
			if (shpObjHandle->padfX[j] < mins[0]) {
				BOOST_LOG_TRIVIAL(error) << "boundary error.";
				return false;
			}
			if (shpObjHandle->padfX[j] > maxs[0]) {
				BOOST_LOG_TRIVIAL(error) << "boundary error.";
				return false;
			}
			if (shpObjHandle->padfY[j] < mins[1]) {
				BOOST_LOG_TRIVIAL(error) << "boundary error.";
				return false;
			}
			if (shpObjHandle->padfY[j] > maxs[1]) {
				BOOST_LOG_TRIVIAL(error) << "boundary error.";
				return false;
			}
		}
	}
	return true;
}

