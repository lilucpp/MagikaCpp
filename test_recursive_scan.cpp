#include "magika_cpp_wrapper.h"
#include <iostream>
#include <string>
#include <filesystem>

// 递归遍历目录并识别文件类型
void scanDirectoryRecursive(const std::string& path, MagikaScanner& scanner) {
    try {
        // 使用std::filesystem遍历目录 (C++17特性)
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                try {
                    std::string filepath = entry.path().string();
                    std::pair<std::string, float> result = scanner.scanFileWithScore(filepath);
                    std::cout << filepath << ": " << result.first 
                              << " (confidence: " << result.second << ")" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error scanning file " << entry.path().string() 
                              << ": " << e.what() << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error accessing directory " << path << ": " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <model_path> <directory_path>" << std::endl;
        return 1;
    }

    std::string modelPath = argv[1];
    std::string directoryPath = argv[2];

    try {
        // 初始化MagikaScanner
        MagikaScanner scanner;
        scanner.initialize(modelPath);
        std::cout << "MagikaScanner initialized with model: " << modelPath << std::endl;

        // 递归扫描目录
        scanDirectoryRecursive(directoryPath, scanner);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}