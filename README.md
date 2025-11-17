# Magika C++ 封装

本项目为 Magika 文件类型检测模型提供了一个 C++ 封装。Magika 是一个基于神经网络的文件类型检测系统，能够以高精度识别超过 100 种文件类型。

## 功能特性

- 使用 ONNX Runtime 进行快速文件类型检测
- 纯 C++ 实现，除了 ONNX Runtime 外无其他外部依赖
- 采用按需读取的内存高效文件处理方式
- 跨平台支持（Windows、Linux）
- 与原始 Magika Python 实现兼容

## 前提条件

1. 支持 C++17 的 C++ 编译器
2. CMake 3.12 或更高版本
3. ONNX Runtime C/C++ 库（在 onnxruntime-static-1.17.3/ 中提供）

## 构建方法

### Windows

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux

```bash
mkdir build
cd build
cmake .. -DONNXRUNTIME_ROOT=../onnxruntime-static-1.17.3/linux
make
```

## 使用方法

### 命令行示例

构建完成后，您可以使用示例程序扫描文件：

```cmd
# Windows
.\build\Release\pure_cpp_example.exe .\models\standard_v3_3\model.onnx example_file.txt

# Linux
./build/pure_cpp_example ./models/standard_v3_3/model.onnx example_file.txt
```

### 在您的 C++ 项目中使用

要在您自己的项目中使用 Magika C++ 封装：

1. 包含头文件：
   ```cpp
   #include "magika_cpp_wrapper.h"
   ```

2. 使用模型路径初始化扫描器：
   ```cpp
   MagikaScanner::initialize("./models/standard_v3_3/model.onnx");
   ```

3. 扫描文件：
   ```cpp
   try {
       // 只获取文件类型
       std::string filetype = MagikaScanner::scanFile("/path/to/file.txt");
       std::cout << "检测到的文件类型: " << filetype << std::endl;
       
       // 或者获取文件类型和置信度分数：
       auto result = MagikaScanner::scanFileWithScore("/path/to/file.txt");
       std::cout << "文件类型: " << result.first << ", 置信度: " << result.second << std::endl;
   } catch (const MagikaException& e) {
       std::cerr << "错误: " << e.what() << std::endl;
   }
   ```

## 架构

项目由几个组件组成：

1. **Magika C++ 核心** ([magika_cpp_pure.cpp](file:///h:/wkspace/test_magic_model/magika_cpp_pure.cpp))：C++ 中的 Magika 算法主要实现
2. **C 封装** ([magika_cpp_wrapper.cpp/.h](file:///h:/wkspace/test_magic_model/magika_cpp_wrapper.h))：便于集成的 C 风格 API
3. **Seekable 类** ([Seekable.cpp/.h](file:///h:/wkspace/test_magic_model/Seekable.h))：支持按需访问的高效文件读取
4. **特征提取器** ([features.cpp/.h](file:///h:/wkspace/test_magic_model/features.h))：从文件中提取特征
5. **配置读取器** ([config.cpp/.h](file:///h:/wkspace/test_magic_model/config.h))：从 JSON 读取模型配置
6. **示例** ([pure_cpp_example.cpp](file:///h:/wkspace/test_magic_model/pure_cpp_example.cpp), [test_recursive_scan.cpp](file:///h:/wkspace/test_magic_model/test_recursive_scan.cpp))：库的使用示例

## 内存效率

与传统的一次性将整个文件加载到内存的方法不同，此实现使用"seekable"方法，只读取文件的必要部分：

- 开始块（可配置大小）
- 中间块（可配置大小）
- 结束块（可配置大小）
- 特定偏移块（在配置中启用时）

这种方法显著减少了内存使用，特别是对于大文件。

## 模型信息

项目包含 Magika `standard_v3_3` 模型，可以识别超过 100 种文件类型，包括：
- 文本文件（txt、html、xml、json 等）
- 编程语言（cpp、py、js、java 等）
- 文档（pdf、doc、xls 等）
- 图像（jpg、png、gif 等）
- 压缩文件（zip、tar、rar 等）
- 还有更多...

## 许可证

本项目采用 Apache 2.0 许可证 - 有关详细信息，请参见 LICENSE 文件。

## 致谢

- 本项目使用了 Google 开发的 Magika 模型
- 使用 ONNX Runtime 进行神经网络推理