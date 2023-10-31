#define _CRT_SECURE_NO_WARNINGS
#include <iostream>

#include "MergeSort.h"
using namespace std;

int main()
{
	//srand(time(NULL));
	//srand(202310353);

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
	test.Sort();


	cout << "\n\n结果：\n";
	int temp;
	ifstream f("test.bin", ios::binary);
	while (f.read((char*)&temp, sizeof(int)))
		cout << temp << " ";


	//system("pause");
	return 0;
}