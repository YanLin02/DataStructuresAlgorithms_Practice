#include "Buffer.h"

/// @brief 获取数据
/// @param row 行号
/// @param column 列号
/// @return 对应的双精度浮点数
double Buffer::getNum(int row, int column) {
	this->visit++;// 访问次数+1
	if (!this->Buf.count(Locate(row, column))) {
		this->miss++;// 缓存未命中次数+1
		readIn(Locate(row, column));// 读入
	}

	return this->Buf[Locate(row, column)];
}

/// @brief 写入数据
/// @param row 行号
/// @param column 列号
/// @param num 需要写入的双精度浮点数
void Buffer::setNum(int row, int column, double num) {
	this->visit++;// 访问次数+1
	if (this->Buf.count(Locate(row, column)))// 如果存在
		this->Buf[Locate(row, column)] = num;// 直接写入
	else {// 如果不存在
		this->miss++;// 缓存未命中次数+1
		this->readIn(Locate(row, column));// 读入
		this->Buf[Locate(row, column)] = num;// 写入
	}
	this->dirty = true;// 脏位置
}

/// @brief 写回
void Buffer::writeBack() {
	ofstream file(this->filename, ios::binary|ios::in);
	if (!file.is_open())
		throw "File Open Failed!";

	int filepos = this->columnSize * this->startLocate.first + this->startLocate.second;//计算文件位置

	file.seekp(2 * sizeof(int) + filepos * sizeof(double), ios::beg);//定位到文件位置
	int max_size = this->rowSize * this->columnSize;
	for (size_t i = filepos; i < (filepos + BUFFER_SIZE > max_size ? max_size : filepos + BUFFER_SIZE); i++)
		file.write((char*)&this->Buf[Locate(i / this->columnSize, i % this->columnSize)], sizeof(double));//写入数据

	file.close();
	this->dirty = false;//写回后脏位清零
}

/// @brief 从指定位置读入
/// @param startLocate 开始位置
void Buffer::readIn(const Locate& startLocate) {
	if (this->dirty)
		writeBack();//写回

	this->startLocate = startLocate;//设置起始位置
	this->clear();//清空缓存

	ifstream file(this->filename, ios::binary);
	if (!file.is_open())
		throw "File Open Failed!";

	int filepos = this->columnSize * this->startLocate.first + this->startLocate.second;//计算文件位置

	file.seekg(2 * sizeof(int) + filepos * sizeof(double), ios::beg);//定位到文件位置

	int max_size = this->rowSize * this->columnSize;
	for (size_t i = filepos; i < (filepos + BUFFER_SIZE > max_size ? max_size : filepos + BUFFER_SIZE); i++) {
		double num = -1.0;
		file.read((char*)&num, sizeof(double));//读取数据
		this->Buf.insert(make_pair(Locate(i / this->columnSize, i % this->columnSize), num));//写入缓存
	}
	file.close();
	this->dirty = false;//读入后脏位清零
}