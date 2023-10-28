#pragma once
#include "Def.h"
#include <string>
#include <fstream>

using namespace std;

class MergeSort;

class Buffer
{
	friend class MergeSort;

	string fileName;///> 文件名》需要初始化
	int fileLength;///> 文件长度》需要初始化

	int* p_buffer;///> 缓存》需要初始化

	int currentSize;///> 当前缓存存储了多少数
	int currentPos;///> 当前缓存读取到的位置
	int startPos;///> 当前缓存起始对应文件位置

	Buffer(string fileName)
		:fileName(fileName), currentSize(0), currentPos(0), startPos(0) {
		// 初始化缓存
		this->p_buffer = new int[BUFFER_SIZE];
		// 获取文件长度
		updateFileLength();
	}

	~Buffer() {
		delete[] p_buffer;
	}

	// 更新文件长度
	void updateFileLength() {
		ifstream in(fileName, ios::binary);
		in.seekg(0, ios::end);
		this->fileLength = in.tellg() / sizeof(int);
		in.close();
	}

	bool get(int& value) {
		if (currentPos < currentSize) {
			value = p_buffer[currentPos++];
			return true;
		}
		return false;
	}

	bool put(int value) {
		if (currentSize < BUFFER_SIZE) {
			p_buffer[currentSize++] = value;
			return true;
		}
		return false;
	}

	/// @brief 从文件中读取数据到缓存中
	/// @param startPos 从startPos处开始读取，0开始（跳过前startPos个）
	/// @return 返回读取的长度
	int readIn(int startPos) {
		// 计算获取文件长度
		int length = BUFFER_SIZE > fileLength - startPos ? fileLength - startPos : BUFFER_SIZE;
		// 读取文件
		ifstream in(fileName, ios::binary);
		in.seekg(startPos * sizeof(int), ios::beg);
		in.read((char*)p_buffer, length * sizeof(int));
		in.close();
		// 更新缓存信息
		this->currentSize = length;
		this->currentPos = 0;
		this->startPos = startPos;
		// 返回读取长度
		return length;
	}

	void writeBack() {
		// 更新文件长度
		updateFileLength();
		// 写入文件
		ofstream out(fileName, ios::binary | ios::app);
		out.write((char*)p_buffer, currentSize * sizeof(int));
		out.close();
		// 更新缓存信息
		this->currentSize = 0;
		this->currentPos = 0;
		this->startPos = fileLength;
	}

	int getStartPos() {
		return startPos;
	}



};

