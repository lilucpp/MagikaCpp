#ifndef MAGIKACPP_CONFIG_H_
#define MAGIKACPP_CONFIG_H_

#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

const std::string kConfigFile = "config.min.json";
const std::string kModelFile = "model.onnx";

/**
 * Config holds the portion of Magika's model configuration that is relevant
 * for inference.
 */
class Config {
 public:
  int beg_size;
  int mid_size;
  int end_size;
  bool use_inputs_at_offsets;
  float medium_confidence_threshold;
  long long min_file_size_for_dl;
  int padding_token;
  int block_size;
  std::vector<std::string> target_labels_space;
  std::map<std::string, float> thresholds;
  std::map<std::string, std::string> overwrite;
  
  /**
   * Default constructor
   */
  Config();
  
  /**
   * ReadConfig is a helper that reads and unmarshal a Config, given an assets
   * dir and a model name.
   * @param assets_dir Path to the assets directory
   * @param name Model name
   * @return Config object
   */
  static Config ReadConfig(const std::string& assets_dir, const std::string& name);
  
  /**
   * ModelPath returns the Onnx model path for the given asset folder and model name.
   * @param asset_dir Path to the asset directory
   * @param name Model name
   * @return Path to the model file
   */
  static std::string ModelPath(const std::string& asset_dir, const std::string& name);

 private:
  /**
   * ConfigPath returns the model config path for the given asset folder and model name.
   * @param asset_dir Path to the asset directory
   * @param name Model name
   * @return Path to the config file
   */
  static std::string ConfigPath(const std::string& asset_dir, const std::string& name);
};

#endif  // MAGIKACPP_CONFIG_H_