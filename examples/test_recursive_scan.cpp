#include "magikacpp.h"
#include <iostream>
#include <string>
#include <filesystem>

// Recursively traverse directory and identify file types
void ScanDirectoryRecursive(const std::string& path) {
  try {
    // Use std::filesystem to traverse directory (C++17 feature)
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
      if (entry.is_regular_file()) {
        try {
          std::string filepath = entry.path().string();
          std::pair<std::string, float> result = MagikaScanner::scanFileWithScore(filepath);
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

  std::string model_path = argv[1];
  std::string directory_path = argv[2];

  try {
    // Initialize MagikaScanner
    MagikaScanner::initialize(model_path);
    std::cout << "MagikaScanner initialized with model: " << model_path << std::endl;

    // Recursively scan directory
    ScanDirectoryRecursive(directory_path);

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}