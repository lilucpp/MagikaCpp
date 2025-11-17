#include "magika_cpp_wrapper.h"
#include "features.h"
#include "config.h"
#include "Seekable.h"
#include <onnxruntime_cxx_api.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <memory>

class MagikaImpl {
private:
    Ort::Env env;
    Ort::Session session;
    std::vector<std::string> target_labels;
    Config config;
    
public:
    MagikaImpl(const std::string& model_path, const Config& cfg);
    
    void init_target_labels();
    
    std::pair<std::string, float> scan_file(const std::string& filepath);
    
    std::vector<float> run_inference(const std::vector<int32_t>& features);
};

MagikaImpl::MagikaImpl(const std::string& model_path, const Config& cfg) : 
    env(ORT_LOGGING_LEVEL_WARNING, "MagikaCPP"),
    session(nullptr),
    config(cfg) {
    
    // 初始化目标标签空间
    init_target_labels();
    
    // 创建会话选项
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    
    // 创建会话
    std::wstring wmodel_path(model_path.begin(), model_path.end());
    session = Ort::Session(env, wmodel_path.c_str(), session_options);
}

void MagikaImpl::init_target_labels() {
    // 使用配置文件中的标签列表
    target_labels = config.TargetLabelsSpace;
}

std::pair<std::string, float> MagikaImpl::scan_file(const std::string& filepath) {
    // 使用Seekable按需读取文件
    Seekable seekable(filepath);
    
    // 对于空文件的特殊处理
    if (seekable.size() == 0) {
        return std::make_pair("empty", 1.0f);
    }
    
    // 提取特征
    Features features = ExtractFeaturesFromSeekable(seekable, config);
    std::vector<int32_t> flattened_features = features.Flatten();
    
    // 运行推理
    std::vector<float> result = run_inference(flattened_features);
    
    // 找到最佳匹配
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

std::vector<float> MagikaImpl::run_inference(const std::vector<int32_t>& features) {
    // 定义输入输出名称
    const char* input_names[] = { "bytes" };
    const char* output_names[] = { "target_label" };
    
    // 创建输入张量
    std::vector<int64_t> input_shape = { 1, static_cast<int64_t>(features.size()) };
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Value input_tensor = Ort::Value::CreateTensor<int32_t>(
        memory_info, 
        const_cast<int32_t*>(features.data()), 
        features.size(), 
        input_shape.data(), 
        input_shape.size()
    );
    try{
        // 运行推理
        std::vector<Ort::Value> output_tensors = session.Run(
            Ort::RunOptions{nullptr}, 
            input_names, 
            &input_tensor, 
            1, 
            output_names, 
            1
        );
        // 获取输出
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

// 全局实例指针
static std::unique_ptr<MagikaImpl> g_magika_impl = nullptr;

void MagikaScanner::initialize(const std::string& model_path) {
    // 从模型路径推断资产目录和模型名称
    std::string assetsDir = "."; // 默认为当前目录
    std::string modelName = "standard_v3_3"; // 默认模型名称
    
    // 尝试从模型路径提取资产目录
    // 支持路径格式: ./models/model_name/model.onnx 或 ./models\model_name\model.onnx
    size_t last_slash = model_path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        size_t second_last_slash = model_path.find_last_of("/\\", last_slash - 1);
        if (second_last_slash != std::string::npos) {
            assetsDir = model_path.substr(0, second_last_slash);
            modelName = model_path.substr(second_last_slash + 1, last_slash - second_last_slash - 1);
        }
    }
    
    // 读取配置
    Config cfg = Config::ReadConfig(assetsDir, modelName);
    
    // 初始化MagikaImpl
    g_magika_impl = std::make_unique<MagikaImpl>(model_path, cfg);
}

std::string MagikaScanner::scanFile(const std::string& filepath) {
    if (!g_magika_impl) {
        throw MagikaException("MagikaScanner not initialized. Call initialize() first.");
    }
    
    std::pair<std::string, float> result = g_magika_impl->scan_file(filepath);
    return result.first;
}

std::pair<std::string, float> MagikaScanner::scanFileWithScore(const std::string& filepath) {
    if (!g_magika_impl) {
        throw MagikaException("MagikaScanner not initialized. Call initialize() first.");
    }
    
    return g_magika_impl->scan_file(filepath);
}