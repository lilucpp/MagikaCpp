#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

Config::Config() : 
    BegSize(0),
    MidSize(0),
    EndSize(0),
    UseInputsAtOffsets(false),
    MediumConfidenceThreshold(0.0f),
    MinFileSizeForDl(0),
    PaddingToken(0),
    BlockSize(0) {
}

Config Config::ReadConfig(const std::string& assetsDir, const std::string& name) {
    Config cfg;
    std::string path = ConfigPath(assetsDir, name);
    
    // Check if file exists
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Config file not found: " + path);
    }
    
    // Read the JSON file
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path);
    }
    
    json jsonData;
    try {
        file >> jsonData;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse JSON in config file: " + path + " - " + e.what());
    }
    
    // Parse the configuration values
    cfg.BegSize = jsonData.value("beg_size", 0);
    cfg.MidSize = jsonData.value("mid_size", 0);
    cfg.EndSize = jsonData.value("end_size", 0);
    cfg.UseInputsAtOffsets = jsonData.value("use_inputs_at_offsets", false);
    cfg.MediumConfidenceThreshold = jsonData.value("medium_confidence_threshold", 0.0f);
    cfg.MinFileSizeForDl = jsonData.value("min_file_size_for_dl", 0LL);
    cfg.PaddingToken = jsonData.value("padding_token", 0);
    cfg.BlockSize = jsonData.value("block_size", 0);
    
    // Parse TargetLabelsSpace
    if (jsonData.contains("target_labels_space") && jsonData["target_labels_space"].is_array()) {
        for (const auto& label : jsonData["target_labels_space"]) {
            cfg.TargetLabelsSpace.push_back(label.get<std::string>());
        }
    }
    
    // Parse Thresholds
    if (jsonData.contains("thresholds") && jsonData["thresholds"].is_object()) {
        for (auto& [key, value] : jsonData["thresholds"].items()) {
            cfg.Thresholds[key] = value.get<float>();
        }
    }
    
    // Parse Overwrite
    if (jsonData.contains("overwrite_map") && jsonData["overwrite_map"].is_object()) {
        for (auto& [key, value] : jsonData["overwrite_map"].items()) {
            cfg.Overwrite[key] = value.get<std::string>();
        }
    }
    
    return cfg;
}

std::string Config::ConfigPath(const std::string& assetDir, const std::string& name) {
    return assetDir + "/" + name + "/" + CONFIG_FILE;
}

std::string Config::ModelPath(const std::string& assetDir, const std::string& name) {
    return assetDir + "/" + name + "/" + MODEL_FILE;
}