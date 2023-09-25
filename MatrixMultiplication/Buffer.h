
#pragma once
#include "Definition.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

using namespace std;

typedef pair<unsigned int, unsigned int> Locate;

/// @brief Locate's hash, used for hashmap
struct Locate_hash {
  template <class T, class U> int operator()(const pair<T, U> &a) const {
    return hash<int>()(a.first) * 17 ^ hash<int>()(a.second) * 71;
  }
};

/// @brief ���黺��
class Buffer {
public:
  unordered_map<Locate, double, Locate_hash> Buf;
  vector<Locate> visitList;

  double getNum(int row, int column);

  void setNum(int row, int column);
};

double Buffer::getNum(int row, int column) {
  // ��ȡ�����ڵ����� �׳�string�쳣
  if (this->Buf.count(Locate(row, column)))
    throw "Not Found in Buf!";
  // ˢ�·��ʶ���
  this->visitList.erase(
      find(visitList.begin(), visitList.end(), Locate(row, column)));
  return this->Buf[Locate(row, column)];
}

void Buffer::setNum(int row, int column) {
  if (this->Buf.size() >= BUFFER_SIZE) {
    // TODO �Ƴ����ϵ�
  }
  // TODO �ļ�����
  // TODO �����Ԫ��
}
