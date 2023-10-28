#pragma once

#include <string>
#include <fstream>
#include "Def.h"

using namespace std;

class MergeSort;

class Buffer {
	friend class MergeSort;

	int* p_buffer;///> 缓冲区

	string fileName;///> 文件名
	int fileLength;///> 文件长度

	int currentSize;///> 当前缓存存储了多少数
	int currentPos;///> 当前缓存读取到的位置

	int startPos;///> 当前缓存起始对应文件位置
	int endPos;///> 当前缓存结束对应文件位置
	bool isEnd;///> 是否读取结束

public:

	Buffer(string fileName)
		:fileName(fileName), fileLength(0),
		currentSize(0), currentPos(0),
		startPos(0), endPos(0), isEnd(true) {
		p_buffer = new int[BUFFER_SIZE];
	}

	~Buffer() {
		delete[] p_buffer;
	}

	/// @brief 根据起始位置和结束位置读入缓存，默认start和end不会越界
	/// @param start 起始位置
	/// @param end 结束位置
	void readIn(int start, int end) {
		ifstream file(fileName, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";

		currentPos = 0;//重置位置
		startPos = start;
		endPos = end;

		//计算缓存大小并判断是否读取结束
		if (BUFFER_SIZE >= endPos - startPos) {//如果缓存足够大
			currentSize = endPos - startPos;
			isEnd = true;
		}
		else {//如果缓存不够大
			currentSize = BUFFER_SIZE;
			isEnd = false;
		}

		file.seekg(startPos * sizeof(int), ios::beg);
		file.read((char*)p_buffer, currentSize * sizeof(int));//读入缓存
		file.close();
	}

	/// @brief 从起始位置读入BUFFER_SIZE个，如果不足则读入剩余的
	/// @param start 起始位置
	void readIn(int start) {
		ifstream file(fileName, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";

		currentPos = 0;//重置位置
		startPos = start;
		isEnd = true;

		//判断是否能读取BUFFER_SIZE个
		if (startPos + BUFFER_SIZE >= fileLength) {//如果不能读取BUFFER_SIZE个
			currentSize = fileLength - startPos;
		}
		else {//如果能读取BUFFER_SIZE个
			currentSize = BUFFER_SIZE;
		}

		endPos = startPos + currentSize;

		file.seekg(startPos * sizeof(int), ios::beg);
		file.read((char*)p_buffer, currentSize * sizeof(int));//读入缓存
		file.close();
	}

	/// @brief 根据是否读完继续读取文件
	/// @return 是否读完
	bool continueRead() {
		if (isEnd)//如果已经读完
			return false;
		else {//如果还没读完

			startPos += BUFFER_SIZE;
			if (startPos + BUFFER_SIZE >= endPos) {//如果缓存足够大
				currentSize = endPos - startPos;
				isEnd = true;
			}
			else//如果缓存不够大
				currentSize = BUFFER_SIZE;

			ifstream file(fileName, ios::binary);
			if (!file.is_open())
				throw "File Open Failed!";

			file.seekg(startPos * sizeof(int), ios::beg);
			file.read((char*)p_buffer, currentSize * sizeof(int));
			file.close();
			currentPos = 0;//重置位置
			return true;
		}
	}

	/// @brief 将缓存从内置的startPos开始写回文件
	void writeBack() {
		ofstream file(fileName, ios::binary | ios::app);//防止覆盖
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekp(startPos * sizeof(int), ios::beg);
		file.write((char*)p_buffer, currentSize * sizeof(int));
		file.close();

		currentSize = 0;//重置大小
		currentPos = 0;//重置位置
	}

	/// @brief 将缓存从给定位置开始写回文件
	/// @param start 起始位置
	void writeBack(int start) {
		startPos = start;
		writeBack();
	}

	bool get(int& num) {
		if (currentPos < currentSize)//如果还没读完,直接读取
			num = p_buffer[currentPos++];
		else//如果已经读完,检测
			if (!continueRead())//如果文件也读完了,返回false
				return false;
			else//如果文件还没读完,继续读取
				num = p_buffer[currentPos++];
		return true;
	}

	bool put(int num) {
		if (currentSize < BUFFER_SIZE) {
			p_buffer[currentSize++] = num;
			return true;
		}
		return false;
	}

	void setFileName(string fileName) {
		this->fileName = fileName;
		this->currentPos = 0;
		this->currentSize = 0;
		this->startPos = 0;
		this->endPos = 0;
		this->isEnd = true;


	}

	void setFileLength(int fileLength) {
		this->fileLength = fileLength;
	}

	int getStatPos() {
		return startPos;
	}

	/// @brief 返回缓存是否达到最大容量
	bool inMaxSize() {
		return currentSize == BUFFER_SIZE;
	}

	int getTail() {
		return p_buffer[currentSize - 1];
	}

	////////测试函数////////
	void testPrint() {
		cout << "fileName:" << fileName << endl;
		cout << "fileLength:" << fileLength << endl;
		cout << "currentSize:" << currentSize << endl;
		cout << "currentPos:" << currentPos << endl;
		cout << "startPos:" << startPos << endl;
		cout << "endPos:" << endPos << endl;
		cout << "isEnd:" << isEnd << endl;
		cout << "Buffer:" << endl;
		for (int i = 0; i < currentSize; i++)
			cout << p_buffer[i] << " ";
		cout << endl;
	}
};