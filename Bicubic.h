#pragma once
#include <memory>

class Bicubic {
private:
	const int _resolution;
	//_data is of dimension NxN
	std::shared_ptr<float> _data;
	//_alpha is of dimension (N-1)x(N-1)x4x4
	std::shared_ptr<float> _alpha;
	std::shared_ptr<float> _fx;
	std::shared_ptr<float> _fy;
	std::shared_ptr<float> _fxy;
public:
	Bicubic(std::shared_ptr<float>& data, const int resolution);
	float valueAt(float di, float dj);
private:
	void computeAlphaAt(int i, int j);
	void initFX();
	void initFY();
	void initFXY();
};