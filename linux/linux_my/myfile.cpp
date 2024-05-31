// myfile.cpp

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

std::vector<std::string> processFiles(const std::string& directory = "../source/frames") {
  // 正则表达式，用于匹配以数字开头的png文件
  std::regex pattern("^\\d+.*\\.png$");

  // 存储匹配的文件名
  std::vector<std::string> files;

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
    try {
      return std::stoi(a.substr(0, a.find("."))) < std::stoi(b.substr(0, b.find(".")));
    } catch (const std::invalid_argument&) {
      return false;
    }
  });

  // 将目录名添加到每个文件名的前面以得到完整的路径
  for (auto& file : files) {
    file = directory + "/" + file;
  }

  // 返回文件路径列表
  return files;
}
