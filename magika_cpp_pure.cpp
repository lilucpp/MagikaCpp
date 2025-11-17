#include "magika_cpp_wrapper.h"
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
    
    // 默认配置参数，匹配standard_v3_3模型
    static const int BEG_SIZE = 1024;
    static const int MID_SIZE = 0;
    static const int END_SIZE = 1024;
    static const int PADDING_TOKEN = 256;
    static const int BLOCK_SIZE = 4096;
    
public:
    MagikaImpl(const std::string& model_path);
    
    void init_target_labels();
    
    std::vector<int32_t> extract_features(const std::vector<uint8_t>& content);
    
    std::vector<int32_t> extract_beg_features(const std::vector<uint8_t>& content);
    
    std::vector<int32_t> extract_mid_features(const std::vector<uint8_t>& content);
    
    std::vector<int32_t> extract_end_features(const std::vector<uint8_t>& content);
    
    std::vector<int32_t> pad_int32(const std::vector<uint8_t>& bytes, size_t prefix_padding, size_t total_size);
    
    std::pair<std::string, float> scan_file(const std::string& filepath);
    
    std::vector<float> run_inference(const std::vector<int32_t>& features);
};

MagikaImpl::MagikaImpl(const std::string& model_path) : 
    env(ORT_LOGGING_LEVEL_WARNING, "MagikaCPP"),
    session(nullptr) {
    
    // 初始化目标标签空间（简化版，实际应该从配置文件读取）
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
    // 从配置文件加载完整的标签列表
    target_labels = {
        "3gp", "ace", "ai", "aidl", "apk", "applebplist", "appleplist", "asm", 
        "asp", "autohotkey", "autoit", "awk", "batch", "bazel", "bib", "bmp", 
        "bzip", "c", "cab", "cat", "chm", "clojure", "cmake", "cobol", "coff", 
        "coffeescript", "cpp", "crt", "crx", "cs", "csproj", "css", "csv", 
        "dart", "deb", "dex", "dicom", "diff", "dm", "dmg", "doc", "dockerfile", 
        "docx", "dsstore", "dwg", "dxf", "elf", "elixir", "emf", "eml", "epub", 
        "erb", "erlang", "flac", "flv", "fortran", "gemfile", "gemspec", "gif", 
        "gitattributes", "gitmodules", "go", "gradle", "groovy", "gzip", "h5", 
        "handlebars", "haskell", "hcl", "hlp", "htaccess", "html", "icns", "ico", 
        "ics", "ignorefile", "ini", "internetshortcut", "ipynb", "iso", "jar", 
        "java", "javabytecode", "javascript", "jinja", "jp2", "jpeg", "json", 
        "jsonl", "julia", "kotlin", "latex", "lha", "lisp", "lnk", "lua", "m3u", 
        "m4", "macho", "makefile", "markdown", "matlab", "mht", "midi", "mkv", 
        "mp3", "mp4", "mscompress", "msi", "mum", "npy", "npz", "nupkg", 
        "objectivec", "ocaml", "odp", "ods", "odt", "ogg", "one", "onnx", "otf", 
        "outlook", "parquet", "pascal", "pcap", "pdb", "pdf", "pebin", "pem", 
        "perl", "php", "pickle", "png", "po", "postscript", "powershell", "ppt", 
        "pptx", "prolog", "proteindb", "proto", "psd", "python", "pythonbytecode", 
        "pytorch", "qt", "r", "randombytes", "randomtxt", "rar", "rdf", "rpm", 
        "rst", "rtf", "ruby", "rust", "scala", "scss", "sevenzip", "sgml", 
        "shell", "smali", "snap", "solidity", "sql", "sqlite", "squashfs", "srt", 
        "stlbinary", "stltext", "sum", "svg", "swf", "swift", "tar", "tcl", 
        "textproto", "tga", "thumbsdb", "tiff", "toml", "torrent", "tsv", "ttf", 
        "twig", "txt", "typescript", "vba", "vcxproj", "verilog", "vhdl", "vtt", 
        "vue", "wasm", "wav", "webm", "webp", "winregistry", "wmf", "woff", 
        "woff2", "xar", "xls", "xlsb", "xlsx", "xml", "xpi", "xz", "yaml", 
        "yara", "zig", "zip", "zlibstream"
    };
}

std::vector<int32_t> MagikaImpl::extract_features(const std::vector<uint8_t>& content) {
    std::vector<int32_t> features;
    features.reserve(BEG_SIZE + MID_SIZE + END_SIZE);
    
    std::vector<int32_t> beg_features = extract_beg_features(content);
    features.insert(features.end(), beg_features.begin(), beg_features.end());

    if (MID_SIZE > 0) {
        std::vector<int32_t> mid_features = extract_mid_features(content);
        features.insert(features.end(), mid_features.begin(), mid_features.end());
    }

    std::vector<int32_t> end_features = extract_end_features(content);
    features.insert(features.end(), end_features.begin(), end_features.end());
    
    return features;
}

std::vector<int32_t> MagikaImpl::extract_beg_features(const std::vector<uint8_t>& content) {
    size_t beg_size = (std::min)(content.size(), static_cast<size_t>(BLOCK_SIZE));
    std::vector<uint8_t> beg_bytes(content.begin(), content.begin() + beg_size);
    
    // 移除开头的空白字符
    size_t start_pos = 0;
    while (start_pos < beg_bytes.size() && 
           (beg_bytes[start_pos] == '\t' || beg_bytes[start_pos] == '\n' ||
            beg_bytes[start_pos] == '\v' || beg_bytes[start_pos] == '\f' ||
            beg_bytes[start_pos] == '\r' || beg_bytes[start_pos] == ' ')) {
        start_pos++;
    }
    
    // 截取有效部分
    if (start_pos < beg_bytes.size()) {
        std::vector<uint8_t> temp_bytes(beg_bytes.begin() + start_pos, beg_bytes.end());
        beg_bytes = temp_bytes;
    } else {
        beg_bytes.clear();
    }
    
    // 限制大小
    if (beg_bytes.size() > BEG_SIZE) {
        beg_bytes.resize(BEG_SIZE);
    }
    
    // 转换为int32并填充
    return pad_int32(beg_bytes, 0, BEG_SIZE);
}

std::vector<int32_t> MagikaImpl::extract_mid_features(const std::vector<uint8_t>& content) {
    size_t content_size = content.size();
    if (content_size <= MID_SIZE) {
        // 如果内容小于MID_SIZE，直接使用全部内容
        std::vector<uint8_t> mid_bytes = content;
        return pad_int32(mid_bytes, (MID_SIZE - mid_bytes.size()) / 2, MID_SIZE);
    }
    
    // 取中间部分
    size_t mid_start = (content_size - MID_SIZE) / 2;
    size_t mid_end = (std::min)(mid_start + MID_SIZE, content_size);
    std::vector<uint8_t> mid_bytes(content.begin() + mid_start, content.begin() + mid_end);
    
    return pad_int32(mid_bytes, 0, MID_SIZE);
}

std::vector<int32_t> MagikaImpl::extract_end_features(const std::vector<uint8_t>& content) {
    size_t content_size = content.size();
    size_t block_size = (std::min)(content_size, static_cast<size_t>(BLOCK_SIZE));
    
    // 获取末尾BLOCK_SIZE字节
    size_t end_start = content_size - block_size;
    std::vector<uint8_t> end_bytes(content.begin() + end_start, content.end());
    
    // 移除末尾的空白字符
    while (!end_bytes.empty() &&
           (end_bytes.back() == '\t' || end_bytes.back() == '\n' ||
            end_bytes.back() == '\v' || end_bytes.back() == '\f' ||
            end_bytes.back() == '\r' || end_bytes.back() == ' ')) {
        end_bytes.pop_back();
    }
    
    // 限制大小
    if (end_bytes.size() > END_SIZE) {
        // 如果仍然超过END_SIZE，从前面截取
        std::vector<uint8_t> temp_bytes(end_bytes.end() - END_SIZE, end_bytes.end());
        end_bytes = temp_bytes;
    }
    
    // 填充到END_SIZE
    return pad_int32(end_bytes, END_SIZE - end_bytes.size(), END_SIZE);
}

std::vector<int32_t> MagikaImpl::pad_int32(const std::vector<uint8_t>& bytes, size_t prefix_padding, size_t total_size) {
    std::vector<int32_t> result;
    result.reserve(total_size);
    
    // 添加前缀填充
    for (size_t i = 0; i < prefix_padding; ++i) {
        result.push_back(PADDING_TOKEN);
    }
    
    // 添加实际数据
    for (uint8_t byte : bytes) {
        result.push_back(static_cast<int32_t>(byte));
    }
    
    // 添加后缀填充
    while (result.size() < total_size) {
        result.push_back(PADDING_TOKEN);
    }
    
    // 确保不会超过总大小（理论上不应该发生）
    if (result.size() > total_size) {
        result.resize(total_size);
    }
    
    return result;
}

std::pair<std::string, float> MagikaImpl::scan_file(const std::string& filepath) {
    // 读取文件内容
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw MagikaException("Cannot open file: " + filepath);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> content(size);
    if (!file.read(reinterpret_cast<char*>(content.data()), size)) {
        throw MagikaException("Failed to read file: " + filepath);
    }
    
    // 对于空文件的特殊处理
    if (content.empty()) {
        return std::make_pair("empty", 1.0f);
    }
    
    // 提取特征
    std::vector<int32_t> features = extract_features(content);
    
    // 运行推理
    std::vector<float> result = run_inference(features);
    
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
    g_magika_impl = std::make_unique<MagikaImpl>(model_path);
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