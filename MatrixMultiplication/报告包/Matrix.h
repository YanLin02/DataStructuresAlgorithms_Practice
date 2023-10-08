#pragma once
#include "Definition.h"
#include "Buffer.h"

class Matrix {
public:
	string filename; //交互文件名
	int rowSize; //行数
	int columnSize; //列数
	Buffer buf; //缓存

	/// @brief 从文件读入矩阵
	/// @param filename 文件名
	Matrix(string filename)
		: filename(filename), buf(filename) {
		ifstream file(this->filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.read((char*)&this->rowSize, sizeof(int));
		file.read((char*)&this->columnSize, sizeof(int));
		file.close();
	}

	/// @brief 随机生成矩阵
	/// @param rowSize 行数
	/// @param columnSize 列数
	/// @param filename 文件名
	Matrix(int rowSize, int columnSize, string filename, string random)
		: filename(filename), rowSize(rowSize), columnSize(columnSize) {
		ofstream file(this->filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.write((char*)&this->rowSize, sizeof(int));
		file.write((char*)&this->columnSize, sizeof(int));
		for (size_t i = 0; i < rowSize * columnSize; i++) {
			double num = rand() % 100;
			file.write((char*)&num, sizeof(double));
		}
		file.close();
		buf.init(filename);
	}

	/// @brief 构造0矩阵
	/// @param rowSize 
	/// @param columnSize 
	/// @param filename 
	Matrix(int rowSize, int columnSize, string filename)
		: filename(filename), rowSize(rowSize), columnSize(columnSize) {
		ofstream file(this->filename, ios::binary);
		if (!file.is_open())
			throw "File Open Failed!";
		file.write((char*)&this->rowSize, sizeof(int));
		file.write((char*)&this->columnSize, sizeof(int));
		double num = 0.0;
		for (size_t i = 0; i < rowSize * columnSize; i++)
			file.write((char*)&num, sizeof(double));
		file.close();
		buf.init(filename);
	}

	//Matrix() : filename(""), rowSize(0), columnSize(0) {}//默认构造函数  需要手动初始化

	/// @brief ijk矩阵乘法
	/// @param B 另一个矩阵
	/// @return 结果矩阵
	Matrix ijk_multiple(Matrix& B) {
		Matrix C(this->rowSize, B.columnSize, "ijk_multiple.bin");
		for (size_t i = 0; i < this->rowSize; i++)
			for (size_t j = 0; j < B.columnSize; j++)
				for (size_t k = 0; k < this->columnSize; k++)
					C.buf.setNum(i, j, C.buf.getNum(i, j) + this->buf.getNum(i, k) * B.buf.getNum(k, j));
		return C;
	}
	/// @brief ikj矩阵乘法
	/// @param B 另一个矩阵
	/// @return 结果矩阵
	Matrix ikj_multiple(Matrix& B) {
		Matrix C(this->rowSize, B.columnSize, "ikj_multiple.bin");
		for (size_t i = 0; i < this->rowSize; i++)
			for (size_t k = 0; k < this->columnSize; k++)
				for (size_t j = 0; j < B.columnSize; j++)
					C.buf.setNum(i, j, C.buf.getNum(i, j) + this->buf.getNum(i, k) * B.buf.getNum(k, j));
		return C;
	}
	/// @brief 打印矩阵
	void print() {
		for (size_t i = 0; i < this->rowSize; i++) {
			for (size_t j = 0; j < this->columnSize; j++)
				cout << this->buf.getNum(i, j) << " ";
			cout << endl;
		}
	}
	/// @brief 打印统计信息
	void printStats() {
		cout << "visit: " << this->buf.visit << endl;
		cout << "miss: " << this->buf.miss << endl;
		cout << "hitRate: " << this->buf.hitRate() << endl;
	}
};

