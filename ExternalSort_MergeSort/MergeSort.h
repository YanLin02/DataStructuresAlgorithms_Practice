#pragma once
#include "Buffer.h"
#include <cmath>

class MergeSort
{
public:
	MergeSort(string filename)
		:filename(filename), input1(filename, INPUT_BUFFER_SIZE), input2(filename, INPUT_BUFFER_SIZE), output(2 * INPUT_BUFFER_SIZE), curMergeSize(1), dataInTemp(false) {
		ifstream file(filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekg(0, ios::end);
		fileLength = file.tellg() / sizeof(int);
		file.close();
	}

	void sort() {

		//curMergeSize = 1 不会进行排序 默认输出文件是filename dataInTemp = false
		//进行一次排序 输出文件是temp.bin dataInTemp = true

		while (curMergeSize < fileLength) {
			/// 需要归并 fileLength / ( 2 * curMergeSize) 次,向上取整
			/// 每次归并 2 * curMergeSize 个数 两个缓存各读入 curMergeSize 个数
			/// 每次交换文件后，第1次归并时，第一个缓存读入 0 ~ curMergeSize - 1 个数，第二个缓存读入 curMergeSize ~ 2 * curMergeSize - 1 个数
			/// 第2次归并时，第一个缓存读入 2 * curMergeSize ~ 3 * curMergeSize - 1 个数，第二个缓存读入 3 * curMergeSize ~ 4 * curMergeSize - 1 个数
			/// 第3次归并时，第一个缓存读入 4 * curMergeSize ~ 5 * curMergeSize - 1 个数，第二个缓存读入 5 * curMergeSize ~ 6 * curMergeSize - 1 个数
			/// 第n次归并时，第一个缓存读入 (2 * n - 2) * curMergeSize ~ (2 * n - 1) * curMergeSize - 1 个数，第二个缓存读入 (2 * n - 1) * curMergeSize ~ 2 * n * curMergeSize - 1 个数
			/// 如果文件不够长，则第二个缓存读入的数目可能不足 curMergeSize 个数，此时出现单支现象，
			/// 单支现象出现在第二个缓存读入的数目不足 curMergeSize 个数时，此时第一个缓存读入的数目一定是 curMergeSize 个数

			//进行一层排序


			int MergeTimes = ceil((double)fileLength / (2 * curMergeSize));//一层需要归并的次数

			//cout << "\nMergeTimes: " << MergeTimes << endl;//TODO:debug
			//cout << "MergeSize: " << curMergeSize << endl;//TODO:debug
			//cout << "GroupSize: " << 2 * curMergeSize << endl;//TODO:debug
			//testPrint();//TODO:debug
			//cout << endl;//TODO:debug

			dataInTemp = !dataInTemp;//输出反向

			output.startPos = 0;//重置输出缓存起始位置 

			for (size_t n = 0; n < MergeTimes; n++) {
				//初始化缓存读入位置
				input1.startPos = min((int)(2 * n) * curMergeSize, fileLength);
				input2.startPos = min((int)(2 * n + 1) * curMergeSize, fileLength);

				//检测是否出现单支现象
				if (input2.startPos >= fileLength) {//出现单支现象
					input1.readIn(input1.startPos, min(input1.startPos + curMergeSize, fileLength));//读入第一个缓存
					int num;
					while (input1.getNum(num)) {
						output.putIn(num);
					}
				}
				else {//没有出现单支现象
					//设置读入范围
					input1.readIn(input1.startPos, min(input1.startPos + curMergeSize, fileLength));//读入第一个缓存
					input2.readIn(input2.startPos, min(input2.startPos + curMergeSize, fileLength));//读入第二个缓存

					//单次归并
					int num1, num2;
					input1.getNum(num1);
					input2.getNum(num2);

					while (true) {
						if (num1 >= num2) {
							output.putIn(num2);
							if (!input2.getNum(num2)) {//第二个缓存读完
								do {//第一个缓存剩余数据读入
									output.putIn(num1);
								} while (input1.getNum(num1));
								break;//归并结束
							}
						}
						else {
							output.putIn(num1);
							if (!input1.getNum(num1)) {//第一个缓存读完
								do {//第二个缓存剩余数据读入
									output.putIn(num2);
								} while (input2.getNum(num2));
								break;//归并结束
							}
						}
					}//双支归并循环结束
				}//没有出现单支现象
				//output.writeBack();//输出缓存写回 TODO:debug
				//output.testPrint();//TODO:debug
			}//一层归并结束

			//后续工作
			changeFile();//交换文件
			curMergeSize *= 2;//归并大小翻倍
		}
		cleanfile();//清理文件
	}

	void print() {
		ifstream file(filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.seekg(0, ios::beg);
		int temp;
		while (file.read((char*)&temp, sizeof(int)))
			cout << temp << " ";
		cout << endl;
		file.close();
	}

private:
	string filename; ///<文件名

	Buffer input1; ///<输入缓存1
	Buffer input2; ///<输入缓存2
	Buffer output; ///<输出缓存

	int curMergeSize; ///<当前归并大小

	int fileLength; ///<文件长度

	bool dataInTemp; ///<数据是否在临时文件中

	void changeFile() {
		output.writeBack();
		string temp = input1.filename;
		input1.filename = input2.filename = output.filename;
		output.filename = temp;
		output.clean();
	}

	void cleanfile() {
		if (dataInTemp) {
			remove(filename.c_str());
			rename("temp.bin", filename.c_str());
		}
		else
			remove("temp.bin");
	}

	void testPrint() {
		ifstream file;
		if (dataInTemp)
			file.open("temp.bin", ios::binary);
		else
			file.open(filename, ios::binary);

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

