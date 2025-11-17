#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

const std::string CONFIG_FILE = "config.min.json";
const std::string MODEL_FILE = "model.onnx";

/**
 * Config holds the portion of Magika's model configuration that is relevant
 * for inference.
 */
class Config {
public:
    int BegSize;
    int MidSize;
    int EndSize;
    bool UseInputsAtOffsets;
    float MediumConfidenceThreshold;
    long long MinFileSizeForDl;
    int PaddingToken;
    int BlockSize;
    std::vector<std::string> TargetLabelsSpace;
    std::map<std::string, float> Thresholds;
    std::map<std::string, std::string> Overwrite;
    
    /**
     * Default constructor
     */
    Config();
    
    /**
     * ReadConfig is a helper that reads and unmarshal a Config, given an assets
     * dir and a model name.
     * @param assetsDir Path to the assets directory
     * @param name Model name
     * @return Config object
     */
    static Config ReadConfig(const std::string& assetsDir, const std::string& name);
    
    /**
     * modelPath returns the Onnx model path for the given asset folder and model name.
     * @param assetDir Path to the asset directory
     * @param name Model name
     * @return Path to the model file
     */
    static std::string ModelPath(const std::string& assetDir, const std::string& name);

private:
    /**
     * configPath returns the model config path for the given asset folder and model name.
     * @param assetDir Path to the asset directory
     * @param name Model name
     * @return Path to the config file
     */
    static std::string ConfigPath(const std::string& assetDir, const std::string& name);
};

#endif // CONFIG_H