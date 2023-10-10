#pragma once
#include "Defination.h"
#include <string>
#include <fstream>

/*  BUFFER 改进方案
* 建立映射关系，通过角标（目的读取位置）进行数据的访问
* 当读写访问的角标超出缓存时，将缓存中的数据写入文件（暂定，是否需要脏位），然后读取新的数据
* 
* 本次BUFFER中，需要手动设置缓存的起始位置和结束位置
* 读写不是通过角标，而是通过get和put函数顺序读写
*/

using namespace std;

class MergeSort;

class Buffer
{
	friend class MergeSort;//允许MergeSort访问私有成员

	string filename; ///<文件名

	int* buf; ///<缓存

	int bufSize; ///<缓存大小
	int curSize; ///<当前大小
	int curPos; ///<当前位置

	int startPos; ///<起始位置
	int endPos; ///<结束位置
	bool isEnd; ///<是否结束

	/// @brief 通过文件名和缓存大小构造缓存
	/// @param filename 文件名
	/// @param bufSize 缓存大小
	Buffer(string filename, int bufSize)
		: filename(filename), bufSize(bufSize), curSize(0), startPos(0), endPos(0), curPos(0), isEnd(false) {
		buf = new int[bufSize];
	}

	/// @brief 通过缓存大小构造临时缓存
	/// @param bufSize 缓存大小
	Buffer(int bufSize)
		: filename("temp.bin"), bufSize(bufSize), curSize(0), startPos(0), endPos(0), curPos(0), isEnd(false) {
		ofstream file("temp.bin", ios::binary);
		file.close();
		buf = new int[bufSize];
	}

	/// @brief 根据起始位置和结束位置读入缓存
	/// @param start 起始位置
	/// @param end 结束位置
	void readIn(int start, int end) {
		ifstream file(filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";

		curPos = 0;//重置位置

		startPos = start;
		endPos = end;
		if (bufSize >= endPos - startPos) {
			curSize = endPos - startPos;
			isEnd = true;
		}
		else {
			curSize = bufSize;
			isEnd = false;
		}

		file.seekg(startPos * sizeof(int), ios::beg);
		file.read((char*)buf, curSize * sizeof(int));
		file.close();
	}

	/// @brief 根据是否读完继续读取文件
	/// @return 是否读完
	bool continueRead() {
		if (isEnd)
			return false;
		else {
			startPos += bufSize;
			if (startPos + bufSize >= endPos) {
				curSize = endPos - startPos;
				isEnd = true;
			}
			else
				curSize = bufSize;
			ifstream file(filename, ios::binary);
			if (!file.is_open())
				throw "File Open Failed!";
			file.seekg(startPos * sizeof(int), ios::beg);
			file.read((char*)buf, curSize * sizeof(int));
			file.close();
			curPos = 0;//重置位置
			return true;
		}
	}

	/// @brief 将缓存写回文件
	void writeBack() {
		ofstream file(filename, ios::binary | ios::in);//防止覆盖
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekp(startPos * sizeof(int), ios::beg);
		file.write((char*)buf, curSize * sizeof(int));
		file.close();

		curSize = 0;//重置大小
	}

	/// @brief 获取一个数
	/// @param num 引用方式返回的数
	/// @return 是否成功
	bool getNum(int& num) {
		if (curPos < curSize)
			num = buf[curPos++];
		else
			if (!continueRead())
				return false;
			else
				num = buf[curPos++];
		return true;
	}

	/// @brief 将一个数放入缓存
	/// @param num 要放入的数
	void putIn(int num) {
		buf[curSize++] = num;
		if (curSize == bufSize) {
			writeBack();
			startPos += bufSize;
			curSize = 0;
		}
	}

	/// @brief 设置文件名
	/// @param filename 文件名
	void setFilename(string filename) {
		this->filename = filename;
	}

	/// @brief 清空文件
	void clean() {
		ofstream file(filename, ios::binary);
		file.close();
	}

	/// @brief 测试打印缓存
	/// @note 仅用于调试
	void testPrint() {
		for (size_t i = 0; i < curSize; i++)
			cout << buf[i] << " ";
		cout << endl;
	}
};

