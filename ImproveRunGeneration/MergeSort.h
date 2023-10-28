#pragma once
#include "Buffer.h"
#include <thread>
#include <iostream>
#include <cmath>

class MergeSort
{
public:
	MergeSort(string fileName)
		:FileName(fileName), dataInTemp(false), InputFinished(false), inputFilePos(0),
		OutPut1(TEMP_FILE_NAME), OutPut2(TEMP_FILE_NAME), Input1(fileName), Input2(fileName),
		OutPut1Ready(false), OutPut2Ready(false), Input1Ready(false), Input2Ready(false) {

		nodeNum = pow(2, TREE_HEIGHT);//树的节点数
		p_LoserTree = new int[nodeNum];//败者树

		leafStartPos = leafNum = pow(2, TREE_HEIGHT - 1);//树叶数
		p_LeavesLevel = new int[leafNum];//树叶的归并层次

	}

	~MergeSort() {
		delete[] p_LoserTree;
		delete[] p_LeavesLevel;
	}

	/// @brief 打印文件
	void print() {
		ifstream file(FileName, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekg(0, ios::beg);
		int temp;
		while (file.read((char*)&temp, sizeof(int)))
			cout << temp << " ";
		cout << endl;
		file.close();
	}

	/// @brief 排序
	void sort() {
		ifstream file(FileName, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";

		file.seekg(0, ios::end);
		int fileLength = file.tellg() / sizeof(int);

		if (fileLength < leafNum)
			throw fileLength;//文件长度不足 TODO 内部排序处理

		thread initTree(&MergeSort::thread_initTree, this);//初始化败者树
		inputFilePos += leafNum;
		thread initReadIn1(&MergeSort::thread_readIn, this, &Input1, inputFilePos, &Input1Ready);//读入第一个缓冲区
		inputFilePos += BUFFER_SIZE;

		if (fileLength > inputFilePos) {
			thread initReadIn2(&MergeSort::thread_readIn, this, &Input2, inputFilePos, &Input2Ready);//读入第二个缓冲区
			inputFilePos += BUFFER_SIZE;
		}

		initTree.join();

		file.close();
	}

private:
	Buffer OutPut1;
	bool OutPut1Ready;
	Buffer OutPut2;
	bool OutPut2Ready;

	Buffer Input1;
	bool Input1Ready;
	Buffer Input2;
	bool Input2Ready;

	int inputFilePos;///>已经存入文件位置

	bool InputFinished;///>输入是否完成

	string FileName;
	bool dataInTemp; ///<数据是否在临时文件中

	int* p_LoserTree; ///<败者树
	int nodeNum; ///<树的节点数
	int* p_LeavesLevel; ///<树叶的归并层次
	int leafNum; ///<树叶数
	int leafStartPos; ///<树叶开始位置

	void thread_initTree() {
		ifstream file(FileName, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekg(0, ios::beg);
		for (size_t i = 0; i < leafNum; i++) {
			int temp;
			file.read((char*)&temp, sizeof(int));
			p_LoserTree[leafStartPos + i] = temp;
		}
		file.close();
	}




	void thread_readIn(Buffer* inputBuf, int startPos, bool* ready) {
		int length = inputBuf->readIn(startPos);
		if (length != BUFFER_SIZE)
			this->InputFinished = true;
		*ready = true;
	}

	void thread_writeBack(Buffer* outputBuf, bool* ready) {
		outputBuf->writeBack();
		*ready = true;
	}

	/// @brief 清理临时文件
	void cleanfile() {
		if (dataInTemp) {
			remove(FileName.c_str());
			rename(TEMP_FILE_NAME, FileName.c_str());
		}
		else
			remove(TEMP_FILE_NAME);
	}

	/// @brief 根据数据位置打印文件
	/// @details 用于调试
	void testPrint() {
		ifstream file;
		if (dataInTemp)
			file.open(TEMP_FILE_NAME, ios::binary);
		else
			file.open(FileName, ios::binary);

		if (!file.is_open())
			throw "File Open Failed!";
		file.seekg(0, ios::beg);
		int temp;
		while (file.read((char*)&temp, sizeof(int)))
			cout << temp << " ";
		cout << endl;
		file.close();
	}

};

