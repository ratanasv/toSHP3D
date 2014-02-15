#pragma once
#include <memory>

class Bicubic {
private:
	const int _resolution;
	//_data is of dimension NxN
	std::shared_ptr<float> _data;
	//_alpha is of dimension (N-1)x(N-1)x4x4
	std::shared_ptr<float> _alpha;
public:
	Bicubic(std::shared_ptr<float>& data, const int resolution);
	float valueAt(float di, float dj);
public:
	void computeAlphaAt(int i, int j);
	std::shared_ptr<float> initFX();
	std::shared_ptr<float> initFY();
	std::shared_ptr<float> initFXY(std::shared_ptr<float>& fx, 
		std::shared_ptr<float>& fy);
	void initAlpha(std::shared_ptr<float>& fx, 
		std::shared_ptr<float>& fy, std::shared_ptr<float>& fxy);
};