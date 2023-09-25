
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

/// @brief 数组缓存
class Buffer {
public:
  unordered_map<Locate, double, Locate_hash> Buf;
  vector<Locate> visitList;

  double getNum(int row, int column);

  void setNum(int row, int column);
};

double Buffer::getNum(int row, int column) {
  // 获取不存在的数据 抛出string异常
  if (this->Buf.count(Locate(row, column)))
    throw "Not Found in Buf!";
  // 刷新访问队列
  this->visitList.erase(
      find(visitList.begin(), visitList.end(), Locate(row, column)));
  return this->Buf[Locate(row, column)];
}

void Buffer::setNum(int row, int column) {
  if (this->Buf.size() >= BUFFER_SIZE) {
    // TODO 移除最老的
  }
  // TODO 文件操作
  // TODO 添加新元素
}
