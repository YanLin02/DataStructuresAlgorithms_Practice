#pragma once
#include "Buffer.h"
#include <thread>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

class MergeSort {
public:
	///////////缓存区////////////
	Buffer OutPut0;
	bool OutPut0Ready;
	Buffer OutPut1;
	bool OutPut1Ready;

	bool OutPutUsing; ///<正在使用哪一个输出缓存区

	Buffer Input0;
	bool Input0Ready;
	Buffer Input1;
	bool Input1Ready;

	bool InputUsing; ///<正在使用哪一个输入缓存区

	/////////////////缓存操作/////////////////

	/// @brief 从文件的startPos位置读入缓存区，尽可能多的读入，文件读完更新InputFinished，缓存区写入完成更新ready
	/// @param p_inputBuf 向那个缓存区读入
	/// @param startPos 开始位置
	/// @param ready 是否完成
	/// @note 该函数为线程函数
	void th_readIn_s(Buffer* p_inputBuf, int startPos, bool* ready) {
		p_inputBuf->readIn(startPos);
		if (!p_inputBuf->inMaxSize())
			this->InputFinished = true;
		*ready = true;
	}

	void th_readIn_se(Buffer* p_inputBuf, int startPos, int endPos, bool* ready) {
		p_inputBuf->readIn(startPos, endPos);
		if (!p_inputBuf->inMaxSize())
			this->InputFinished = true;
		*ready = true;
	}

	void th_writeBack(Buffer* p_outputBuf, bool* ready) {
		p_outputBuf->writeBack();
		*ready = true;
	}

	void th_writeBack_s(Buffer* p_outputBuf, int startPos, bool* ready) {
		p_outputBuf->writeBack(startPos);
		*ready = true;
	}

	/// @brief 正在使用的写回缓冲区
	Buffer& OutputBuf() {
		return OutPutUsing == 0 ? OutPut0 : OutPut1;
	}

	/// @brief 正在使用的写回缓冲区的标识符
	bool& ThisOutputReady() {
		return OutPutUsing == 0 ? OutPut0Ready : OutPut1Ready;
	}

	/// @brief 未使用的写回缓冲区的标识符
	bool& AnotherOutputReady() {
		return OutPutUsing == 0 ? OutPut1Ready : OutPut0Ready;
	}

	/// @brief 正在使用的输入缓冲区
	Buffer& InputBuf() {
		return InputUsing == 0 ? Input0 : Input1;
	}

	/// @brief 未使用的输入缓冲区
	Buffer& AnotherInputBuf() {
		return InputUsing == 0 ? Input1 : Input0;
	}

	/// @brief 正在使用的输入缓冲区的标识符
	bool& ThisInputReady() {
		return InputUsing == 0 ? Input0Ready : Input1Ready;
	}

	/// @brief 未使用的输入缓冲区的标识符
	bool& AnotherInputReady() {
		return InputUsing == 0 ? Input1Ready : Input0Ready;
	}

	///////////////交互文件////////////////
	string FileName; ///<文件名
	int fileLength; ///<文件长度
	bool dataInTemp; ///<数据是否在临时文件中

	int inputFilePos;///>已经存入文件位置
	int outputFilePos;///>已经输出文件位置

	bool InputFinished;///>输入是否完成

	////////////有序的runs////////////
	//int runNum; ///<有序的runs数
	vector<int> v_RunStartPos; ///<每个run的开始位置
	//vector<int> v_RunLength; ///<每个run的长度

	/////////////runs生成///////////// 

	int GenerateRuns() {
		//填充缓存区与初始化败者树

		if (fileLength <= leafNum)
			throw fileLength;//文件长度不足 TODO 内部排序处理

		thread initTree(&MergeSort::initLoserTree, this);//初始化败者树
		inputFilePos += leafNum;
		thread initReadIn1(&MergeSort::th_readIn_s, this, &Input0, inputFilePos, &Input0Ready);//读入第一个缓冲区
		initReadIn1.detach();
		inputFilePos += BUFFER_SIZE;

		if (fileLength > inputFilePos) {
			thread initReadIn2(&MergeSort::th_readIn_s, this, &Input1, inputFilePos, &Input1Ready);//读入第二个缓冲区
			initReadIn2.detach();
			inputFilePos += BUFFER_SIZE;
		}
		//至少一个缓存区读入完成并且树初始化完成
		initTree.join();
		while (!ThisInputReady());

		//向树中插入数据，先对比归并层次，进行低层次处理。如果输入的数据比上一个输出的小，则将其归并层次在上一个输出的基础上加1。

		int currentLevel = -1;//当前归并层次
		int counter = -1;//计数器，用于计算run的开始位置(由于首个为0，所以初始化为-1)

		do {
			int minValIndex = p_LoserTree[0];//最小值的索引，同时也是下一次要放入的位置
			int minVal = p_LoserTree[minValIndex];//最小值
			if (!OutputBuf().put(minVal)) {//最小值放入输出缓存区
				// 放入失败，输出缓存区满，写回文件
				ThisOutputReady() = false;//标记缓存区正在写回
				thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
				writeBack.detach();
				outputFilePos += OutputBuf().currentSize;//更新文件位置
				while (!AnotherOutputReady());//等待另一个缓存区写回完成

				OutPutUsing = !OutPutUsing;//切换缓存区

				// 重新放入
				if (!OutputBuf().put(minVal))
					throw "Unknow";
			}
			counter++;//计数器加1
			//检测输出的level是否与上一次相同，如果不相同，增加run的开始位置
			if (p_LeavesLevel[minValIndex - leafStartIndex] > currentLevel) {
				currentLevel = p_LeavesLevel[minValIndex - leafStartIndex];
				v_RunStartPos.push_back(counter);
			}

			int nowVal;//本次需要处理的值
			if (!InputBuf().get(nowVal)) {//从输入缓存区取出一个值
				// 取出失败
				ThisInputReady() = false;//标记本缓存区不可用

				if (InputBuf().inMaxSize() && this->InputFinished != true)
					while (!(AnotherInputReady()));// 等待另一个缓存区读入完成或者文件读完
				if (AnotherInputReady() && AnotherInputBuf().currentSize > 0) {//另一个缓存区有数据

					if (!this->InputFinished) {//如果文件没有读完
						thread readIn(&MergeSort::th_readIn_s, this, &InputBuf(), inputFilePos, &ThisInputReady());//读入文件
						readIn.detach();
						inputFilePos += BUFFER_SIZE;//更新文件位置
					}

					InputUsing = !InputUsing;//切换缓存区

					// 重新取出
					if (!InputBuf().get(nowVal))
						throw "Unknow";
				}
				else
				{
					// 如果文件读完，输入完成break
					break;
				}
			}
			if (nowVal < minVal) //如果本次处理的值比上次输出的值小
				p_LeavesLevel[minValIndex - leafStartIndex]++;//归并层次加1
			p_LoserTree[minValIndex] = nowVal;//把本次处理的值放入树中
			Adjust(p_LoserTree[0]);//调整树
		} while (true);

		// 读入完成，向树内填充INTMAX（内部排序处理），输出缓存区写回文件
		while (p_LoserTree[p_LoserTree[0]] != INT_MAX) {
			if (!OutputBuf().put(p_LoserTree[p_LoserTree[0]])) {//最小值放入输出缓存区
				// 放入失败，输出缓存区满，写回文件
				ThisOutputReady() = false;//标记缓存区正在写回
				thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
				writeBack.detach();
				outputFilePos += BUFFER_SIZE;//更新文件位置
				while (!AnotherOutputReady());//等待另一个缓存区写回完成

				OutPutUsing = !OutPutUsing;//切换缓存区

				// 重新放入
				if (!OutputBuf().put(p_LoserTree[p_LoserTree[0]]))
					throw "Unknow";
			}
			counter++;//计数器加1
			//检测输出的level是否与上一次相同，如果不相同，增加run的开始位置
			if (p_LeavesLevel[p_LoserTree[0] - leafStartIndex] > currentLevel) {
				currentLevel = p_LeavesLevel[p_LoserTree[0] - leafStartIndex];
				v_RunStartPos.push_back(counter);
			}
			p_LoserTree[p_LoserTree[0]] = INT_MAX;
			p_LeavesLevel[p_LoserTree[0] - leafStartIndex] = INT_MAX;
			Adjust(p_LoserTree[0]);
		}

		//// 写回文件
		ThisOutputReady() = false;//标记缓存区正在写回
		thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
		writeBack.detach();

		// 等待所有缓冲区结束
		while (!OutPut0Ready || !OutPut1Ready || Input0Ready || Input1Ready);

		dataInTemp = true;//数据在临时文件中

	}
	///////////////败者树///////////////
	int* p_LoserTree; ///<败者树
	int nodeNum; ///<树的节点数
	int* p_LeavesLevel; ///<树叶的归并层次
	int leafNum; ///<树叶数
	int leafStartIndex; ///<树叶开始位置

	/////////////败者树的操作/////////////

	void Adjust(int valPos, int pos = -1)
	{
		if (pos == -1) pos = valPos / 2;//如果没有指定位置，则默认为父节点

		if (pos == 0) {//到达根节点
			p_LoserTree[0] = valPos;
			return;
		}

		if (p_LoserTree[pos] == -1) {//本地为空，直接存入
			p_LoserTree[pos] = valPos;
			return;
		}

		if (p_LoserTree[p_LoserTree[pos]] >= p_LoserTree[valPos] && p_LeavesLevel[valPos - leafStartIndex] <= p_LeavesLevel[p_LoserTree[pos] - leafStartIndex])//送上来的比较小，并且层次相同，继续向上送
			Adjust(valPos, pos / 2);
		else {//送上来的比较大或层次较大，把大的位置存到本地，把自己送上去
			int temp = p_LoserTree[pos];
			p_LoserTree[pos] = valPos;
			Adjust(temp, pos / 2);
		}
	}


	// 初始化败者树
	void initLoserTree() {
		ifstream file(FileName, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekg(0, ios::beg);
		for (size_t i = 0; i < leafNum; i++) {
			int temp;
			file.read((char*)&temp, sizeof(int));
			p_LoserTree[leafStartIndex + i] = temp;//初始化树叶
			Adjust(leafStartIndex + i);//调整树叶
			//testPtintTree();
			p_LeavesLevel[i] = 0;//初始化树叶的归并层次
		}
		file.close();
	}
	///////////////构造和析构////////////////
public:
	MergeSort(string fileName)
		:FileName(fileName), dataInTemp(false), InputFinished(false), inputFilePos(0), outputFilePos(0),//初始化文件名和文件位置
		OutPut0(TEMP_FILE_NAME), OutPut1(TEMP_FILE_NAME), Input0(fileName), Input1(fileName),//初始化缓存区文件对象
		OutPut0Ready(true), OutPut1Ready(true), Input0Ready(false), Input1Ready(false), //初始化缓存区标识符
		OutPutUsing(0), InputUsing(0)//初始化缓存区使用标识符
	{
		nodeNum = pow(2, TREE_HEIGHT);//树的节点数
		p_LoserTree = new int[nodeNum];//败者树
		for (size_t i = 0; i < nodeNum; i++)
			p_LoserTree[i] = -1;//初始化败者树

		leafStartIndex = leafNum = pow(2, TREE_HEIGHT - 1);//树叶数
		p_LeavesLevel = new int[leafNum];//树叶的归并层次

		for (int i = 0; i < leafNum; i++)
			p_LeavesLevel[i] = 0;//初始化树叶的归并层次

		ifstream file(FileName, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";

		file.seekg(0, ios::end);
		fileLength = file.tellg() / sizeof(int);
		file.close();

		OutPut0.setFileLength(fileLength);
		OutPut1.setFileLength(fileLength);
		Input0.setFileLength(fileLength);
		Input1.setFileLength(fileLength);

	}

	~MergeSort() {
		delete[] p_LoserTree;
		delete[] p_LeavesLevel;
	}

	/////////////测试函数/////////////////
	void testPrint() {
		cout << "BUFFER_SIZE:" << BUFFER_SIZE << endl;
		cout << "TREE_HEIGHT:" << TREE_HEIGHT << endl;
		cout << boolalpha << endl;
		cout << "FileName:" << FileName << endl;
		cout << "fileLength:" << fileLength << endl;
		cout << "dataInTemp:" << dataInTemp << endl;
		cout << "inputFilePos:" << inputFilePos << endl;
		cout << "outputFilePos:" << outputFilePos << endl;
		cout << "InputFinished:" << InputFinished << endl;
		cout << "leafNum:" << leafNum << endl;
		cout << "leafStartIndex:" << leafStartIndex << endl;
		cout << "nodeNum:" << nodeNum << endl;
		cout << "OutPut1Ready:" << OutPut0Ready << endl;
		cout << "OutPut2Ready:" << OutPut1Ready << endl;
		cout << "Input1Ready:" << Input0Ready << endl;
		cout << "Input1Ready:" << Input1Ready << endl;
		cout << endl << "OutPut0:" << endl;
		OutPut0.testPrint();
		cout << endl << "OutPut1:" << endl;
		OutPut1.testPrint();
		cout << endl << "Input0:" << endl;
		Input0.testPrint();
		cout << endl << "Input1:" << endl;
		Input1.testPrint();
		testPtintTree();
		cout << endl << "p_LeavesLevel:" << endl;
		for (size_t i = 0; i < leafNum; i++)
			cout << p_LeavesLevel[i] << " ";
		cout << endl;

		cout << endl << "v_RunStartPos:" << endl;
		for (size_t i = 0; i < v_RunStartPos.size(); i++)
			cout << v_RunStartPos[i] << " ";
		cout << endl;
	}

	void testPtintTree() {
		cout << endl << "p_LoserTree:" << endl;
		for (size_t i = 0; i < nodeNum; i++) {
			cout << p_LoserTree[i] << " ";
			if (i == 0 || i == 1 || i == 3 || i == 7 || i == 15 || i == 31 || i == 63)
				cout << endl;
		}
		cout << endl;
	}
};