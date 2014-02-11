#include "stdafx.h"
#include "createSHP3D.h"

using namespace vir;
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

std::shared_ptr<std::vector<double>> giveMeHeights(SHPObject* shpObjIn, struct ElevationData ed)
{
	double latRange = ed.MaxLat-ed.MinLat;
	double lngRange = ed.MaxLng-ed.MinLng;
	std::shared_ptr<std::vector<double>> zA(new std::vector<double>);
	for(int i= 0; i<shpObjIn->nVertices; i++){
		double x = shpObjIn->padfX[i];
		double y = shpObjIn->padfY[i];
		
		int latI = (int)( ((y-ed.MinLat)/latRange)*((float)ed.NumLats-1.0)  );
		int lngI = (int)( ((x-ed.MinLng)/lngRange)*((float)ed.NumLngs-1.0)  );
		assert(latI < ed.NumLats);
		assert(lngI < ed.NumLngs);
		assert(lngI >= 0);
		assert(latI >= 0);

		zA->push_back( ed.Grid->at(ed.NumLngs*latI+lngI) );
	}
	return zA;
}

DECLDIR void createSHP3D(const char* latlong, const char* shp3d, const char* xtrN)
{
	SHPHandle shpIn = SHPOpen(latlong,"rb");
	double mins[4], maxs[4];
	int numShapes;
	SHPGetInfo(shpIn,&numShapes,NULL,mins,maxs);
	for(int i= 0 ;i<4; i++)
		fprintf(stderr, "min,max %f %f \n", mins[i],maxs[i]);

	struct ElevationData ed = readXTR(xtrN);
	SHPHandle shpOut = SHPCreate(shp3d,SHPT_POLYGONZ);


	for(int i = 0 ; i<numShapes ;i++){//for each shape
		SHPObject* shpObjIn = SHPReadObject(shpIn, i);
		std::shared_ptr<std::vector<double>> zA = giveMeHeights(shpObjIn, ed);
		SHPObject* shpObjOut = SHPCreateObject(
			SHPT_POLYGONZ,
			shpObjIn->nShapeId,
			shpObjIn->nParts,
			shpObjIn->panPartStart,
			shpObjIn->panPartType,
			shpObjIn->nVertices,
			shpObjIn->padfX,
			shpObjIn->padfY,
			zA->data(),
			NULL
			);
		int entityNumber = SHPWriteObject(shpOut,-1, shpObjOut);
		SHPDestroyObject(shpObjOut);
		SHPDestroyObject(shpObjIn);
	}


	SHPClose(shpIn);
	SHPClose(shpOut);
}

DECLDIR void createBMP(const char* name, const char* xtrN)
{
	struct ElevationData ed = readXTR(xtrN);
	double hMin = DBL_MAX;
	double hMax = DBL_MIN;
	for(int i= 0; i<ed.NumLngs*ed.NumLats; i++)
	{
		double Gi = ed.Grid->at(i);
		if(Gi>hMax)
			hMax = Gi;
		if(Gi<hMin)
			hMin = Gi;
	}

	std::shared_ptr<std::vector<unsigned char>> pic(
		new std::vector<unsigned char>(ed.NumLngs*ed.NumLats, (unsigned char)128));
	
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for(int i = 0 ; i<ed.NumLats ;i++)
		for(int j = 0; j<ed.NumLngs; j++)
			pic->at(i*ed.NumLngs+j) = (ed.Grid->at(i*ed.NumLngs+j)-hMin)/(hMax-hMin)*255.0;
#pragma omp barrier

	pic = flipVertically(pic,ed.NumLngs,1);

	int result = SOIL_save_image
		(
		name,
		SOIL_SAVE_TYPE_BMP,
		ed.NumLats, ed.NumLngs, 1,
		pic->data()
		);
	if(result == 0)
		fprintf(stderr, "failed to create a BMP file\n");
	else if(result ==1)
		fprintf(stderr, "successfully create a BMP file\n");
	else
		fprintf(stderr, "SOIL_save_image returned an unknown value\n");

}

DECLDIR void createNormalTexture(const char* image, const char* xtr)
{
	struct ElevationData ed = readXTR(xtr);

	std::shared_ptr<std::vector<unsigned char>> normals(
		new std::vector<unsigned char>(ed.NumLngs*ed.NumLats*3, (unsigned char)0));

	std::shared_ptr<std::vector<float>> Grid = ed.Grid;

	int w = ed.NumLngs; int h = ed.NumLats;

	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for(int i =1; i<h-1; i++){
		for(int j =1; j<w-1; j++){
			Angel::vec3 d1(2,0,Grid->at(w*i+j+1)-Grid->at(w*i+j-1));
			Angel::vec3 d2(0,2,Grid->at(w*(i+1)+j)-Grid->at(w*(i-1)+j));
			Angel::vec3 c = normalize(cross(d1,d2));
			normals->at(w*3*i+3*j+0) = 255.0*(0.5*c.x+0.5);
			normals->at(w*3*i+3*j+1) = 255.0*(0.5*c.y+0.5);
			normals->at(w*3*i+3*j+2) = 255.0*(0.5*c.z+0.5);
		}
	}
#pragma omp barrier
	normals = flipVertically(normals,w,3);

	int result = SOIL_save_image
		(
		image,
		SOIL_SAVE_TYPE_BMP,
		w, h, 3,
		normals->data()
		);
	assert(result == 1);
}

DECLDIR void createEvenlySpacedSHP(const char* shpName, const char* xtrName)
{

	SHPHandle shpOut = SHPCreate(shpName,SHPT_POLYGON);
	FILE* FpIn = fopen(xtrName,"rb");
	assert(FpIn!=NULL);
	double minX,minY,maxX,maxY;
	int w,h;

	fscanf( FpIn, "%f %f %d", &minX, &maxX, &w );
	fscanf( FpIn, "%f %f %d", &minY, &maxY, &h);

	fclose(FpIn);


	
	double dx = (maxX-minX)/w;
	double dy = (maxY-minY)/h;

	for(int i =0; i<h-1; i++){
		for(int j =0; j<w-1; j++){
			dA x = dA(new dV(5, 0.0));
			dA y = dA(new dV(5, 0.0));
			x->at(0) = minX+j*dx;
			y->at(0) = minY+i*dy;
			x->at(1) = x->at(0)+dx;
			y->at(1) = y->at(0);
			x->at(2) = x->at(0)+dx;
			y->at(2) = y->at(0)+dy;
			x->at(3) = x->at(0);
			y->at(3) = y->at(0)+dy;
			x->at(4) = x->at(0);
			y->at(4) = y->at(0);
			SHPObject* shpObjOut = SHPCreateSimpleObject(
				SHPT_POLYGON,5,x->data(),y->data(),NULL);
			int entityNumber = SHPWriteObject(shpOut,-1, shpObjOut);
			SHPDestroyObject(shpObjOut);
		}
	}
	
	SHPClose(shpOut);
}


DECLDIR void createBMPwithSHP(const char* shpFile, const char* bmpFile, const char* xtrN)
{
	SHPHandle shpIn = SHPOpen(shpFile,"rb");
	double mins[4], maxs[4];
	int numShapes;
	SHPGetInfo(shpIn,&numShapes,NULL,mins,maxs);
	for(int i= 0 ;i<4; i++)
		fprintf(stderr, "%s min,max %f %f \n", shpFile, mins[i],maxs[i]);

	struct ElevationData ed = readXTR(xtrN);
	double hMin = DBL_MAX;
	double hMax = DBL_MIN;
	for(int i= 0; i<ed.NumLngs*ed.NumLats; i++)
	{
		double Gi = ed.Grid->at(i);
		if(Gi>hMax)
			hMax = Gi;
		if(Gi<hMin)
			hMin = Gi;
	}

	std::shared_ptr<std::vector<unsigned char>> pic(
		new std::vector<unsigned char>(ed.NumLngs*ed.NumLats, (unsigned char)128));

	SHPObject* shape = SHPReadObject(shpIn,0);
	assert(shape != NULL);
	dA heights = giveMeHeights(shape,ed);
	for(int i= 0 ;i<ed.NumLngs*ed.NumLats;i++)
		pic->at(i) = (heights->at(i)-hMin)/hMax*255.0;

	SHPDestroyObject(shape);	
	

	pic = flipVertically(pic,ed.NumLngs,1);

	int result = SOIL_save_image
		(
		bmpFile,
		SOIL_SAVE_TYPE_BMP,
		ed.NumLats, ed.NumLngs, 1,
		pic->data()
		);
	assert(result == 1);
	SHPClose(shpIn);
	
}

DECLDIR void createBMPwithSHP3D(const char* shpFile, const char* bmpFile, int dim)
{
	SHPHandle shpIn = SHPOpen(shpFile,"rb");
	double mins[4], maxs[4];
	int numShapes, shpType;
	SHPGetInfo(shpIn,&numShapes,&shpType,mins,maxs);
	for(int i= 0 ;i<4; i++)
		fprintf(stderr, "%s min,max %f %f \n", shpFile, mins[i],maxs[i]);
	assert(shpType == SHPT_POLYGONZ);

	std::shared_ptr<std::vector<unsigned char>> pic(
		new std::vector<unsigned char>(dim*dim*3, (unsigned char)65));

	for(int i= 0; i<numShapes;i++){
		SHPObject* shape = SHPReadObject(shpIn,i);
		assert(shape != NULL);
		for(int j= 0; j<shape->nVertices; j++){
			double x = shape->padfX[j];
			double y= shape->padfY[j];
			int xi = (x-mins[0])/(maxs[0]-mins[0])*((float)dim - 1.0);
			int yi = (y-mins[1])/(maxs[1]-mins[1])*((float)dim - 1.0);


			double z= shape->padfZ[j];//the height value
			z = 220.0 - (z-mins[2])/(maxs[2]-mins[2])*220.0;//scale it
			float rgb[3];
			float hsv[3] = {z,1.0,1.0};
			HSVtoRGB(hsv,rgb);
			assert(xi <dim);
			assert(yi<dim);
			pic->at(yi*dim*3+xi*3+0) = rgb[0]*255.0;
			pic->at(yi*dim*3+xi*3+1) = rgb[1]*255.0;
			pic->at(yi*dim*3+xi*3+2) = rgb[2]*255.0;
		}
		SHPDestroyObject(shape);
	}

	pic = flipVertically(pic,dim,3);

	int result = SOIL_save_image
		(
		bmpFile,
		SOIL_SAVE_TYPE_BMP,
		dim, dim, 3,
		pic->data()
		);
	assert(result == 1);

	SHPClose(shpIn);
	
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

void createSHP3D(const char* inSHP, const char* outSHP) {
	fs::path inPRJPath(inSHP);
	if (!exists(inPRJPath)) {
		fprintf(stderr, "%s doesn't exist\n", inPRJPath.string().c_str());
		exit(EXIT_FAILURE);
	}
	inPRJPath.replace_extension(".prj");

	fprintf(stderr, "determining transformation....\n");
	OGRSpatialReference sourceSRS;
	char* prjWKT = getContent(inPRJPath);
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
	maxLat = FLT_MIN; maxLng = FLT_MIN;

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

	fprintf(stderr, "connecting to DEM server with query %f %f %f %f %d %d \n", 
		minLng, maxLng, minLat, maxLat, 2048, 2048);
	MikeDEM demHeightField(minLng, maxLng, minLat, maxLat, 2048, 2048);


	fprintf(stderr, "creating 3D shapefile.... \n");
	SHPHandle shpOut = SHPCreate(outSHP,SHPT_POLYGONZ);


	for (int i=0; i<numShapes; i++) {
		SHPObject* shpObjIn = SHPReadObject(shpIn, i);


		bool isSuccessful = transformation->Transform(
			shpObjIn->nVertices, shpObjIn->padfX, shpObjIn->padfY);

		if (!isSuccessful) {
			fprintf(stderr, "failed to project shp no. %d\n", i);
			exit(EXIT_FAILURE);
		}

		double* heights = new double[shpObjIn->nVertices];

		for (int j=0; j<shpObjIn->nVertices; j++) {
			heights[j] = demHeightField.elevAt(
				shpObjIn->padfX[j], shpObjIn->padfY[j]);
		}

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
}
