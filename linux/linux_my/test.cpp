// test.cpp

#include <iostream>
#include "file.h"

int main() {
  std::vector<std::string> imagePaths = processFiles("../source/frames");

  // 遍历 imagePaths 并打印每个文件路径
  for (const auto& path : imagePaths) {
    std::cout << path << std::endl;
  }

  // 其他代码...
  return 0;
}
