#include "magikacpp.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <model_path> <file_path1> [file_path2] ..." << std::endl;
        std::cerr << "Example: " << argv[0] << " ./models/standard_v3_3/model.onnx ./test.txt" << std::endl;
        return 1;
    }

    try {
        // 初始化MagikaScanner
        std::string model_path = argv[1];
        MagikaScanner::initialize(model_path);
        std::cout << "MagikaScanner initialized with model: " << model_path << std::endl;
        
        // 处理每个文件
        for (int i = 2; i < argc; ++i) {
            std::string filepath(argv[i]);
            
            try {
                // 扫描文件并获取内容类型和置信度分数
                auto result = MagikaScanner::scanFileWithScore(filepath);
                
                // 打印结果
                std::cout << filepath << ": " << result.first 
                          << " (confidence: " << result.second << ")" << std::endl;
                          
            } catch (const MagikaException& e) {
                std::cerr << "Error scanning " << filepath << ": " << e.what() << std::endl;
                continue;
            }
        }
        
    } catch (const MagikaException& e) {
        std::cerr << "Error initializing MagikaScanner: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "unknown exception..." << std::endl;
    }
    
    return 0;
}