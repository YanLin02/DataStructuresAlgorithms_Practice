#include "Buffer.h"

/// @brief ��ȡ����
/// @param row �к�
/// @param column �к�
/// @return ��Ӧ��˫���ȸ�����
double Buffer::getNum(int row, int column) {
	// ��ȡ�����ڵ����� �׳�string�쳣
	if (!this->Buf.count(Locate(row, column)))
		throw "Not Found in Buf!";
	return this->Buf[Locate(row, column)];
}

/// @brief д������
/// @param row �к�
/// @param column �к�
/// @param num ��Ҫд���˫���ȸ�����
void Buffer::setNum(int row, int column, double num) {
	if (this->Buf.count(Locate(row, column)))// �������
		this->Buf[Locate(row, column)] = num;// ֱ��д��
	else {// ���������
		writeBack();// д��
		this->readIn(Locate(row, column));// ����
		this->Buf[Locate(row, column)] = num;// д��
	}
	this->dirty = true;// ��λ��
}

/// @brief д��
void Buffer::writeBack() {
	ofstream file(this->filename, ios::binary);
	if (!file.is_open())
		throw "File Open Failed!";

	int filepos = this->columnSize * this->startLocate.first + this->startLocate.second;//�����ļ�λ��

	file.seekp((2 + filepos) * sizeof(double), ios::beg);//��λ���ļ�λ��

	for (size_t i = filepos; i < filepos + BUFFER_SIZE; i++)
		file.write((char*)&this->Buf[Locate(i / this->columnSize, i % this->columnSize)], sizeof(double));//д������

	file.close();
	this->dirty = false;//д�غ���λ����
}

/// @brief ��ָ��λ�ö���
/// @param startLocate ��ʼλ��
void Buffer::readIn(const Locate& startLocate) {
	if (this->dirty)
		writeBack();//д��

	this->startLocate = startLocate;//������ʼλ��
	this->clear();//��ջ���

	ifstream file(this->filename, ios::binary);
	if (!file.is_open())
		throw "File Open Failed!";

	int filepos = this->columnSize * this->startLocate.first + this->startLocate.second;//�����ļ�λ��

	file.seekg((2 + filepos) * sizeof(double), ios::beg);//��λ���ļ�λ��

	for (size_t i = filepos; i < filepos + BUFFER_SIZE; i++) {
		double num = -1.0;
		file.read((char*)&num, sizeof(double));//��ȡ����
		this->Buf.insert(make_pair(Locate(i / this->columnSize, i % this->columnSize), num));//д�뻺��
	}
	file.close();
	this->dirty = false;//�������λ����
}