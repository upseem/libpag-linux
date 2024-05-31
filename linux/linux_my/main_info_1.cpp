#include <inttypes.h>
#include <myfile.h>
#include <pag/file.h>
#include <pag/pag.h>
#include <algorithm>
#include <iostream>

//这个函数将时间（微秒）转换为帧编号。
int64_t TimeToFrame(int64_t time, float frameRate) {
  return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
}

int main() {

  auto pagFile = pag::PAGFile::Load("../source/ae.pag");

  auto pagSurface = pag::PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
  if (pagSurface == nullptr) {
    printf("---pagSurface is nullptr!!!\n");
    return -1;
  }
  auto pagPlayer = new pag::PAGPlayer();
  pagPlayer->setSurface(pagSurface);
  pagPlayer->setComposition(pagFile);

  auto totalFrames = TimeToFrame(pagFile->duration(), pagFile->frameRate());
  auto currentFrame = 0;

  int bytesLength = pagFile->width() * pagFile->height() * 4;

  // 获取标签级别
  uint16_t tagLevel = pagFile->tagLevel();
  std::cout << "Tag Level: " << tagLevel << std::endl;

  // 获取文本数量
  int numTexts = pagFile->numTexts();
  std::cout << "文本图层: " << numTexts << std::endl;

  // 获取图像数量
  int numImages = pagFile->numImages();

  std::cout << "图片图层: " << numImages << std::endl;

  // 当当前帧的序号小于或等于总帧数时，继续循环。
  while (currentFrame <= totalFrames) {

    // 设置 PAG 播放器的播放进度。播放进度的范围是 0.0 到 1.0，
    // 其中 0.0 表示播放的开始，1.0 表示播放的结束。
    pagPlayer->setProgress(currentFrame * 1.0 / totalFrames);

    // 执行一次 PAG 播放器的刷新操作。刷新操作将会根据播放进度，
    // 在 PAG 播放器的表面上进行一次绘制。返回值 status 是一个枚举值，
    // 表示刷新操作的状态。
    auto status = pagPlayer->flush();

    // 打印当前帧的序号和刷新操作的状态。
    printf("---currentFrame:%d, flushStatus:%d \n", currentFrame, status);
    pagPlayer->

        // 创建一个用于存储图像数据的数组。数组的长度为每帧图像的字节数。
        auto data = new uint8_t[bytesLength];

    // 将当前帧的序号转换为字符串，用作图像文件的名称。
    std::string imageName = std::to_string(currentFrame);

    delete[] data;

    // 将当前帧的序号加 1，准备处理下一帧。
    currentFrame++;
  }
  // 获取索引为 0 的 ImageLayer

  // std::vector<pag::ImageLayer*> imageLayers = pagFile->;
  // // 现在你可以遍历这个 vector，并对每一个 ImageLayer 进行操作
  // for (pag::ImageLayer* imageLayer : imageLayers) {
  //   // 对 imageLayer 进行你需要的操作
  // }

  // 获取图层
  // 遍历每个图像图层
  for (int i = 0; i < numImages; i++) {
    auto layers = pagFile->getLayersByEditableIndex(i, pag::LayerType::Image);

    for (const auto& layer : layers) {
      std::cout << "Layer Name: " << layer->layerName() << std::endl;
      std::cout << "Layer Start Time: " << layer->startTime() << std::endl;
      std::cout << "Layer End Time: " << layer->startTime() + layer->duration() << std::endl;
    }
    if (!layers.empty()) {
      auto layer = layers[0];

      printf("------图层索引: %d :\n", i);
      // 获取开始帧
      int64_t startFrame = layer->startTime();
      // std::cout << "Layer " << i << " Start Frame: " << startFrame << std::endl;
      auto startZhen = TimeToFrame(startFrame, layer->frameRate());
      printf("开始时间 : %f seconds\n", startFrame / 1000000.0);
      printf("开始帧数 : %" PRId64 "\n", startZhen);

      // 获取结束帧
      int64_t endFrame = startFrame + layer->duration();
      auto endZhen = TimeToFrame(endFrame, layer->frameRate());
      // std::cout << "Layer " << i << " End Frame: " << endFrame << std::endl;
      printf("结束时间 : %f seconds\n", endFrame / 1000000.0);
      printf("结束帧数 : %" PRId64 "\n", endZhen);

      // 获取总共帧
      int64_t totalla = layer->duration();
      auto totalZhen = TimeToFrame(totalla, layer->frameRate());
      // std::cout << "Layer " << i << " End Frame: " << endFrame << std::endl;
      printf("总计时间 : %f seconds\n", totalla / 1000000.0);
      printf("总计帧数 : %" PRId64 "\n", totalZhen);

      printf("--------------------------:\n");
    } else {
      std::cout << "No layer found for index " << i << "." << std::endl;
    }
  }

  // 获取视频数量
  int numVideos = pagFile->numVideos();
  std::cout << "BMP视频图层(不可修改): " << numVideos << std::endl;

  // 获取视频宽度
  int width = pagFile->width();
  std::cout << "视频宽px: " << width << std::endl;

  // 获取视频高度
  int height = pagFile->height();
  std::cout << "视频高px: " << height << std::endl;

  // 获取文件路径
  std::string path = pagFile->path();
  std::cout << "File Path: " << path << std::endl;

  // // 获取组合的边界
  // auto bounds = pagFile->getBounds();
  // std::cout << "Bounds: " << bounds << std::endl;  // 这需要你自己定义 << 运算符

  // // 获取可编辑的索引
  // int editableIndex = pagFile->editableIndex();
  // std::cout << "Editable Index: " << editableIndex << std::endl;

  // // 获取父组合
  // auto parent = pagFile->parent();
  // std::cout << "Parent: " << parent << std::endl;  // 这需要你自己定义 << 运算符

  // // 获取标记
  // auto markers = pagFile->markers();
  // std::cout << "Markers: " << markers << std::endl;  // 这需要你自己定义 << 运算符

  // // 获取持续时间
  // int64_t duration = pagFile->duration();
  // std::cout << "Duration: " << duration << std::endl;

  // // 获取帧率
  // float frameRate = pagFile->frameRate();
  // std::cout << "Frame Rate: " << frameRate << std::endl;

  // // 进入下一帧
  // pagFile->nextFrame();
  // std::cout << "Switched to next frame." << std::endl;

  std::cout << "File Path: " << path << std::endl;

  std::cout << "File Path: " << path << std::endl;

  // 获取层
  // 注意：这里的索引为示例，可能需要根据实际情况修改
  // auto layers = pagFile->getLayersByEditableIndex(0, pag::LayerType::Image);

  // 获取和设置时间拉伸模式
  auto timeStretchMode = pagFile->timeStretchMode();
  pagFile->setTimeStretchMode(timeStretchMode);

  // 获取可编辑的索引
  auto editableIndices = pagFile->getEditableIndices(pag::LayerType::Image);
  for (auto index : editableIndices) {
    std::cout << "Editable Index: " << index << std::endl;
  }

  std::cout << "Flush: " << pagPlayer->flush() << std::endl;
  std::cout << "Video Enabled: " << pagPlayer->videoEnabled() << std::endl;
  std::cout << "Cache Enabled: " << pagPlayer->cacheEnabled() << std::endl;
  std::cout << "Max Frame Rate: " << pagPlayer->maxFrameRate() << std::endl;
  // std::cout << "Matrix: " << pagPlayer->matrix() << std::endl;  // 需要实现 Matrix 类的打印功能
  std::cout << "Progress: " << pagPlayer->getProgress() << std::endl;
  pagPlayer->prepare();
  auto layers = pagPlayer->getLayersUnderPoint(1.1, 1.1);
  for (const auto& layer : layers) {
    std::cout << "Layer: " << layer << std::endl;  // 需要实现 PAGLayer 类的打印功能
  }
  std::cout << "Rendering Time: " << pagPlayer->renderingTime() << std::endl;
  std::cout << "Image Decoding Time: " << pagPlayer->imageDecodingTime() << std::endl;
  std::cout << "Presenting Time: " << pagPlayer->presentingTime() << std::endl;
  std::cout << "Graphics Memory: " << pagPlayer->graphicsMemory() << std::endl;

  // 打印 PAG 文件的持续时间（单位为微秒）。
  printf("PAG 文件的持续时间（单位为微秒）: %" PRId64 " microseconds\n", pagFile->duration());

  // 打印 PAG 文件的持续时间（单位为秒）。
  printf("PAG 文件的持续时间（单位为秒）: %f seconds\n", pagFile->duration() / 1000000.0);

  // 打印 PAG 文件的帧率（单位为每秒帧数）。
  printf("PAG 文件的帧率（单位为每秒帧数）: %f frames per second\n", pagFile->frameRate());

  // 打印计算得到的总帧数。
  printf("----总帧数--:%ld \n", totalFrames);

  delete pagPlayer;

  return 0;
}
