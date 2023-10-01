#pragma once
#include "Definition.h"
#include "Buffer.h"

class Matrix
{
public:
	string filename; //交互文件名
	int rowSize; //行数
	int columnSize; //列数
	Buffer buf; //缓存


};

