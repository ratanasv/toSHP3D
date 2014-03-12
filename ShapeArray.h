#pragma once
#include <vector>
#include <memory>


//header for shape files
struct shpmainheader
{
	int filecode;
	int unused0;
	int unused1;
	int unused2;
	int unused3;
	int unused4;
	int filelength;
	int version;
	int shapetype;
	int pad;
	double xmin, ymin;
	double xmax, ymax;
	double zmin, zmax;
	double mmin, mmax;
};

//pack xyz components into a point
struct point3d
{
	double x, y, z;
};

//header for shapes
struct shpheader
{
	double xmin, ymin, xmax, ymax, zmin, zmax;
	int numparts;
	int numpoints;
};



//a shape contains the following info
class VI_Shape {
public:
	int ShapeType;
	struct shpheader ShapeHeader;
	std::vector<int> Parts;
	std::shared_ptr<std::vector<struct point3d>> Vertices;

	VI_Shape() {
		Vertices.reset(new std::vector<struct point3d>());
	}
};

/**Shape array reference class.*/
class VI_ShapeArray {
private:
	std::shared_ptr<std::vector<VI_Shape>> i_array;
	struct shpmainheader i_shpHeader;
	std::size_t i_hashCode;
	unsigned i_numVertices;
public:
	/** Constructor. 
	* @param array The array of data to reference.
	* @numValues The number of values in this array. */
	VI_ShapeArray(std::shared_ptr<std::vector<VI_Shape>> array = 
		std::shared_ptr<std::vector<VI_Shape>>(), struct shpmainheader header = 
		struct shpmainheader());

	/** Returns true if an array is set. 
	* @return true if this reference contains an array, false otherwise.
	*/
	bool HasData() const;

	/** Get array size. 
	* @return The number of values in this array.
	*/
	unsigned	GetNumShapes() const;

	unsigned GetNumVertices() const;

	/** Get value at index. 
	* @param index An index of the array.
	* @return The value at index.
	*/
	const VI_Shape&	GetValue(unsigned index) const;
	VI_Shape&	GetValue(unsigned index);

	/**Get mainheader of the shapefile
	* @return the mainheader of the shapefile
	*/
	struct shpmainheader GetShpMainHeader() const;

	std::size_t GetHashCode() const;

private:
	std::size_t ComputeHashCode();
	unsigned ComputeNumVertices();
};