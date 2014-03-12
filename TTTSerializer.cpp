#include "stdafx.h"
#include "TTTSerializer.h"


using namespace std;


TTTSerializer::TTTSerializer(size_t hashCode,
	shared_ptr<vector<unsigned>> buffer) 
{
	_hashCode = hashCode;
	_buffer = buffer;
}

