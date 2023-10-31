#pragma once
#include "Buffer.h"
#include <thread>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <queue>

typedef pair<int, int> pi2;

//class BufferPointer {
//public:
//	BufferPointer() :BufferUsingIndex(0) {}
//	int BufferUsingIndex;
//	int operator++() {
//		int NowValue = BufferUsingIndex;
//		if (BufferUsingIndex++ >= )
//		{
//
//		}
//	}
//};

class MergeSort {
	///////////缓存区////////////
	Buffer OutPut0;
	bool OutPut0Ready;
	Buffer OutPut1;
	bool OutPut1Ready;

	bool OutPutUsing; ///<正在使用哪一个输出缓存区

	Buffer* p_Input;
	bool* p_InputReady;
	int* p_InputBufferPos;//-1表示空闲池，0 到 MERGE_WAYS-1 表示在对应的队列中

	bool InputUsing; ///<生成runs的时候正在使用哪一个输入缓存区

	/////////////////缓存操作/////////////////
	int getEmptyBufIndex() {
		for (size_t i = 0; i < 2 * MERGE_WAYS; i++)
			if (p_InputBufferPos[i] == -1)
				return i;
		return -1;
	}

	/// @brief 从文件的startPos位置读入缓存区，尽可能多的读入，文件读完更新InputFinished，缓存区写入完成更新ready
	/// @param p_inputBuf 向那个缓存区读入
	/// @param startPos 开始位置
	/// @param ready 是否完成
	/// @note 该函数为线程函数
	void th_readIn_s(Buffer* p_inputBuf, int startPos, bool* ready) {
		p_inputBuf->readIn(startPos);
		if (!p_inputBuf->inMaxSize())
			this->InputFinished = true;
		if (p_inputBuf->currentSize != 0)
			*ready = true;
		else
			this->InputFinished = true;
	}

	void th_readIn_se(Buffer* p_inputBuf, int startPos, int endPos, bool* ready) {
		p_inputBuf->readIn(startPos, endPos);
		if (!p_inputBuf->inMaxSize())
			this->InputFinished = true;
		if (p_inputBuf->currentSize != 0)
			*ready = true;
		else
			this->InputFinished = true;
		if (startPos == 0 && endPos == 0)
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
		return InputUsing == 0 ? p_Input[0] : p_Input[1];
	}

	/// @brief 未使用的输入缓冲区
	Buffer& AnotherInputBuf() {
		return InputUsing == 0 ? p_Input[1] : p_Input[0];
	}

	/// @brief 正在使用的输入缓冲区的标识符
	bool& ThisInputReady() {
		return InputUsing == 0 ? p_InputReady[0] : p_InputReady[1];
	}

	/// @brief 未使用的输入缓冲区的标识符
	bool& AnotherInputReady() {
		return InputUsing == 0 ? p_InputReady[1] : p_InputReady[0];
	}

	/// @brief 
	/// @param val
	/// @note 需要预先设置 outputFilePos 文件写入起始位置
	void OutputBufAutoPut(int val) {
		if (!OutputBuf().put(val)) {//放入输出缓存区
			// 放入失败，输出缓存区满，写回文件
			ThisOutputReady() = false;//标记缓存区正在写回
			while (!AnotherOutputReady());//等待另一个缓存区写回完成
			thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
			writeBack.detach();
			outputFilePos += OutputBuf().currentSize;//更新文件位置

			OutPutUsing = !OutPutUsing;//切换缓存区

			// 重新放入
			if (!OutputBuf().put(val))
				throw "Unknow";
		}
	}

	//////////////////输入队列及其操作/////////////////////

	queue<int>q_InputQueueIndex[MERGE_WAYS];//输入缓存序号队列

	int inputQueueAutoGet(int index) {
		int temp;
		int curBufIndex = q_InputQueueIndex[index].front();//当前缓冲区的序号
		if (!p_Input[curBufIndex].manualGet(temp)) {
			// 读取失败
			//原因1：该归并段没有内容(虚拟队列或读完)->返回INTMAX
			if (p_Input[curBufIndex].isEndOfFile())
				return INT_MAX;
			//原因2：该归并段还有剩余，但是缓存区读完了->换新缓存区
			p_InputReady[curBufIndex] = false;//标记缓存区不可用
			p_InputBufferPos[curBufIndex] = -1;//放到空闲池
			q_InputQueueIndex[index].pop();//队列中删除该缓存区
			if (q_InputQueueIndex[index].empty()) {//队列为空 -> TODO呼叫新缓存区(理论上到不了这里)、
				throw "Unknow";
				while (q_InputQueueIndex[index].empty());//等待新缓存区添加完成
			}
			curBufIndex = q_InputQueueIndex[index].front();//更新当前缓存区的序号
			while (!p_InputReady[curBufIndex]);//等待新缓存区可用
			if (!p_Input[curBufIndex].manualGet(temp))//重新读取
				throw "Unknow";
		}
		return temp;
	}

	//获得队列已经缓冲的长度
	int getQueueNumInBuff(int index) {
		int QueueFirstLast = p_Input[q_InputQueueIndex[index].front()].getLastNum();//队列第一个缓存区的剩余长度
		return QueueFirstLast + (q_InputQueueIndex[index].size() - 1) * BUFFER_SIZE;//队列已经缓冲的长度
	}


	///////////////交互文件////////////////
	string FileName; ///<文件名
	int fileLength; ///<文件长度

	int inputFilePos;///>已经存入文件位置
	int outputFilePos;///>已经输出文件位置

	bool InputFinished;///>输入是否完成

	////////////有序的runs////////////
	vector<int> v_RunStartPos; ///<每个run的开始位置

	/////////////runs生成///////////// 

	int GenerateRuns() {
		//填充缓存区与初始化败者树
		if (fileLength <= leafNum)
			throw fileLength;//文件长度不足 

		thread initTree(&MergeSort::initLoserTree, this);//初始化败者树
		inputFilePos += leafNum;
		thread initReadIn1(&MergeSort::th_readIn_s, this, &p_Input[0], inputFilePos, &p_InputReady[0]);//读入第一个缓冲区
		initReadIn1.detach();
		inputFilePos += BUFFER_SIZE;

		if (fileLength > inputFilePos) {
			thread initReadIn2(&MergeSort::th_readIn_s, this, &p_Input[1], inputFilePos, &p_InputReady[1]);//读入第二个缓冲区
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
			//testPtintTree();
			int minValIndex = p_LoserTree[0];//最小值的索引，同时也是下一次要放入的位置
			int minVal = p_LoserTree[minValIndex];//最小值

			int nowVal;//本次需要处理的值
			if (!InputBuf().get(nowVal)) {//从输入缓存区取出一个值
				// 取出失败
				ThisInputReady() = false;//标记本缓存区不可用

				if (InputBuf().inMaxSize() && this->InputFinished != true)
					while (!(AnotherInputReady() || this->InputFinished == true));// 等待另一个缓存区读入完成或者文件读完
				if (AnotherInputReady() && AnotherInputBuf().currentSize > 0) {//另一个缓存区有数据

					if (!this->InputFinished) {//如果文件没有读完
						thread readIn(&MergeSort::th_readIn_s, this, &InputBuf(), inputFilePos, &ThisInputReady());//读入文件
						readIn.detach();
						inputFilePos += InputBuf().currentSize;//更新文件位置
					}

					InputUsing = !InputUsing;//切换缓存区

					// 重新取出
					if (!InputBuf().get(nowVal))
						throw "Unknow";
				}
				else
					break;//文件读完，退出循环
			}

			OutputBufAutoPut(minVal);
			//if (!OutputBuf().put(minVal)) {//最小值放入输出缓存区
			//	// 放入失败，输出缓存区满，写回文件
			//	ThisOutputReady() = false;//标记缓存区正在写回
			//	while (!AnotherOutputReady());//等待另一个缓存区写回完成
			//	thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
			//	writeBack.detach();
			//	outputFilePos += OutputBuf().currentSize;//更新文件位置
			//	OutPutUsing = !OutPutUsing;//切换缓存区
			//	// 重新放入
			//	if (!OutputBuf().put(minVal))
			//		throw "Unknow";
			//}
			counter++;//计数器加1
			//检测输出的level是否与上一次相同，如果不相同，增加run的开始位置
			if (p_LeavesLevel[minValIndex - leafStartIndex] > currentLevel) {
				currentLevel = p_LeavesLevel[minValIndex - leafStartIndex];
				v_RunStartPos.push_back(counter);
			}

			if (nowVal < minVal) //如果本次处理的值比上次输出的值小
				p_LeavesLevel[minValIndex - leafStartIndex]++;//归并层次加1
			p_LoserTree[minValIndex] = nowVal;//把本次处理的值放入树中
			Adjust(p_LoserTree[0]);//调整树
		} while (true);
		// 读入完成，向树内填充INTMAX（内部排序处理），输出缓存区写回文件
		while (p_LoserTree[p_LoserTree[0]] != INT_MAX) {
			//testPtintTree();
			OutputBufAutoPut(p_LoserTree[p_LoserTree[0]]);
			//if (!OutputBuf().put(p_LoserTree[p_LoserTree[0]])) {//最小值放入输出缓存区
			//	// 放入失败，输出缓存区满，写回文件
			//	ThisOutputReady() = false;//标记缓存区正在写回
			//	while (!AnotherOutputReady());//等待另一个缓存区写回完成
			//	thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
			//	writeBack.detach();
			//	outputFilePos += OutputBuf().currentSize;//更新文件位置
			//	OutPutUsing = !OutPutUsing;//切换缓存区
			//	// 重新放入
			//	if (!OutputBuf().put(p_LoserTree[p_LoserTree[0]]))
			//		throw "Unknow";
			//}
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
		// 写回文件
		while (!AnotherOutputReady());
		ThisOutputReady() = false;//标记缓存区正在写回
		thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
		writeBack.join();

		// 等待所有缓冲区结束
		while (!OutPut0Ready || !OutPut1Ready || p_InputReady[0] || p_InputReady[1]);
	}

	///////////////败者树///////////////
	int* p_LoserTree; ///<败者树
	int nodeNum; ///<树的节点数
	int* p_LeavesLevel; ///<树叶的归并层次
	int leafNum; ///<树叶数
	int leafStartIndex; ///<树叶开始位置

	/////////////败者树的操作/////////////

	/// @brief 
	/// @param valPos 
	/// @param pos 
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

		if (p_LeavesLevel[valPos - leafStartIndex] < p_LeavesLevel[p_LoserTree[pos] - leafStartIndex])//层次较小，直接存入
			Adjust(valPos, pos / 2);
		else if (p_LeavesLevel[valPos - leafStartIndex] > p_LeavesLevel[p_LoserTree[pos] - leafStartIndex]) {//层次较大，把大的位置存到本地，把自己送上去
			int temp = p_LoserTree[pos];
			p_LoserTree[pos] = valPos;
			Adjust(temp, pos / 2);
		}
		else if (p_LoserTree[p_LoserTree[pos]] >= p_LoserTree[valPos])// 层次相同，送上来的比较小，继续向上送
			Adjust(valPos, pos / 2);
		else {//送上来的比较大，把大的位置存到本地，把自己送上去
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

	/////////////归并树/////////////
	int* p_MergeTree; ///<归并败者树

	/////////////归并树的操作/////////////

	void cleanMergeTree() {
		for (size_t i = 0; i < 2 * MERGE_WAYS; i++)
			p_MergeTree[i] = -1;//初始化归并败者树
	}

	void adjustMergeTree(int valPos, int pos = -1) {
		if (pos == -1) pos = valPos / 2;//如果没有指定位置，则默认为父节点

		if (pos == 0) {//到达根节点
			p_MergeTree[0] = valPos;
			return;
		}

		if (p_MergeTree[pos] == -1) {//本地为空，直接存入
			p_MergeTree[pos] = valPos;
			return;
		}

		if (p_MergeTree[p_MergeTree[pos]] >= p_MergeTree[valPos])// 送上来的比较小，继续向上送
			adjustMergeTree(valPos, pos / 2);
		else {//送上来的比较大，把大的位置存到本地，把自己送上去
			int temp = p_MergeTree[pos];
			p_MergeTree[pos] = valPos;
			adjustMergeTree(temp, pos / 2);
		}
	}

	/////////////测试函数/////////////////

	void testPrint() {
		cout << "BUFFER_SIZE:" << BUFFER_SIZE << endl;
		cout << "TREE_HEIGHT:" << TREE_HEIGHT << endl;
		cout << boolalpha << endl;
		cout << "FileName:" << FileName << endl;
		cout << "fileLength:" << fileLength << endl;
		cout << "inputFilePos:" << inputFilePos << endl;
		cout << "outputFilePos:" << outputFilePos << endl;
		cout << "InputFinished:" << InputFinished << endl;
		cout << "leafNum:" << leafNum << endl;
		cout << "leafStartIndex:" << leafStartIndex << endl;
		cout << "nodeNum:" << nodeNum << endl;
		cout << "OutPut1Ready:" << OutPut0Ready << endl;
		cout << "OutPut2Ready:" << OutPut1Ready << endl;

		cout << "InputReady:" << endl;
		for (size_t i = 0; i < 2 * MERGE_WAYS; i++)
			cout << i << ": " << p_InputReady[i] << "\n";


		cout << endl << "OutPut0:" << endl;
		OutPut0.testPrint();
		cout << endl << "OutPut1:" << endl;
		OutPut1.testPrint();

		cout << endl << "Input:" << endl;
		for (size_t i = 0; i < 2 * MERGE_WAYS; i++) {
			cout << "InputBuffer " << i << ": \n";
			p_Input[i].testPrint();
		}

		testPtintTree();
		testRunStartPos();
	}

	void testRunStartPos() {
		cout << endl << "v_RunStartPos:" << endl;
		for (size_t i = 0; i < v_RunStartPos.size(); i++)
			cout << v_RunStartPos[i] << " ";
		cout << endl;
	}

	void testPtintTree() {
		cout << endl << "p_LoserTree:" << endl;
		for (size_t i = 0; i < nodeNum; i++) {
			cout << p_LoserTree[i] << "  ";
			if (i == 0 || i == 1 || i == 3 || i == 7 || i == 15 || i == 31 || i == 63)
				cout << endl;
		}
		cout << "p_LeavesLevel:" << endl;
		for (size_t i = 0; i < leafNum; i++)
			cout << p_LeavesLevel[i] << "  ";
		cout << endl;
	}

	void testPrintQueue() {
		cout << endl << "q_InputQueueIndex:" << endl;
		for (size_t i = 0; i < MERGE_WAYS; i++) {
			cout << "InputQueue " << i << ": \n";
			queue<int> temp = q_InputQueueIndex[i];
			while (!temp.empty()) {
				cout << temp.front() << " ";
				temp.pop();
			}
			cout << endl;
		}
	}

	void testMergeTree() {
		cout << endl << "p_MergeTree:" << endl;
		for (size_t i = 0; i < 2 * MERGE_WAYS; i++)
			cout << (p_MergeTree[i] == INT_MAX ? -9 : p_MergeTree[i]) << "  ";
		cout << endl;
	}

	void testInputBufferPos() {
		cout << endl << "p_InputBufferPos:" << endl;
		for (size_t i = 0; i < 2 * MERGE_WAYS; i++)
			cout << p_InputBufferPos[i] << "  ";
		cout << endl;
	}

	void testMergeWayBuffer() {
		cout << endl << "MergeWayBuffer:" << endl;
		for (size_t i = 0; i < MERGE_WAYS; i++) {
			cout << endl << "MergeWayBuffer " << i << ": \n";
			p_Input[q_InputQueueIndex[i].front()].testPrint();
		}
	}

	void testBufferReady() {
		cout << endl << "BufferReady:" << endl;
		for (size_t i = 0; i < 2 * MERGE_WAYS; i++)
			cout << boolalpha << p_InputReady[i] << "  ";
		cout << endl;
	}

	void testTempFile() {
		cout << endl << "TempFile:" << endl;
		ifstream file(TEMP_FILE_NAME, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekg(0, ios::beg);
		int temp;
		int counter = 1;
		while (file.read((char*)&temp, sizeof(int))) {
			cout << temp << " ";
			if (counter == TEST_SIZE)
				cout << endl;
			if (counter == 555 || counter == 639 || counter == 728 || counter == 825 || counter == 928)
				cout << endl;
			if (counter % 100 == 0)
				cout << "\n|==  " << counter << "  ==|";
			counter++;
		}
		cout << endl;
		file.close();
	}

public:
	///////////////构造和析构////////////////

	MergeSort(string fileName)
		:FileName(fileName), InputFinished(false), inputFilePos(0), outputFilePos(0),//初始化文件名和文件位置
		OutPut0(TEMP_FILE_NAME), OutPut1(TEMP_FILE_NAME), //初始化缓存区文件对象
		OutPut0Ready(true), OutPut1Ready(true), //初始化缓存区标识符
		OutPutUsing(0), InputUsing(0)//初始化缓存区使用标识符
	{

		//除了前两个准备用来生成runs的缓存区，其他缓存区都用来归并
		p_Input = new Buffer[2 * MERGE_WAYS];//初始化输入缓存区
		p_InputReady = new bool[2 * MERGE_WAYS];//初始化输入缓存区标识符
		p_InputBufferPos = new int[2 * MERGE_WAYS];//初始化输入缓存区标识符
		for (int i = 0; i < 2 * MERGE_WAYS; i++) {
			p_Input[i].setFileName(TEMP_FILE_NAME);//设置文件名
			p_InputReady[i] = false;//初始化输入缓存区标识符
			p_InputBufferPos[i] = -1;//初始化输入缓存区位置符
		}
		p_Input[0].setFileName(FileName);
		p_Input[1].setFileName(FileName);

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
		p_Input[0].setFileLength(fileLength);
		p_Input[1].setFileLength(fileLength);

		ofstream cl(TEMP_FILE_NAME, ios::binary | ios::trunc);//清空文件
		if (!cl.is_open())
			throw "File Open Failed!";
		cl.close();

		p_MergeTree = new int[2 * MERGE_WAYS];//归并败者树
		cleanMergeTree();//清空归并败者树

	}

	~MergeSort() {
		delete[] p_LoserTree;
		delete[] p_LeavesLevel;
		delete[] p_Input;
		delete[] p_InputReady;
		delete[] p_InputBufferPos;
		delete[] p_MergeTree;
	}

	int test = 0;


	void Sort() {
		try
		{
			GenerateRuns();//生成有序的runs

			//testRunStartPos();
			//testTempFile();

			p_Input[0].setFileName(TEMP_FILE_NAME);
			p_Input[1].setFileName(TEMP_FILE_NAME);
			OutPut0.setFileName(TEMP_FILE_NAME);
			OutPut1.setFileName(TEMP_FILE_NAME);
			OutPutUsing = 0;

			int runNum = v_RunStartPos.size();//run的数量

			//初始化run堆
			priority_queue<pi2, vector<pi2>, greater<pi2>> pq;//小顶堆
			int index = 0;
			for (; index < runNum; index++) {
				if (index == runNum - 1)
					pq.push(make_pair(fileLength - v_RunStartPos[index], index));
				else
					pq.push(make_pair(v_RunStartPos[index + 1] - v_RunStartPos[index], index));
			}


			//需要进行runNum-1次归并
			int totalLength = fileLength;//temp文件的总长度，用于计算归并结果的位置
			while (pq.size() != 1) {
				int thisLength = 0;

				// 读取最多MERGE_WAYS个run，归并到一个新的run中

				cleanMergeTree();//清空归并败者树
				// 重置队列
				for (int i = 0; i < MERGE_WAYS; i++)
					while (!q_InputQueueIndex[i].empty())
						q_InputQueueIndex[i].pop();
				// 重置缓存区
				for (int i = 0; i < 2 * MERGE_WAYS; i++) {
					p_InputReady[i] = false;
					p_InputBufferPos[i] = -1;
					p_Input[i].setFileName(TEMP_FILE_NAME);
				}

				if (runNum == 23)
					testTempFile();

				// 创造MERGE_WAYS个输入缓存队列，并各初始化一个输入缓存区
				cout << "用";
				for (size_t i = 0; i < MERGE_WAYS; i++)
				{
					if (pq.empty())
						pq.push(make_pair(0, 0));//虚拟队列,该队列没有内容

					//初始化一个缓存区进队列
					thread Read(&MergeSort::th_readIn_se, this, &p_Input[i], v_RunStartPos[pq.top().second], v_RunStartPos[pq.top().second] + pq.top().first, &p_InputReady[i]);
					Read.detach();

					q_InputQueueIndex[i].push(i);//把缓存区的序号放入队列
					p_InputBufferPos[i] = i;//标记缓存区位置

					thisLength += pq.top().first;//计算归并结果的长度

					cout << pq.top().second << "段（" << v_RunStartPos[pq.top().second] << "开始，长度为" << pq.top().first << "）";

					pq.pop();

				}

				//testInputBufferPos();
				//testMergeWayBuffer();

				v_RunStartPos.push_back(totalLength);//把归并结果的起始位置放入run的起始位置
				pq.push(make_pair(thisLength, runNum++));//把归并结果放入小顶堆

				cout << "生成" << runNum - 1 << "段的归并进行中，结果从" << totalLength << "开始，长度为" << thisLength << endl;
				totalLength += thisLength;


				// 自动从输入队列总取出一个值，期间有缓存区的自动切换（若缓存区读完，则加入缓存区？），若下一个缓存区在读，则等待，若队列读完，则返回INTMAX

				//testBufferReady();

				// 等待前几个缓存完成读入
				bool finishRead = true;
				do {
					finishRead = true;
					for (size_t i = 0; i < MERGE_WAYS; i++)
						finishRead = finishRead && p_InputReady[q_InputQueueIndex[i].front()];
				} while (!finishRead);

				//填充归并败者树
				for (int i = MERGE_WAYS - 1; i >= 0; i--) {
					p_MergeTree[MERGE_WAYS + i] = inputQueueAutoGet(i);
					adjustMergeTree(MERGE_WAYS + i);
				}


				while (p_MergeTree[p_MergeTree[0]] != INT_MAX) {

					//cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>OneMerge<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"<< endl;
					//cout << test++ << "  p_MergeTree[p_MergeTree[0]]: " << p_MergeTree[p_MergeTree[0]] << endl;
					//testMergeTree();
					//testPrintQueue();
					//testMergeWayBuffer();

					//归并败者树的根节点不为INTMAX，说明还有数据
					OutputBufAutoPut(p_MergeTree[p_MergeTree[0]]);
					p_MergeTree[p_MergeTree[0]] = inputQueueAutoGet(p_MergeTree[0] - MERGE_WAYS);//对应的队列再取一个值
					adjustMergeTree(p_MergeTree[0]);//调整树


					int exhaustFirst = -1;//最先耗尽的队列
					int minLast = INT_MAX;//最小的队列的长度
					// 找到最先耗尽的队列
					for (int i = 0; i < MERGE_WAYS; i++) {
						if (p_Input[q_InputQueueIndex[i].back()].isEndOfFile())continue;//如果队列已经耗尽
						if (getQueueNumInBuff(i) < minLast) {
							minLast = getQueueNumInBuff(i);
							exhaustFirst = i;
						}
					}

					if (exhaustFirst != -1) {
						//为输入队列增加缓存区
						int newBufIndex = getEmptyBufIndex();//获得一个空闲的缓存区序号
						if (newBufIndex != -1) {
							while (!p_InputReady[q_InputQueueIndex[exhaustFirst].back()]);//等待最后一个缓存区读入完成-->这个应该提前到找到最先耗尽的队列之前，但是因为后面的if语句会判断是否耗尽，所以这里不会出错
							if (p_Input[q_InputQueueIndex[exhaustFirst].back()].getNextStatPos() == p_Input[q_InputQueueIndex[exhaustFirst].back()].endPos)
								p_Input[q_InputQueueIndex[exhaustFirst].back()].isEnd = true;//标记队列已经耗尽
							else {
								thread Read(&MergeSort::th_readIn_se, this, &p_Input[newBufIndex], p_Input[q_InputQueueIndex[exhaustFirst].back()].getNextStatPos(), p_Input[q_InputQueueIndex[exhaustFirst].back()].endPos, &p_InputReady[newBufIndex]);
								Read.detach();
								q_InputQueueIndex[exhaustFirst].push(newBufIndex);//把缓存区的序号放入队列
								p_InputBufferPos[newBufIndex] = exhaustFirst;//标记缓存区位置
							}
						}
					}

				}

				// 写回
				while (!AnotherOutputReady());
				ThisOutputReady() = false;//标记缓存区正在写回
				thread writeBack(&MergeSort::th_writeBack_s, this, &OutputBuf(), outputFilePos, &ThisOutputReady());//写回文件
				writeBack.join();

			}//归并结束
			//将最后一个run写回文件
			ifstream f(TEMP_FILE_NAME, ios::binary);
			ofstream finalout(FileName, ios::binary | ios::trunc);
			if (!finalout.is_open())
				throw "File Open Failed!";
			f.seekg(v_RunStartPos[pq.top().second] * sizeof(int), ios::beg);
			int temp;
			while (f.read((char*)&temp, sizeof(int)))
				finalout.write((char*)&temp, sizeof(int));
			f.close();
			finalout.close();

			remove(TEMP_FILE_NAME);

		}
		catch (int smallLength)
		{
			//内部排序处理
			int* p = new int[smallLength];
			ifstream file(FileName, ios::binary);
			if (!file.is_open())
				throw "File Open Failed!";
			file.seekg(0, ios::beg);
			for (size_t i = 0; i < smallLength; i++) {
				int temp;
				file.read((char*)&temp, sizeof(int));
				p[i] = temp;
			}
			file.close();
			sort(p, p + smallLength);
			ofstream file2(FileName, ios::binary | ios::trunc);
			if (!file2.is_open())
				throw "File Open Failed!";
			for (size_t i = 0; i < smallLength; i++)
				file2.write((char*)&p[i], sizeof(int));
			file2.close();
			delete[] p;
		}
	}
};