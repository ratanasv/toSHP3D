#include "stdafx.h"
#include "Linear.h"

using namespace std;

Linear::Linear(shared_ptr<float>& data, const int resolution) : 
	Nearest(data, resolution)
{

}

float Linear::valueAt(float di, float dj) {
	return 0.0;
}
