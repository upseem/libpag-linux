#include <inttypes.h>
#include <json/json.h>
#include <pag/file.h>
#include <pag/pag.h>
#include <fstream>
#include <iostream>
#include <filesystem> // C++17 filesystem

void CreateDirectoryIfNotExists(const std::string &path)
{
  if (!std::filesystem::exists(path))
  {
    std::filesystem::create_directories(path);
  }
}

std::shared_ptr<pag::PAGFile> pagFile;

std::string ReadFile(const std::string &filepath)
{
  std::ifstream ifs(filepath);
  return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

Json::Value ParseJson(const std::string &jsonStr)
{
  Json::Value root;
  Json::CharReaderBuilder builder;
  std::string errs;
  std::istringstream iss(jsonStr);
  if (!Json::parseFromStream(builder, iss, &root, &errs))
  {
    std::cerr << "Failed to parse JSON: " << errs << std::endl;
  }
  return root;
}

// 这个函数用于获取从程序启动到现在的时间，单位为秒。
double GetTimer()
{
  static auto START_TIME = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  auto s = std::chrono::duration_cast<std::chrono::seconds>(now - START_TIME);
  return static_cast<double>(s.count());
}

// 替换文字
void ReplaceText(pag::PAGFile *pagFile, int index, const std::string &newText,
                 const std::string &newFontPath)
{
  if (index >= 0 && index < pagFile->numTexts())
  {
    auto textData = pagFile->getTextData(index);
    textData->text = newText;

    if (!newFontPath.empty())
    {
      auto newFont = pag::PAGFont::RegisterFont(newFontPath, 0);
      textData->fontFamily = newFont.fontFamily;
      textData->fontStyle = newFont.fontStyle;
    }

    pagFile->replaceText(index, textData);
  }
  else
  {
    std::cerr << "Text index out of range: " << index << std::endl;
  }
}

// 替换图片
void ReplaceOneImage(pag::PAGFile *pagFile, int index, const std::string &newImagePath)
{
  if (index >= 0 && index < pagFile->numImages())
  {
    auto newImage = pag::PAGImage::FromPath(newImagePath);
    pagFile->replaceImage(index, newImage);
  }
  else
  {
    std::cerr << "Image index out of range: " << index << std::endl;
  }
}

std::shared_ptr<pag::PAGFile> ReplaceImage(int index, const std::string &newImagePath)
{
  auto pagImage = pag::PAGImage::FromPath(newImagePath);
  pagFile->replaceImage(index, pagImage);
  return pagFile;
}

// 这个函数将时间（微秒）转换为帧编号。
int64_t TimeToFrame(int64_t time, float frameRate)
{
  return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
}

// 这个函数将图像数据保存为 BMP 文件。 这里的路径应以斜杠 (/) 结尾。
void BmpWrite(unsigned char *image, int imageWidth, int imageHeight, const char *filename, const char *path = nullptr)
{
  // 如果path为空，将其设置为当前目录
  std::string fullPath;
  if (path == nullptr)
  {
    fullPath = std::string("./") + std::string(filename) + std::string(".bmp");
  }
  else
  {
    // 检查path的最后一个字符是否为'/'
    if (path[strlen(path) - 1] == '/')
    {
      fullPath = std::string(path) + std::string(filename) + std::string(".bmp");
    }
    else
    {
      fullPath = std::string(path) + std::string("/") + std::string(filename) + std::string(".bmp");
    }
    CreateDirectoryIfNotExists(path);
  }

  unsigned char header[54] = {0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
  sprintf(fname_bmp, "%s/%s.bmp", path, filename);

  FILE *fp;
  if (!(fp = fopen(fullPath.c_str(), "wb")))
  {
    return;
  }

  fwrite(header, sizeof(unsigned char), 54, fp);
  fwrite(image, sizeof(unsigned char), (size_t)(int64_t)imageWidth * imageHeight * 4, fp);
  fclose(fp);
}

int main(int argc, char *argv[])
{
  auto startTime = GetTimer();

  // 默认的配置文件路径
  std::string configFilePath = "config.json";

  // 解析命令行参数
  for (int i = 1; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg == "-c" && i + 1 < argc)
    {
      // 如果找到 "-c" 参数，那么下一个参数就是配置文件路径
      configFilePath = argv[++i];
    }
    else
    {
      std::cerr << "Unknown argument: " << arg << std::endl;
      return -1;
    }
  }

  // 加载 JSON 配置
  std::string jsonStr = ReadFile(configFilePath);
  Json::Value config = ParseJson(jsonStr);

  std::cout << "----pagFile--:" << config["pagFile"].asString() << "\n";
  // 加载 PAG 文件
  pagFile = pag::PAGFile::Load(config["pagFile"].asString());
  if (!pagFile)
  {
    std::cerr << "Failed to load PAG file" << std::endl;
    return -1;
  }
  printf("----加载成功!!!\n");

  std::string outImagePath = config["outImagePath"].asString();
  const char *c_outImagePath = outImagePath.c_str();

  // 设置后备字体
  std::vector<std::string> fallbackFontPaths;
  for (const auto &fontPath : config["fallbackFontPaths"])
  {
    fallbackFontPaths.push_back(fontPath.asString());
  }
  std::vector<int> ttcIndices(fallbackFontPaths.size());
  pag::PAGFont::SetFallbackFontPaths(fallbackFontPaths, ttcIndices);
  printf("----设置字体成功!!!\n");
  // 应用替换
  for (const auto &replacement : config["replacements"])
  {
    std::string type = replacement["type"].asString();
    int index = replacement["index"].asInt();
    std::string newContent = replacement["newContent"].asString();

    if (type == "text")
    {
      std::string newFontPath = replacement["fontPath"].asString();
      ReplaceText(pagFile.get(), index, newContent, newFontPath);
      printf("----文字替换成功!!!\n");
    }
    else if (type == "image")
    {
      // 如果替换的是一张图片那么直接替换
      pagFile = ReplaceImage(index, newContent);
      printf("----图片替换成功!!!\n");
    }
  }

  // 创建一个离屏表面（offscreen surface），其宽度和高度与 PAG 文件的宽度和高度相同。
  // 离屏表面是一种可以在内存中进行绘制，但不会直接显示在屏幕上的表面。
  auto pagSurface = pag::PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
  // 检查创建的离屏表面是否为空。如果为空，打印错误信息，并返回 -1 退出程序。
  if (pagSurface == nullptr)
  {
    printf("----pagSurface is nullptr!!!\n");
    return -1;
  }
  printf("----文字图片替换成功!!!\n");
  // 创建一个 PAG 播放器（PAGPlayer）的实例。
  auto pagPlayer = new pag::PAGPlayer();

  // 将创建的离屏表面设置为 PAG 播放器的表面。PAG 播放器将在此表面上进行绘制。
  pagPlayer->setSurface(pagSurface);

  // 将 PAG 文件设置为 PAG 播放器的组成部分。PAG 播放器将播放这个 PAG 文件。
  pagPlayer->setComposition(pagFile);

  // 计算总帧数。使用 PAG 文件的持续时间和帧率进行计算。
  auto totalFrames = TimeToFrame(pagFile->duration(), pagFile->frameRate());

  // 打印 PAG 文件的持续时间（单位为微秒）。
  printf("Duration: %" PRId64 " microseconds\n", pagFile->duration());

  // 打印 PAG 文件的帧率（单位为每秒帧数）。
  printf("Frame Rate: %f frames per second\n", pagFile->frameRate());

  // 打印计算得到的总帧数。
  printf("----totalFrames--:%ld \n", totalFrames);

  // 初始化当前帧的序号为 0。
  auto currentFrame = 0;

  // 计算每帧图像的字节数（每像素 4 字节）。
  int bytesLength = pagFile->width() * pagFile->height() * 4;
  // 打印每帧图像的字节数。
  printf("----bytesLength--:%d \n", bytesLength);

  // 加载视频图层帧数配置currentFrames
  Json::Value currentVideoFrames = config["currentFrames"];

  // 当当前帧的序号小于或等于总帧数时，继续循环。
  while (currentFrame <= totalFrames)
  {
    printf("-----------------------------------\n");
    // 遍历图层配置
    for (const auto &frameConfig : currentVideoFrames)
    {
      int start = frameConfig[0].asInt();
      int end = frameConfig[1].asInt();
      int imageIndex = frameConfig[2].asInt();

      // 检查currentFrame是否在图层范围内
      if (currentFrame >= start && currentFrame <= end)
      {
        // 如果在当前视频的帧范围内，进行图片替换
        std::string imagePath = config["mp4Path"].asString() + std::to_string(imageIndex) + "_" +
                                std::to_string(currentFrame) + ".png";
        std::cout << "img " << imagePath << std::endl;
        // 检查图片是否存在
        std::ifstream ifile(imagePath);
        if (ifile)
        {
          pagFile = ReplaceImage(imageIndex, imagePath);
        }

        std::cout << "Frame " << currentFrame << " is in imageIndex " << imageIndex << std::endl;
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
                           pagFile->width() * 4);

    // 将当前帧的序号转换为字符串，用作图像文件的名称。
    std::string imageName = std::to_string(currentFrame);

    // 将读取的图像数据保存为 BMP 文件。
    BmpWrite(data, pagFile->width(), pagFile->height(), imageName.c_str(), c_outImagePath);

    delete[] data;

    // 将当前帧的序号加 1，准备处理下一帧。
    currentFrame++;
  }

  delete pagPlayer;

  std::cout << "----timeCost--:" << GetTimer() - startTime << "\n";

  return 0;
}
