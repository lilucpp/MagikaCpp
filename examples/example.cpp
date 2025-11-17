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
    // Initialize MagikaScanner
    std::string model_path = argv[1];
    MagikaScanner::initialize(model_path);
    std::cout << "MagikaScanner initialized with model: " << model_path << std::endl;
    
    // Process each file
    for (int i = 2; i < argc; ++i) {
      std::string filepath(argv[i]);
      
      try {
        // Scan file and get content type and confidence score
        auto result = MagikaScanner::scanFileWithScore(filepath);
        
        // Print results
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