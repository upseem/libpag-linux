#include <algorithm>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

int main() {
  // 正则表达式，用于匹配以数字开头的png文件
  std::regex pattern("^\\d+.*\\.png$");

  // 存储匹配的文件名
  std::vector<std::string> files;

  // 声明并初始化目录名
  std::string directory = "../source/frames";

  // 遍历目录下的所有文件
  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
    // 获取文件名
    std::string filename = entry.path().filename().string();
    // 如果文件名符合正则表达式的规则，则存入vector
    if (std::regex_match(filename, pattern)) {
      files.push_back(filename);
    }
  }

  // 对文件名进行排序
  // 这里的排序规则是：提取文件名中的数字，然后按照数字的大小进行排序
  std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) {
    return std::stoi(a.substr(0, a.find("."))) < std::stoi(b.substr(0, b.find(".")));
  });

  // 打印排序后的文件名，并加上目录名
  for (const auto& file : files) {
    std::cout << directory << "/" << file << std::endl;
  }

  // 根据你的需求加载文件
  // ...

  return 0;
}
