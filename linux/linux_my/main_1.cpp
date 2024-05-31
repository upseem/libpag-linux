#include <pag/file.h>
#include <pag/pag.h>
#include <iostream>

// 这个函数用于获取从程序启动到现在的时间，单位为微秒。
int64_t GetTimer() {
  static auto START_TIME = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - START_TIME);
  return static_cast<int64_t>(ns.count() * 1e-3);
}

// 将指定的文本层替换为新的文本
void ReplaceText(pag::PAGFile* pagFile, int textIndex, const std::string& newText) {
  if (textIndex >= 0 && textIndex < pagFile->numTexts()) {
    auto textDocumentHandle = pagFile->getTextData(textIndex);
    textDocumentHandle->text = newText;
    pagFile->replaceText(textIndex, textDocumentHandle);
  } else {
    std::cerr << "Text index out of range: " << textIndex << std::endl;
  }
}

// 将指定的图像层替换为新的图像
void ReplaceImage(pag::PAGFile* pagFile, int imageIndex, const std::string& newImagePath) {
  if (imageIndex >= 0 && imageIndex < pagFile->numImages()) {
    auto newImage = pag::PAGImage::FromPath(newImagePath);
    pagFile->replaceImage(imageIndex, newImage);
  } else {
    std::cerr << "Image index out of range: " << imageIndex << std::endl;
  }
}

// 将图像数据保存为 BMP 文件。
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
  auto startTime = GetTimer();

  // 注册后备字体。 它应该只在应用程序初始化时调用一次。
  std::vector<std::string> fallbackFontPaths = {};
  fallbackFontPaths.emplace_back("../../resources/font/NotoSerifSC-Regular.otf");
  fallbackFontPaths.emplace_back("../../resources/font/NotoColorEmoji.ttf");
  std::vector<int> ttcIndices(fallbackFontPaths.size());
  pag::PAGFont::SetFallbackFontPaths(fallbackFontPaths, ttcIndices);

  auto pagFile = pag::PAGFile::Load("../source/2.pag");
  if (pagFile == nullptr) {
    printf("---pagFile is nullptr!!!\n");
    return -1;
  }

  // 替换文本和图像
  ReplaceText(pagFile.get(), 1, "你好帅");
  ReplaceImage(pagFile.get(), 2, "../source/1.jpg");  // 这里是你要替换的图像的路径

  auto pagSurface = pag::PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
  if (pagSurface == nullptr) {
    printf("---pagSurface is nullptr!!!\n");
    return -1;
  }

  auto pagPlayer = new pag::PAGPlayer();
  pagPlayer->setSurface(pagSurface);
  pagPlayer->setComposition(pagFile);

  // 用于计算帧编号的函数
  auto TimeToFrame = [](int64_t time, float frameRate) {
    return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
  };

  auto totalFrames = TimeToFrame(pagFile->duration(), pagFile->frameRate());
  auto currentFrame = 0;

  int bytesLength = pagFile->width() * pagFile->height() * 4;

  while (currentFrame <= totalFrames) {
    pagPlayer->setProgress(currentFrame * 1.0 / totalFrames);
    auto status = pagPlayer->flush();

    printf("---currentFrame:%d, flushStatus:%d \n", currentFrame, status);

    auto data = new uint8_t[bytesLength];
    pagSurface->readPixels(pag::ColorType::BGRA_8888, pag::AlphaType::Premultiplied, data,
                           pagFile->width() * 4);

    std::string imageName = "output_" + std::to_string(currentFrame);
    BmpWrite(data, pagFile->width(), pagFile->height(), imageName.c_str());

    delete[] data;

    currentFrame++;
  }

  delete pagPlayer;

  printf("----timeCost--:%lld \n", GetTimer() - startTime);

  return 0;
}
