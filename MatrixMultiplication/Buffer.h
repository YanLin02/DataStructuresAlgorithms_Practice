
#pragma once
#include "Definition.h"

#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <string>

using namespace std;

typedef pair<unsigned int, unsigned int> Locate;

/// @brief Locate's hash, used for hashmap
struct Locate_hash {
	template <class T, class U> int operator()(const pair<T, U>& a) const {
		return hash<T>()(a.first) * 13 ^ hash<U>()(a.second) * 31;
	}
};

/// @brief ���黺��
class Buffer {
public:
	unordered_map<Locate, double, Locate_hash> Buf; //����
	bool dirty; //�Ƿ���
	Locate startLocate; //��ʼλ��

	string filename; //�����ļ���
	int rowSize; //����
	int columnSize; //����

	/// @brief ���캯��
	/// @param filename �����ļ���
	Buffer(string filename)
		: dirty(false), filename(filename), visit(0), miss(0) {
		ifstream file(this->filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.read((char*)&this->rowSize, sizeof(int));
		file.read((char*)&this->columnSize, sizeof(int));
		file.close();
	}

	/// @brief ��������
	~Buffer() {
		if (this->dirty)
			writeBack();
	}

	/// @brief ��ȡ����
	double getNum(int row, int column);

	/// @brief д������
	void setNum(int row, int column, double num);

	/// @brief ��ջ���
	void clear() {
		this->Buf.clear();
		this->dirty = false;
	}

	/// @brief д��
	void writeBack();

	/// @brief ��ָ��λ�ö���
	void readIn(const Locate& startLocate);

	//ͳ�ƻ���������
	int visit;
	int miss;
	double hitRate() {
		return 1.0 - (double)this->miss / this->visit;
	}
};
