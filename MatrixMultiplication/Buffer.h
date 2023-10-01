
#pragma once
#include "Definition.h"

#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <string>

using namespace std;

typedef pair<unsigned int, unsigned int> Locate;

/// @brief Locate's hash, used for hashmap
struct Locate_hash {
	template <class T, class U> int operator()(const pair<T, U>& a) const {
		return hash<T>()(a.first) * 13 ^ hash<U>()(a.second) * 31;
	}
};

/// @brief 数组缓存
class Buffer {
public:
	unordered_map<Locate, double, Locate_hash> Buf; //缓存
	bool dirty; //是否脏
	Locate startLocate; //起始位置

	string filename; //交互文件名
	int rowSize; //行数
	int columnSize; //列数

	/// @brief 构造函数
	/// @param filename 交互文件名
	Buffer(string filename)
		: dirty(false), filename(filename), visit(0), miss(0) {
		ifstream file(this->filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.read((char*)&this->rowSize, sizeof(int));
		file.read((char*)&this->columnSize, sizeof(int));
		file.close();
	}

	/// @brief 析构函数
	~Buffer() {
		if (this->dirty)
			writeBack();
	}

	/// @brief 获取数据
	double getNum(int row, int column);

	/// @brief 写入数据
	void setNum(int row, int column, double num);

	/// @brief 清空缓存
	void clear() {
		this->Buf.clear();
		this->dirty = false;
	}

	/// @brief 写回
	void writeBack();

	/// @brief 从指定位置读入
	void readIn(const Locate& startLocate);

	//统计缓存命中率
	int visit;
	int miss;
	double hitRate() {
		return 1.0 - (double)this->miss / this->visit;
	}
};
