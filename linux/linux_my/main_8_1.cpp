#include <inttypes.h>
#include <myfile.h>
#include <pag/file.h>
#include <pag/pag.h>

// std::vector<std::string> imagePaths = {/* 你的10张图像路径 */};
std::vector<std::string> imagePaths = processFiles("../source/ae");
std::shared_ptr<pag::PAGFile> g_pagFile;

int imageCount = imagePaths.size();

// 这个函数用于获取从程序启动到现在的时间，单位为微秒。
int64_t GetTimer() {
  static auto START_TIME = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - START_TIME);
  return static_cast<int64_t>(ns.count() * 1e-3);
}

//这个函数将 PAG 文件中的所有图片替换为指定的图片，所有文本替换为指定的文本，并且更改了文本的字体。
std::shared_ptr<pag::PAGFile> ReplaceImageOrText(int currentFrame) {
  if (currentFrame >= imagePaths.size()) {
    // 当currentFrame大于或等于imagePaths的长度时，返回nullptr
    return nullptr;
  }

  // auto pagFile = g_pagFile;
  // if (pagFile == nullptr) {
  //   return nullptr;
  // }

  auto pagImage = pag::PAGImage::FromPath(imagePaths[currentFrame]);
  printf("----img--:%s \n", imagePaths[currentFrame].c_str());
  g_pagFile->replaceImage(3, pagImage);

  return g_pagFile;
}

//这个函数将时间（微秒）转换为帧编号。
int64_t TimeToFrame(int64_t time, float frameRate) {
  return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
}

//这个函数将图像数据保存为 BMP 文件。
void BmpWrite(unsigned char* image, int imageWidth, int imageHeight, const char* filename) {
  unsigned char header[54] = {0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
                              0,    0,    0, 0, 0, 0, 0, 0, 1, 0, 32, 0, 0, 0, 0,  0, 0, 0,
                              0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0};

  int64_t file_size = static_cast<int64_t>(imageWidth) * static_cast<int64_t>(imageHeight) * 4 + 54;
  header[2] = static_cast<unsigned char>(file_size & 0x000000ff);
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  int64_t width = imageWidth;
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  int64_t height = -imageHeight;
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  char fname_bmp[128];
  sprintf(fname_bmp, "%s.bmp", filename);

  FILE* fp;
  if (!(fp = fopen(fname_bmp, "wb"))) {
    return;
  }

  fwrite(header, sizeof(unsigned char), 54, fp);
  fwrite(image, sizeof(unsigned char), (size_t)(int64_t)imageWidth * imageHeight * 4, fp);
  fclose(fp);
}

int main() {
  g_pagFile = pag::PAGFile::Load("../source/ae.pag");
  if (g_pagFile == nullptr) {
    printf("---pagFile is nullptr!!!\n");
    return -1;
  }
  auto startTime = GetTimer();
  // 注册后备字体。 它应该只在应用程序初始化时调用一次。
  std::vector<std::string> fallbackFontPaths = {};
  fallbackFontPaths.emplace_back("../../resources/font/NotoSerifSC-Regular.otf");
  fallbackFontPaths.emplace_back("../../resources/font/NotoColorEmoji.ttf");
  std::vector<int> ttcIndices(fallbackFontPaths.size());
  pag::PAGFont::SetFallbackFontPaths(fallbackFontPaths, ttcIndices);

  // auto pagFile = ReplaceImageOrText(0);
  // if (pagFile == nullptr) {
  //   printf("---pagFile is nullptr!!!\n");
  //   return -1;
  // }

  // 创建一个离屏表面（offscreen surface），其宽度和高度与 PAG 文件的宽度和高度相同。
  // 离屏表面是一种可以在内存中进行绘制，但不会直接显示在屏幕上的表面。
  auto pagSurface = pag::PAGSurface::MakeOffscreen(g_pagFile->width(), g_pagFile->height());
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
  pagPlayer->setComposition(g_pagFile);

  // 计算总帧数。使用 PAG 文件的持续时间和帧率进行计算。
  auto totalFrames = TimeToFrame(g_pagFile->duration(), g_pagFile->frameRate());

  // 打印 PAG 文件的持续时间（单位为微秒）。
  printf("Duration: %" PRId64 " microseconds\n", g_pagFile->duration());

  // 打印 PAG 文件的帧率（单位为每秒帧数）。
  printf("Frame Rate: %f frames per second\n", g_pagFile->frameRate());

  // 打印计算得到的总帧数。
  printf("----totalFrames--:%ld \n", totalFrames);

  // 初始化当前帧的序号为 0。
  auto currentFrame = 0;

  // 计算每帧图像的字节数（每像素 4 字节）。
  int bytesLength = g_pagFile->width() * g_pagFile->height() * 4;
  // 打印每帧图像的字节数。
  printf("----bytesLength--:%d \n", bytesLength);

  // 当当前帧的序号小于或等于总帧数时，继续循环。
  while (currentFrame <= totalFrames) {

    if (currentFrame < imageCount) {
      // 在每一帧的开始时替换图像
      g_pagFile = ReplaceImageOrText(currentFrame);
      if (g_pagFile == nullptr) {
        // 当ReplaceImageOrText返回nullptr时，停止渲染
        break;
      }
    }

    // 设置 PAG 播放器的播放进度。播放进度的范围是 0.0 到 1.0，
    // 其中 0.0 表示播放的开始，1.0 表示播放的结束。
    pagPlayer->setProgress(currentFrame * 1.0 / totalFrames);

    // 执行一次 PAG 播放器的刷新操作。刷新操作将会根据播放进度，
    // 在 PAG 播放器的表面上进行一次绘制。返回值 status 是一个枚举值，
    // 表示刷新操作的状态。
    auto status = pagPlayer->flush();

    // 打印当前帧的序号和刷新操作的状态。
    printf("---currentFrame:%d, flushStatus:%d \n", currentFrame, status);

    // 创建一个用于存储图像数据的数组。数组的长度为每帧图像的字节数。
    auto data = new uint8_t[bytesLength];

    // 从 PAG 播放器的表面上读取图像数据。读取的图像数据将被存储到刚刚创建的数组中。
    pagSurface->readPixels(pag::ColorType::BGRA_8888, pag::AlphaType::Premultiplied, data,
                           g_pagFile->width() * 4);

    // 将当前帧的序号转换为字符串，用作图像文件的名称。
    std::string imageName = std::to_string(currentFrame);

    // 将读取的图像数据保存为 BMP 文件。
    BmpWrite(data, g_pagFile->width(), g_pagFile->height(), imageName.c_str());

    delete[] data;

    // 将当前帧的序号加 1，准备处理下一帧。
    currentFrame++;
  }

  delete pagPlayer;

  printf("----timeCost--:%" PRId64 " \n", GetTimer() - startTime);

  return 0;
}
