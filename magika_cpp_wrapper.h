#ifndef MAGIKA_CPP_WRAPPER_H
#define MAGIKA_CPP_WRAPPER_H

#include <string>
#include <utility>
#include <stdexcept>
#include <memory>

/**
 * Exception thrown when there is an error scanning a file
 */
class MagikaException : public std::runtime_error {
public:
    explicit MagikaException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * Simple C++ wrapper for Magika file type detection
 */
class MagikaScanner {
public:
    /**
     * Initialize the MagikaScanner with the path to the ONNX model
     * @param model_path Path to the model.onnx file
     */
    static void initialize(const std::string& model_path);
    
    /**
     * Scan a file and return its content type
     * @param filepath Path to the file to scan
     * @return String representation of the detected content type
     * @throws MagikaException if there is an error scanning the file
     */
    static std::string scanFile(const std::string& filepath);
    
    /**
     * Scan a file and return its content type with confidence score
     * @param filepath Path to the file to scan
     * @return Pair of content type and confidence score
     * @throws MagikaException if there is an error scanning the file
     */
    static std::pair<std::string, float> scanFileWithScore(const std::string& filepath);
};

#endif // MAGIKA_CPP_WRAPPER_H