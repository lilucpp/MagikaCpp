#include "magikacpp.h"
#include "filefeatures.h"
#include "config.h"
#include "seekable.h"
#include <onnxruntime_cxx_api.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <memory>

namespace impl {
#ifdef _WIN32
  std::wstring utf8_to_wstring(const std::string& str)
  {
      int size_needed = MultiByteToWideChar(CP_UTF8, 0,
                                            str.c_str(), (int)str.size(),
                                            nullptr, 0);
      std::wstring wstr(size_needed, 0);
      MultiByteToWideChar(CP_UTF8, 0,
                          str.c_str(), (int)str.size(),
                          &wstr[0], size_needed);
      return wstr;
  }

  std::wstring ort_model_path = utf8_to_wstring(model_path);
#endif
}

class MagikaImpl {
 private:
  Ort::Env env;
  Ort::Session session;
  std::vector<std::string> target_labels;
  Config config;
  
 public:
  MagikaImpl(const std::string& model_path, const Config& cfg);
  
  void InitTargetLabels();
  
  std::pair<std::string, float> ScanFile(const std::string& filepath);
  
  std::vector<float> RunInference(const std::vector<int32_t>& features);
};

MagikaImpl::MagikaImpl(const std::string& model_path, const Config& cfg) : 
    env(ORT_LOGGING_LEVEL_WARNING, "MagikaCPP"),
    session(nullptr),
    config(cfg) {
  
  // Initialize target label space
  InitTargetLabels();
  
  // Create session options
  Ort::SessionOptions session_options;
  session_options.SetIntraOpNumThreads(1);
  session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  
  // Create session
#ifdef _WIN32
  std::wstring ort_model_path = impl::utf8_to_wstring(model_path);
#else
  std::string ort_model_path = model_path;
#endif
  session = Ort::Session(env, ort_model_path.c_str(), session_options);
}

void MagikaImpl::InitTargetLabels() {
  // Use the label list from the configuration file
  target_labels = config.target_labels_space;
}

std::pair<std::string, float> MagikaImpl::ScanFile(const std::string& filepath) {
  // Use Seekable to read file on demand
  Seekable seekable(filepath);
  
  // Special handling for empty files
  if (seekable.size() == 0) {
    return std::make_pair("empty", 1.0f);
  }
  
  // Extract features
  Features features = ExtractFeaturesFromSeekable(seekable, config);
  std::vector<int32_t> flattened_features = features.Flatten();
  
  // Run inference
  std::vector<float> result = RunInference(flattened_features);
  
  // Find the best match
  size_t best_index = 0;
  for (size_t i = 1; i < result.size(); ++i) {
    if (result[i] > result[best_index]) {
      best_index = i;
    }
  }
  
  if (best_index < target_labels.size()) {
    return std::make_pair(target_labels[best_index], result[best_index]);
  } else {
    return std::make_pair("unknown", result[best_index]);
  }
}

std::vector<float> MagikaImpl::RunInference(const std::vector<int32_t>& features) {
  // Define input and output names
  const char* input_names[] = { "bytes" };
  const char* output_names[] = { "target_label" };
  
  // Create input tensor
  std::vector<int64_t> input_shape = { 1, static_cast<int64_t>(features.size()) };
  auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
  Ort::Value input_tensor = Ort::Value::CreateTensor<int32_t>(
      memory_info, 
      const_cast<int32_t*>(features.data()), 
      features.size(), 
      input_shape.data(), 
      input_shape.size()
  );
  try {
    // Run inference
    std::vector<Ort::Value> output_tensors = session.Run(
        Ort::RunOptions{nullptr}, 
        input_names, 
        &input_tensor, 
        1, 
        output_names, 
        1
    );
    // Get output
    std::vector<float> result;
    if (output_tensors.size() > 0) {
      auto info = output_tensors.front().GetTensorTypeAndShapeInfo();
      size_t output_count = info.GetElementCount();

      float* output_data = output_tensors.front().GetTensorMutableData<float>();
      result.assign(output_data, output_data + output_count);
    }

    return result;
  } catch(const Ort::Exception& e) {
    std::cout << "run exception:" << e.GetOrtErrorCode() << ", msg:" << e.what() << std::endl;
    return {};
  }
}

// Global instance pointer
static std::unique_ptr<MagikaImpl> g_magika_impl = nullptr;

void MagikaScanner::initialize(const std::string& model_path) {
  // Infer asset directory and model name from model path
  std::string assets_dir = "."; // Default to current directory
  std::string model_name = "standard_v3_3"; // Default model name
  
  // Try to extract asset directory from model path
  // Support path formats: ./models/model_name/model.onnx or ./models\model_name\model.onnx
  size_t last_slash = model_path.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    size_t second_last_slash = model_path.find_last_of("/\\", last_slash - 1);
    if (second_last_slash != std::string::npos) {
      assets_dir = model_path.substr(0, second_last_slash);
      model_name = model_path.substr(second_last_slash + 1, last_slash - second_last_slash - 1);
    }
  }
  
  // Read configuration
  Config cfg = Config::ReadConfig(assets_dir, model_name);
  
  // Initialize MagikaImpl
  g_magika_impl = std::make_unique<MagikaImpl>(model_path, cfg);
}

std::string MagikaScanner::scanFile(const std::string& filepath) {
  if (!g_magika_impl) {
    throw MagikaException("MagikaScanner not initialized. Call initialize() first.");
  }
  
  std::pair<std::string, float> result = g_magika_impl->ScanFile(filepath);
  return result.first;
}

std::pair<std::string, float> MagikaScanner::scanFileWithScore(const std::string& filepath) {
  if (!g_magika_impl) {
    throw MagikaException("MagikaScanner not initialized. Call initialize() first.");
  }
  
  return g_magika_impl->ScanFile(filepath);
}