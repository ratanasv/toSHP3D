#include "stdafx.h"
#include "createSHP3D.h"

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

	std::vector<float>* hVec = new std::vector<float>(i_elevData.NumLngs * i_elevData.NumLats,3.14);


	for( int y = 0; y < i_elevData.NumLats; y++ )
		fread( &((*hVec)[y*i_elevData.NumLngs]), sizeof(float), i_elevData.NumLngs, FpIn );
	
	fclose(FpIn);
	i_elevData.Grid = std::shared_ptr<std::vector<float>>(hVec);
	return i_elevData;
}


DECLDIR void createNormalTexture(const char* image, const MikeDEM& dem, 
	const double minX, const double maxX, const double minY, const double maxY, 
	OGRCoordinateTransformation* transformation, const int resolution)
{
	using Angel::vec3;

	shared_ptr<std::vector<unsigned char>> normals(
		new vector<unsigned char>(resolution*resolution*3, (unsigned char)0));

	double dx = (maxX-minX)/(double)resolution;
	double dy = (maxY-minY)/(double)resolution;

	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
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
#pragma omp barrier
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


DECLDIR std::shared_ptr<std::vector<double>> getSHPInfo(const char* shpName)
{
	FILE* fp = fopen(shpName, "rb");
	fseek(fp, 32, SEEK_SET);
	int shapeType;
	double bounds[6];
	int count = fread(&shapeType, sizeof(int), 1, fp);
	count = fread(bounds, sizeof(double), 6, fp);
	fclose(fp);
	
	

	std::shared_ptr<std::vector<double>> MinsMaxs(new std::vector<double>);
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

static unsigned getIndexFromListPoly(TPPLPoint& Point,
	std::shared_ptr<SHPObject>& shp)
{
	float EPS = 0.000000000000001;
	for(int i = 0; i<shp->nVertices; i++)//search thru to find what we want
		if(abs(Point.x - shp->padfX[i]) < EPS)
			if(abs(Point.y - shp->padfY[i]) < EPS)
				return (unsigned) i;
	return 0;
}

DECLDIR void createTTTFile(const char* tName, const char* shpName)
{
	SMART_SHAPE_HANDLE(shp_handle, shpName)
	int num_shapes;
	SHPGetInfo(shp_handle.get(), &num_shapes, NULL, NULL, NULL);
	vector<shared_ptr<vector<unsigned>>> results(num_shapes, 
		shared_ptr<vector<unsigned>>(NULL));
	
	for(int i =0; i<num_shapes;i++) {
		SMART_SHAPE(shape, shp_handle, i);
		SMART_ARRAY(result, unsigned);
		list<TPPLPoly> listPoly, listResult;
		for(int j = 0; j<shape->nParts;j++){
			int first = shape->panPartStart[j];
			int last;
			if(j ==shape->nParts-1)
				last = shape->nVertices-1;
			else
				last = shape->panPartStart[j+1]-1;
			TPPLPoly poly;
			poly.Init(last-first+1);
			unsigned c=0;
			for(int i = first; i<= last; i++){
				poly[c].x = shape->padfX[i];
				poly[c].y = shape->padfY[i];
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
			result->push_back(index);
			index = getIndexFromListPoly((*iter)[1],shape);//v2
			result->push_back(index);
			index = getIndexFromListPoly((*iter)[2],shape);//v3
			result->push_back(index);
		}
		results.at(i) = result;
	}
	unsigned accumulative = 0;
	int total_tris = 0;
	FILE* tri_file = fopen(tName, "wb");
	fwrite(&num_shapes, sizeof(int), 1, tri_file);
	fwrite(&num_shapes, sizeof(int), 1, tri_file);
	for(int i=0; i<num_shapes; i++){
		auto result = results.at(i);
		SMART_SHAPE(shape, shp_handle, i)
		for(int j=0; j<result->size(); j++){
			unsigned index = result->at(j)+accumulative;
			fwrite(&index, sizeof(unsigned), 1, tri_file);
		}
		total_tris += result->size()/3;
		accumulative += shape->nVertices;
	}
	fseek(tri_file, sizeof(int), SEEK_SET);
	fwrite(&total_tris, sizeof(int), 1, tri_file);
	fclose(tri_file);
}

char* getContent(const fs::path& path) {
	shared_ptr<FILE> file (fopen(path.string().c_str(), "rb"), [](FILE* fp) {
		fclose(fp);
	});

	if (file.get() == NULL) {
		return NULL;
	}

	fseek(file.get(), 0L, SEEK_END);
	long size = ftell(file.get());

	fseek(file.get(), 0L, SEEK_SET);
	char* buffer = new char[size+1];
	fread(buffer, 1, size, file.get());

	buffer[size] = '\0';

	return buffer;
}



template <class T> shared_ptr<T> initArray(T* data) {
	return shared_ptr<T>(data, [](T* ptr) {
		delete[] ptr;
	});
}

void copyFile(const fs::path& in, const fs::path& out) {
	if (!exists(in)) {
		fprintf(stderr, "%s doesn't exist\n", in.string().c_str());
		exit(EXIT_FAILURE);
	}

	ifstream source(in.string().c_str(), ios::binary);
	ofstream dest(out.string().c_str(), ios::binary);

	dest << source.rdbuf();

	source.close();
	dest.close();
}

void createSHP3D(const char* inSHP, const char* outSHP, const int resolution) {
	fs::path inputShpPath(inSHP);
	if (!exists(inputShpPath)) {
		fprintf(stderr, "%s doesn't exist\n", inputShpPath.string().c_str());
		exit(EXIT_FAILURE);
	}
	fs::path inPrjPath = inputShpPath;
	inPrjPath.replace_extension(".prj");

	fprintf(stderr, "determining transformation....\n");
	OGRSpatialReference sourceSRS;
	char* prjWKT = getContent(inPrjPath);
	OGRErr result = sourceSRS.importFromWkt(&prjWKT);
	if (result != OGRERR_NONE) {
		fprintf(stderr, "unable to parse WKT");
		exit(EXIT_FAILURE);
	}
	OGRSpatialReference targetSRS;
	result = targetSRS.SetWellKnownGeogCS("WGS84");
	if (result != OGRERR_NONE) {
		fprintf(stderr, "unable to obtain WGS84 projection");
		exit(EXIT_FAILURE);
	}
	OGRCoordinateTransformation* transformation;

	transformation = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);

	if (transformation == NULL) {
		fprintf(stderr, "unable to determine the transformation");
		exit(EXIT_FAILURE);
	}


	fs::path inSHPPath(inSHP);
	SHPHandle shpIn = SHPOpen(inSHPPath.string().c_str(),"rb");
	double mins[4], maxs[4];
	int numShapes;
	SHPGetInfo(shpIn,&numShapes,NULL,mins,maxs);

	double minLat, maxLat, minLng, maxLng;
	minLat = FLT_MAX; minLng = FLT_MAX;
	maxLat = -999999.0; maxLng = -999999.0;

	for (int i=0; i<numShapes; i++) {//for each shape
		SHPObject* shpObjIn = SHPReadObject(shpIn, i);
		for (int j = 0; j<shpObjIn->nVertices; j++) {
			double x = shpObjIn->padfX[j];
			double y = shpObjIn->padfY[j];

			bool isSuccessful = transformation->Transform(1, &x, &y);

			if (!isSuccessful) {
				fprintf(stderr, "failed to project shp no. %d point no. %d\n", i, j);
				exit(EXIT_FAILURE);
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
	double cushionX = (maxLng-minLng)*0.1; 
	double cushionY = (maxLat-minLat)*0.1;

	MikeDEM demHeightField(minLng - cushionX, maxLng + cushionX, minLat - cushionY, 
		maxLat + cushionY, resolution, resolution);


	fprintf(stderr, "creating 3D shapefile.... \n");
	SHPHandle shpOut = SHPCreate(outSHP,SHPT_POLYGONZ);


	for (int i=0; i<numShapes; i++) {
		SHPObject* shpObjIn = SHPReadObject(shpIn, i);

		const int numVertices = shpObjIn->nVertices;
		double* xVertices = new double[numVertices];
		double* yVertices = new double[numVertices];
		memcpy(xVertices, shpObjIn->padfX, sizeof(double)*numVertices);
		memcpy(yVertices, shpObjIn->padfY, sizeof(double)*numVertices);

		bool isSuccessful = transformation->Transform(
			numVertices, xVertices, yVertices);

		if (!isSuccessful) {
			fprintf(stderr, "failed to project shp no. %d\n", i);
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

	fs::path outPrjPath(outSHP);
	outPrjPath.replace_extension(".prj");
	copyFile(inPrjPath, outPrjPath);

	fs::path outDbfPath(outSHP);
	outDbfPath.replace_extension(".dbf");
	fs::path inDbfPath(inSHP);
	inDbfPath.replace_extension(".dbf");
	copyFile(inDbfPath, outDbfPath);

	fs::path outXmlPath(outSHP);
	outXmlPath.replace_extension(".xml");
	fs::path inXmlPath(inSHP);
	inXmlPath.replace_extension(".xml");
	copyFile(inXmlPath, outXmlPath);

	fs::path outTttPath(outSHP);
	outTttPath.replace_extension(".ttt");
	createTTTFile(outTttPath.string().c_str(), inSHP);

	string normalsPathString(outSHP);
	auto loc0 = normalsPathString.find(".");
	normalsPathString.replace(loc0, string::npos, "Normals.bmp");
	
	createNormalTexture(normalsPathString.c_str(), demHeightField, mins[0], maxs[0],
		mins[1], maxs[1], transformation, resolution);
}
