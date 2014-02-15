#include "stdafx.h"
#include "Bicubic.h"
#include <stdexcept>

using namespace std;

// see http://en.wikipedia.org/wiki/Bicubic_interpolation#Bicubic_interpolation.
const Eigen::MatrixXf& MatrixA() {
	static bool hasBeenInit = false;
	static Eigen::MatrixXf AInverse(16, 16);
	if (!hasBeenInit) {
		AInverse << 1 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
			0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
			-3 , 3 , 0 , 0 , -2 , -1 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
			2 , -2 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
			0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
			0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 , 0 ,
			0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , -3 , 3 , 0 , 0 , -2 , -1 , 0 , 0 ,
			0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 2 , -2 , 0 , 0 , 1 , 1 , 0 , 0 ,
			-3 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , -2 , 0 , -1 , 0 , 0 , 0 , 0 , 0 ,
			0 , 0 , 0 , 0 , -3 , 0 , 3 , 0 , 0 , 0 , 0 , 0 , -2 , 0 , -1 , 0 ,
			9 , -9 , -9 , 9 , 6 , 3 , -6 , -3 , 6 , -6 , 3 , -3 , 4 , 2 , 2 , 1 ,
			-6 , 6 , 6 , -6 , -3 , -3 , 3 , 3 , -4 , 4 , -2 , 2 , -2 , -2 , -1 , -1 ,
			2 , 0 , -2 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 , 0 , 0 , 0 , 0 , 0 ,
			0 , 0 , 0 , 0 , 2 , 0 , -2 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 1 , 0 ,
			-6 , 6 , 6 , -6 , -4 , -2 , 4 , 2 , -3 , 3 , -3 , 3 , -2 , -1 , -2 , -1 ,
			4 , -4 , -4 , 4 , 2 , 2 , -2 , -2 , 2 , -2 , 2 , -2 , 1 , 1 , 1 , 1;
		hasBeenInit = true;
	}
	static Eigen::MatrixXf A = AInverse.inverse();
	return A;
}

Bicubic::Bicubic(shared_ptr<float>& data, const int resolution) : 
	_data(data), _resolution(resolution)
{
	auto fx = initFX();
	auto fy = initFY();
	auto fxy = initFXY(fx, fy);
	initAlpha(fx, fy, fxy);
}

float Bicubic::valueAt(float di, float dj) {
	if (di > _resolution || di < 0 || dj > _resolution || dj < 0) {
		throw runtime_error("di dj out of range");
	}

	const int ii = (int)di;
	const int jj = (int)dj;

	const float* alpha = _alpha.get() + (ii*_resolution*16 + jj*16);

	float x = di - (float)ii;
	float y = dj - (float)jj;

	float sum = 0.0;
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			sum += alpha[4*i+j]*pow(x, i)*pow(y, j);
		}
	}

	return sum;
}

void Bicubic::computeAlphaAt(int i, int j) {
	
}

shared_ptr<float> Bicubic::initFX() {
	shared_ptr<float> managedFx(new float[_resolution*_resolution], [](float* what) {
		delete[] what;
	});
	auto fx = managedFx.get();
	const float* f = _data.get();
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for (int i=0; i<_resolution; i++) {
		for (int j=0; j<_resolution; j++) {
			const int row = i*_resolution;
			if (j == 0) {// left edge
				fx[row + j] = (f[row + j + 1] - f[row + j]);
			} else if (j == _resolution-1) { //right edge
				fx[row + j] = (f[row + j] - f[row + j - 1]);
			} else {
				fx[row + j] = (f[row + j + 1] - f[row + j - 1])/2.0;
			}
		}
	}
#pragma omp barrier
	return managedFx;
}

shared_ptr<float> Bicubic::initFY() {
	shared_ptr<float> managedFy(new float[_resolution*_resolution], [](float* what) {
		delete[] what;
	});
	auto fy = managedFy.get();
	const float* f = _data.get();
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for (int i=0; i<_resolution; i++) {
		for (int j=0; j<_resolution; j++) {
			const int row = i*_resolution;
			if (i == 0) {
				fy[row+j] = (f[row+_resolution+j]-f[row+j]);
			} else if (i == _resolution-1) {
				fy[row+j] = (f[row+j]-f[row-_resolution+j]);
			} else {
				fy[row + j] = (f[row + _resolution + j] - f[row - _resolution + j])/2.0;
			}
		}
	}
#pragma omp barrier
	return managedFy;
}

shared_ptr<float> Bicubic::initFXY(shared_ptr<float>& mfx, shared_ptr<float>& mfy) {
	shared_ptr<float> managedFxy(new float[_resolution*_resolution], [](float* what) {
		delete[] what;
	});
	auto fxy = managedFxy.get();
	const float* fx = mfx.get();
	const float* fy = mfy.get();
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for (int i=0; i<_resolution; i++) {
		for (int j=0; j<_resolution; j++) {
			const int row = i*_resolution;
			if (j == 0) {// left edge
				fxy[row+j] = (fy[row+j+1] - fy[row+j]);
			} else if (j == _resolution-1) { //right edge
				fxy[row+j] = (fy[row+j] - fy[row+j-1]);
			} else {
				fxy[row+j] = (fy[row+j+1] - fy[row+j-1])/2.0;
			}
		}
	} 
#pragma omp barrier
	return managedFxy;
}

void Bicubic::initAlpha(shared_ptr<float>& mfx, shared_ptr<float>& mfy, 
	shared_ptr<float>& mfxy) 
{
	_alpha.reset(new float[(_resolution-1)*(_resolution-1)*16], [](float *what) {
		delete[] what;	
	});

	assert(mfx);
	assert(mfy);
	assert(mfxy);
	assert(_alpha);

	for (int i=0; i<_resolution-1; i++) {
		for (int j=0; j<_resolution-1; j++) {
			Eigen::VectorXf alphas(16);
			Eigen::VectorXf xs(16);

			const float* fx = mfx.get();
			const float* fy = mfy.get();
			const float* fxy = mfxy.get();
			const float* f = _data.get();
			float* a = _alpha.get() + (i*(_resolution-1)*16+j*16);

			const int zerozero = i*_resolution + j;
			const int zeroone = i*_resolution + j + 1;
			const int onezero = i*(_resolution+1) + j;
			const int oneone = i*(_resolution+1) + j + 1;

			xs << f[zerozero], f[onezero], f[zeroone], f[oneone], 
				fx[zerozero], fx[onezero], fx[zeroone], fx[oneone], 
				fy[zerozero], fy[onezero], fy[zeroone], fy[oneone], 
				fxy[zerozero], fxy[onezero], fxy[zeroone], fxy[oneone];
			alphas = MatrixA()*xs;

			for (int k=0; k<16; k++) {
				a[k] = alphas[k];
			}
		}
	}
}
