#include <pag/file.h>
#include <pag/pag.h>
#include <fstream>
#include <iostream>
#include "codec/mp4/MP4BoxHelper.h"

// 定义一个从 pag::PAGFile 继承的类，以便访问其中的 protected 方法
class MyPAGFile : public pag::PAGFile {
 public:
  using pag::PAGFile::PAGFile;  // Inherit constructors

  const pag::Layer* publicGetLayer() {
    return this->getLayer();
  }
};

int main() {
  // 加载 PAG 文件
  std::shared_ptr<MyPAGFile> myPagFile = std::static_pointer_cast<MyPAGFile>(
      pag::PAGFile::Load("../source/video_sequence_with_mp4header.pag"));

  // Now you can call the function
  const pag::Layer* layer = myPagFile->publicGetLayer();

  if (myPagFile == nullptr) {
    std::cerr << "加载 PAG 文件失败." << std::endl;
    return -1;
  }

  // 确保 PAG 文件的图层类型是 PreCompose
  if (myPagFile->layerType() != pag::LayerType::PreCompose) {
    std::cerr << "PAG 文件的图层类型不是 PreCompose." << std::endl;
    return -1;
  }

  // 获取 PAG 文件的图层并将其转换为 PreComposeLayer
  const pag::PreComposeLayer* preComposeLayer =
      static_cast<const pag::PreComposeLayer*>(myPagFile->publicGetLayer());

  // 确保 PreComposeLayer 的 composition 存在且其类型为 Video
  if (preComposeLayer->composition == nullptr) {
    std::cerr << "PreComposeLayer 的 composition 为空." << std::endl;
    return -1;
  } else {
    auto type = preComposeLayer->composition->type();
    std::cout << "Composition 类型为: " << static_cast<int>(type) << std::endl;
    if (type != pag::CompositionType::Video) {
      std::cerr << "PreComposeLayer 的 composition 类型不是 Video." << std::endl;
      return -1;
    }
  }

  // 将 PreComposeLayer 的 composition 转换为 VideoComposition
  pag::VideoComposition* videoComposition =
      static_cast<pag::VideoComposition*>(preComposeLayer->composition);

  // 确保 VideoComposition 的 sequences 不为空
  if (videoComposition->sequences.empty()) {
    std::cerr << "VideoComposition 的 sequences 为空." << std::endl;
    return -1;
  }

  // 获取 VideoComposition 的 sequences 的第一个元素
  const auto* videoSequence = videoComposition->sequences.at(0);

  // 将视频序列转换为 MP4 数据
  auto MP4Data = pag::MP4BoxHelper::CovertToMP4(videoSequence);
  if (MP4Data == nullptr) {
    std::cerr << "将视频序列转换为 MP4 数据失败." << std::endl;
    return -1;
  }

  // 将 MP4 数据保存到文件
  std::ofstream outFile("output.mp4", std::ios::binary);
  outFile.write(reinterpret_cast<const char*>(MP4Data->data()), MP4Data->length());

  std::cout << "PAG 文件已成功导出为视频." << std::endl;
  return 0;
}
