#include <inttypes.h>
#include <myfile.h>
#include <pag/file.h>
#include <pag/pag.h>
#include <algorithm>
#include <iostream>

class MyPAGFile : public pag::PAGFile {
 public:
  using pag::PAGFile::PAGFile;  // Inherit constructors

  bool publicGotoTime(int64_t layerTime) {
    return this->gotoTime(layerTime);
  }

  bool publicStretchedFrameDuration() {
    return this->stretchedFrameDuration();
  }
  bool publicStretchedContentFrame() {
    return this->stretchedContentFrame();
  }
};

//这个函数将时间（微秒）转换为帧编号。
int64_t TimeToFrame(int64_t time, float frameRate) {
  return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
}

int main(int argc, char** argv) {

  // 在所有分支之前声明 myPagFile
  std::shared_ptr<MyPAGFile> myPagFile;

  // 检查是否有命令行参数 "-c"
  auto argument = std::find(argv, argv + argc, std::string("-c"));
  if (argument != argv + argc && ++argument != argv + argc) {
    // 如果找到了 "-c"，并且它后面还有一个参数
    // 使用该参数来加载 PAG 文件
    std::shared_ptr<MyPAGFile> myPagFile =
        std::static_pointer_cast<MyPAGFile>(MyPAGFile::Load(*argument));

  } else {
    // 如果没有找到 "-c"，或者它后面没有参数
    // 停止程序
    std::cerr << "No PAG file provided. Use -c to specify the file path.\n";
    return 1;
  }

  // 获取标签级别
  uint16_t tagLevel = myPagFile->tagLevel();
  std::cout << "Tag Level: " << tagLevel << std::endl;

  // 获取文本数量
  int numTexts = myPagFile->numTexts();
  std::cout << "文本图层: " << numTexts << std::endl;

  // 获取图像数量
  int numImages = myPagFile->numImages();
  std::cout << "图片图层: " << numImages << std::endl;

  // 获取视频数量
  int numVideos = myPagFile->numVideos();
  std::cout << "BMP视频图层(不可修改): " << numVideos << std::endl;

  // 获取文件路径
  std::string path = myPagFile->path();
  std::cout << "File Path: " << path << std::endl;

  // 获取文本数据
  // 注意：这里的索引为示例，可能需要根据实际情况修改
  auto textData = myPagFile->getTextData(0);
  // TODO: Process textData

  // 替换文本
  // 注意：这里使用的 textData 为空，需要替换为具体的文本数据
  int textIndex = 0;
  auto textDocumentHandle = myPagFile->getTextData(textIndex);
  textDocumentHandle->text = "这是文本";
  myPagFile->replaceText(textIndex, textDocumentHandle);

  // 替换图像
  // 注意：这里使用的图像为空，需要替换为具体的图像
  // auto pagImage = pag::PAGImage::FromPath("../source/1.jpg");
  // myPagFile->replaceImage(0, pagImage);

  // 通过名称替换图像
  // 注意：这里使用的图像为空，需要替换为具体的图像
  // pagFile->replaceImageByName("ImageLayerName", nullptr);

  // 获取层
  // 注意：这里的索引为示例，可能需要根据实际情况修改
  auto layers = myPagFile->getLayersByEditableIndex(0, pag::LayerType::Image);

  // 获取和设置时间拉伸模式
  auto timeStretchMode = myPagFile->timeStretchMode();
  myPagFile->setTimeStretchMode(timeStretchMode);

  // 设置持续时间
  // 注意：这里的持续时间为示例，可能需要根据实际情况修改
  myPagFile->setDuration(1000);

  // 检查是否为 PAG 文件
  bool isPAGFile = myPagFile->isPAGFile();
  std::cout << "Is PAG File: " << std::boolalpha << isPAGFile << std::endl;

  // 获取可编辑的索引
  auto editableIndices = myPagFile->getEditableIndices(pag::LayerType::Image);
  // TODO: Process editableIndices

  // 获取拉伸帧的持续时间和内容帧
  auto stretchedFrameDuration = myPagFile->publicStretchedFrameDuration();
  auto stretchedContentFrame = myPagFile->publicStretchedContentFrame();
  std::cout << "Stretched Frame Duration: " << stretchedFrameDuration << std::endl;
  std::cout << "Stretched Content Frame: " << stretchedContentFrame << std::endl;

  // 创建一个离屏表面（offscreen surface），其宽度和高度与 PAG 文件的宽度和高度相同。
  // 离屏表面是一种可以在内存中进行绘制，但不会直接显示在屏幕上的表面。
  auto pagSurface = pag::PAGSurface::MakeOffscreen(myPagFile->width(), myPagFile->height());
  // 检查创建的离屏表面是否为空。如果为空，打印错误信息，并返回 -1 退出程序。
  if (pagSurface == nullptr) {
    printf("---pagSurface is nullptr!!!\n");
    return -1;
  }

  // 创建一个 PAG 播放器（PAGPlayer）的实例。
  auto pagPlayer = new pag::PAGPlayer();

  // 将创建的离屏表面设置为 PAG 播放器的表面。PAG 播放器将在此表面上进行绘制。
  pagPlayer->setSurface(pagSurface);

  // 将 PAG 文件设置为 PAG 播放器的组成部分。PAG 播放器将播放这个 PAG 文件。
  pagPlayer->setComposition(myPagFile);

  // 计算总帧数。使用 PAG 文件的持续时间和帧率进行计算。
  auto totalFrames = TimeToFrame(myPagFile->duration(), myPagFile->frameRate());

  // 打印 PAG 文件的持续时间（单位为微秒）。
  printf("PAG 文件的持续时间（单位为微秒）: %" PRId64 " microseconds\n", myPagFile->duration());

  // 打印 PAG 文件的持续时间（单位为秒）。
  printf("PAG 文件的持续时间（单位为秒）: %f seconds\n", myPagFile->duration() / 1000000.0);

  // 打印 PAG 文件的帧率（单位为每秒帧数）。
  printf("PAG 文件的帧率（单位为每秒帧数）: %f frames per second\n", myPagFile->frameRate());

  // 打印计算得到的总帧数。
  printf("----总帧数--:%ld \n", totalFrames);

  myPagFile->publicGotoTime(1000);  // Now you can call the function

  return 0;
}
