#pragma once

#include <memory>
#include <cassert>

template<class T> shared_ptr<T> initCStyleArray(T* data) {
	return shared_ptr<T>(data, [](T* tData) {
		delete[] tData;
	});
}

template<class T> shared_ptr<vector<T>> initVectorArray() {
	return shared_ptr<vector<T>>(new vector<T>());
}

shared_ptr<SHPObject> initShape(SHPObject* shp);

shared_ptr<SHPInfo> initShapeHandle(SHPInfo* shpInfo);

class FileGuard {
private:
	FILE* _fp;
public:
	FileGuard(const std::string& where, const char* mode);
	~FileGuard();
	FILE* get();
private:
	FileGuard(const FileGuard& other);
	FileGuard& operator=(const FileGuard& other);
};

template <class T> std::shared_ptr<T> InitCStyleArray(T* data) {
	return std::shared_ptr<T>(data, [](T* ptr) {
		delete[] ptr;
	});
}

template<class T> class TwoDArray {
public:
	TwoDArray(const int width, const int height) : _width(width), _height(height), 
		_data(initCStyleArray(new T[width*height])) {};
	TwoDArray(std::shared_ptr<T> data, const int width, const int height) : 
		_width(width), _height(height), _data(data) {};
	T* operator[] (int i) {
		assert(i<_height);
		return _data.get() + i*_width;
	}

	const T* operator[] (int i) const {
		assert(i<_height);
		return _data.get() + i*_width;
	}

	operator std::shared_ptr<T>() {
		return _data;
	}

	std::shared_ptr<T> get() const {
		return _data;
	}

private:
	const int _width;
	const int _height;
	std::shared_ptr<T> _data;
};