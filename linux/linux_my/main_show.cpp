#include <json/json.h>
#include <pag/file.h>
#include <pag/pag.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::string ReadFile(const std::string& filepath) {
  std::ifstream ifs(filepath);
  return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

Json::Value ParseJson(const std::string& jsonStr) {
  Json::Value root;
  Json::CharReaderBuilder builder;
  std::string errs;
  std::istringstream iss(jsonStr);
  if (!Json::parseFromStream(builder, iss, &root, &errs)) {
    std::cerr << "Failed to parse JSON: " << errs << std::endl;
  }
  return root;
}

// Utility function to replace the text
void ReplaceText(pag::PAGFile* pagFile, int index, const std::string& newText,
                 const std::string& newFontPath) {
  if (index >= 0 && index < pagFile->numTexts()) {
    auto textData = pagFile->getTextData(index);
    textData->text = newText;

    if (!newFontPath.empty()) {
      auto newFont = pag::PAGFont::RegisterFont(newFontPath, 0);
      textData->fontFamily = newFont.fontFamily;
      textData->fontStyle = newFont.fontStyle;
    }

    pagFile->replaceText(index, textData);
  } else {
    std::cerr << "Text index out of range: " << index << std::endl;
  }
}

// Utility function to replace the image
void ReplaceImage(pag::PAGFile* pagFile, int index, const std::string& newImagePath) {
  if (index >= 0 && index < pagFile->numImages()) {
    auto newImage = pag::PAGImage::FromPath(newImagePath);
    pagFile->replaceImage(index, newImage);
  } else {
    std::cerr << "Image index out of range: " << index << std::endl;
  }
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

int main(int argc, char* argv[]) {

  // 默认的配置文件路径
  std::string configFilePath = "config.json";

  // 解析命令行参数
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-c" && i + 1 < argc) {
      // 如果找到 "-c" 参数，那么下一个参数就是配置文件路径
      configFilePath = argv[++i];
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      return -1;
    }
  }

  // 加载 JSON 配置
  std::string jsonStr = ReadFile("config.json");
  Json::Value config = ParseJson(jsonStr);

  // 加载 PAG 文件
  auto pagFile = pag::PAGFile::Load(config["pagFile"].asString());
  if (!pagFile) {
    std::cerr << "Failed to load PAG file" << std::endl;
    return -1;
  }

  // 设置后备字体
  std::vector<std::string> fallbackFontPaths;
  for (const auto& fontPath : config["fallbackFontPaths"]) {
    fallbackFontPaths.push_back(fontPath.asString());
  }
  std::vector<int> ttcIndices(fallbackFontPaths.size());
  pag::PAGFont::SetFallbackFontPaths(fallbackFontPaths, ttcIndices);

  // 应用替换
  for (const auto& replacement : config["replacements"]) {
    std::string type = replacement["type"].asString();
    int index = replacement["index"].asInt();
    std::string newContent = replacement["newContent"].asString();

    if (type == "text") {
      std::string newFontPath = replacement["fontPath"].asString();
      ReplaceText(pagFile.get(), index, newContent, newFontPath);
    } else if (type == "image") {
      ReplaceImage(pagFile.get(), index, newContent);
    }
  }

  // 创建PAGPlayer来播放PAG文件
  auto pagPlayer = new pag::PAGPlayer();
  pagPlayer->setComposition(pagFile);

  // 创建 PAGSurface 来渲染 PAG 文件
  auto pagSurface = pag::PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
  pagPlayer->setSurface(pagSurface);

  // 用于计算帧编号的函数
  auto TimeToFrame = [](int64_t time, float frameRate) {
    return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
  };

  // 计算总帧数
  auto totalFrames = TimeToFrame(pagFile->duration(), pagFile->frameRate());

  // 渲染每一帧并将其保存为 BMP 文件
  for (int i = 0; i <= totalFrames; i++) {
    // 设置PAGPlayer的进度
    pagPlayer->setProgress(i * 1.0 / totalFrames);

    // 渲染当前帧
    pagPlayer->flush();

    // 从 PAGSurface 读取像素
    auto data = new uint8_t[pagFile->width() * pagFile->height() * 4];
    pagSurface->readPixels(pag::ColorType::BGRA_8888, pag::AlphaType::Premultiplied, data,
                           pagFile->width() * 4);

    // 将当前帧保存为 BMP 文件
    std::string filename = config["outputPrefix"].asString() + std::to_string(i);
    BmpWrite(data, pagFile->width(), pagFile->height(), filename.c_str());

    // 清理
    delete[] data;
  }

  // 清理
  delete pagPlayer;

  return 0;
}
