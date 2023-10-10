#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "MergeSort.h"
using namespace std;

void getTestFile() {
	srand(time(NULL));

	//随机生成测试文件
	ofstream file(TEST_FILE_NAME, ios::binary);
	if (!file.is_open())
		throw "File Open Failed!";
	int random;
	for (size_t i = 0; i < TEST_FILE_SIZE; i++) {
		random = rand() % TEST_NUM_MAX;
		file.write((char*)&random, sizeof(int));
	}
	file.close();
}

int main()
{
	getTestFile();

	MergeSort mergeSort(TEST_FILE_NAME);
	mergeSort.print();
	mergeSort.sort();
	mergeSort.print();

	return 0;
}