#pragma once
#include "Defination.h"
#include <string>
#include <fstream>

using namespace std;

class Buffer
{
public:
	string filename; ///<文件名

	int* buf; ///<缓存

	int bufSize; ///<缓存大小
	int curSize; ///<当前大小
	int curPos; ///<当前位置

	int startPos; ///<起始位置
	int endPos; ///<结束位置
	bool isEnd; ///<是否结束

	Buffer(string filename, int bufSize)
		: filename(filename), bufSize(bufSize), curSize(0), startPos(0), endPos(0), curPos(0), isEnd(false) {
		buf = new int[bufSize];
	}

	Buffer(int bufSize)
		: filename("temp.bin"), bufSize(bufSize), curSize(0), startPos(0), endPos(0), curPos(0), isEnd(false) {
		ofstream file("temp.bin", ios::binary);
		file.close();
		buf = new int[bufSize];
	}

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

	void writeBack() {
		ofstream file(filename, ios::binary | ios::in);//防止覆盖
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekp(startPos * sizeof(int), ios::beg);
		file.write((char*)buf, curSize * sizeof(int));
		file.close();

		curSize = 0;//重置大小
	}

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

	void putIn(int num) {
		buf[curSize++] = num;
		if (curSize == bufSize) {
			writeBack();
			startPos += bufSize;
			curSize = 0;
		}
	}

	void setFilename(string filename) {
		this->filename = filename;
	}

	void setBufSize(unsigned int bufSize) {
		this->bufSize = bufSize;
	}

	void clean() {
		ofstream file(filename, ios::binary);
		file.close();
	}

	void testPrint() {
		for (size_t i = 0; i < curSize; i++)
			cout << buf[i] << " ";
		cout << endl;
	}
};

