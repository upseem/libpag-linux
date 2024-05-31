## 安装系统

```bash



PyTorch 2.1.0
Cuda 12.1
Python 3.10(ubuntu22.04)


# 安装最新版nodejs

# installs nvm (Node Version Manager)
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash

source ~/.bashrc

nvm -v  # 0.39.7

nvm install 22

node -v # should print `v22.2.0`

npm -v # should print `10.7.0`

#换阿里云镜像
npm config set registry https://registry.npmmirror.com
npm config set strict-ssl false

npm install -g depsync
depsync --version			#Version 1.4.0


sudo apt-get install git-lfs
sudo apt-get install ninja-build -y
sudo apt-get install libx11-dev -y
# 选装
sudo apt-get install ffmpeg libavcodec-dev libavformat-dev libavutil-dev
sudo apt-get install libjsoncpp-dev #json包

# cmake version 3.25.0
# gcc version 11.4.0 (Ubuntu 11.4.0-1ubuntu1~22.04)

#4.3.57版本
git clone  https://github.com/Tencent/libpag.git   

cd libpag
depsync


# 切换到包含CMakeLists.txt的目录
cd libpag/linux

./build_pag.sh 

# 创建一个新的目录来存放构建文件
mkdir build
cd build

# 使用CMake来生成Makefile
cmake ..

# 使用make命令来编译你的程序
make

# 运行你的程序
./pag-linux
```