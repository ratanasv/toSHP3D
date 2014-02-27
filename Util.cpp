#include "stdafx.h"
#include "Util.h"


shared_ptr<SHPObject> initShape(SHPObject* shp) {
	return shared_ptr<SHPObject>(shp, [](SHPObject* shpData) {
		SHPDestroyObject(shpData);
	});
}

shared_ptr<SHPInfo> initShapeHandle(SHPInfo* shpInfo) {
	return shared_ptr<SHPInfo>(shpInfo, [](SHPInfo* shpInfoHandle) {
		SHPClose(shpInfoHandle);
	});
}

FILE* FileGuard::get() {
	return _fp;
}

FileGuard::~FileGuard() {
	if (_fp != NULL) fclose(_fp);
}

FileGuard::FileGuard(const std::string& where, const char* mode) {
	_fp = fopen(where.c_str(), mode);
}