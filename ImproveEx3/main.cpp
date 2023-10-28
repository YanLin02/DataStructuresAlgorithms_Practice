#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include<algorithm>
#include "MergeSort.h"
using namespace std;

int main()
{
	//srand(time(NULL));

	int* arr = new int[TEST_SIZE];
	for (int i = 0; i < TEST_SIZE; i++)
	{
		arr[i] = rand() % 100;
		cout << arr[i] << " ";
	}
	cout << endl;
	ofstream t("test.bin", ios::binary);
	t.write((char*)arr, sizeof(int) * TEST_SIZE);
	t.close();


	sort(arr, arr + TEST_SIZE);
	for (int i = 0; i < TEST_SIZE; i++)
		cout << arr[i] << " ";
	cout << endl;

	MergeSort test("test.bin");
	test.GenerateRuns();
	test.testPrint();

	int temp;
	ifstream f(TEMP_FILE_NAME, ios::binary);
	for (int i = 0; i < TEST_SIZE; i++)
	{
		f.read((char*)&temp, sizeof(int));
		cout << temp << " ";
	}

	//system("pause");
	return 0;
}