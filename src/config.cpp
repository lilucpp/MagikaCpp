#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

Config::Config() : 
    beg_size(0),
    mid_size(0),
    end_size(0),
    use_inputs_at_offsets(false),
    medium_confidence_threshold(0.0f),
    min_file_size_for_dl(0),
    padding_token(0),
    block_size(0) {
}

Config Config::ReadConfig(const std::string& assets_dir, const std::string& name) {
  Config cfg;
  std::string path = ConfigPath(assets_dir, name);
  
  // Check if file exists
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("Config file not found: " + path);
  }
  
  // Read the JSON file
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open config file: " + path);
  }
  
  json json_data;
  try {
    file >> json_data;
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to parse JSON in config file: " + path + " - " + e.what());
  }
  
  // Parse the configuration values
  cfg.beg_size = json_data.value("beg_size", 0);
  cfg.mid_size = json_data.value("mid_size", 0);
  cfg.end_size = json_data.value("end_size", 0);
  cfg.use_inputs_at_offsets = json_data.value("use_inputs_at_offsets", false);
  cfg.medium_confidence_threshold = json_data.value("medium_confidence_threshold", 0.0f);
  cfg.min_file_size_for_dl = json_data.value("min_file_size_for_dl", 0LL);
  cfg.padding_token = json_data.value("padding_token", 0);
  cfg.block_size = json_data.value("block_size", 0);
  
  // Parse TargetLabelsSpace
  if (json_data.contains("target_labels_space") && json_data["target_labels_space"].is_array()) {
    for (const auto& label : json_data["target_labels_space"]) {
      cfg.target_labels_space.push_back(label.get<std::string>());
    }
  }
  
  // Parse Thresholds
  if (json_data.contains("thresholds") && json_data["thresholds"].is_object()) {
    for (auto& [key, value] : json_data["thresholds"].items()) {
      cfg.thresholds[key] = value.get<float>();
    }
  }
  
  // Parse Overwrite
  if (json_data.contains("overwrite_map") && json_data["overwrite_map"].is_object()) {
    for (auto& [key, value] : json_data["overwrite_map"].items()) {
      cfg.overwrite[key] = value.get<std::string>();
    }
  }
  
  return cfg;
}

std::string Config::ConfigPath(const std::string& asset_dir, const std::string& name) {
  return asset_dir + "/" + name + "/" + kConfigFile;
}

std::string Config::ModelPath(const std::string& asset_dir, const std::string& name) {
  return asset_dir + "/" + name + "/" + kModelFile;
}