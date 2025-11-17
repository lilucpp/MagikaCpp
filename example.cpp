#include "magika_cpp_wrapper.h"
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path1> [file_path2] ..." << std::endl;
        return 1;
    }

    // Process each file provided as argument
    for (int i = 1; i < argc; ++i) {
        std::string filepath(argv[i]);
        
        try {
            // Scan file and get both content type and confidence score
            auto result = MagikaScanner::scanFileWithScore(filepath);
            
            // Print the result
            std::cout << filepath << ": " << result.first 
                      << " (confidence: " << result.second << ")" << std::endl;
                      
        } catch (const MagikaException& e) {
            std::cerr << "Error scanning " << filepath << ": " << e.what() << std::endl;
            continue;
        }
    }
    
    return 0;
}