#include "stdafx.h"
#include "Nearest.h"


Nearest::Nearest(std::shared_ptr<float>& data, const int resolution) : 
	_data(data), _resolution(resolution)
{

}


float Nearest::valueAt(float di, float dj) {
	int ii = (int)di;
	int jj = (int)dj;
	if (ii > _resolution || jj > _resolution || ii < 0 || jj < 0) {
		throw out_of_range("invalid lat/long");
	}
	return (_data.get())[_resolution*ii + jj];
}
