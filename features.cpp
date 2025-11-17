#include "features.h"
#include "Seekable.h"
#include <algorithm>
#include <cstring>

std::vector<int32_t> Features::Flatten() const {
    // 基本大小为Beg + End特征
    size_t reserved_size = Beg.size() + End.size();
    
    // 只有当Mid特征非空时才包含它
    if (!Mid.empty()) {
        reserved_size += Mid.size();
    }
    
    // 只有当Offset特征非空时才包含它们
    if (!Offset8000.empty()) {
        reserved_size += Offset8000.size() + Offset8800.size() + 
                        Offset9000.size() + Offset9800.size();
    }
    
    std::vector<int32_t> result;
    result.reserve(reserved_size);
    
    result.insert(result.end(), Beg.begin(), Beg.end());
    
    // 只有当Mid特征非空时才包含它
    if (!Mid.empty()) {
        result.insert(result.end(), Mid.begin(), Mid.end());
    }
    
    result.insert(result.end(), End.begin(), End.end());
    
    // 只有当Offset特征非空时才包含它们
    if (!Offset8000.empty()) {
        result.insert(result.end(), Offset8000.begin(), Offset8000.end());
        result.insert(result.end(), Offset8800.begin(), Offset8800.end());
        result.insert(result.end(), Offset9000.begin(), Offset9000.end());
        result.insert(result.end(), Offset9800.begin(), Offset9800.end());
    }
    
    return result;
}

static std::vector<int32_t> padInt32(const std::vector<uint8_t>& bytes, size_t prefix, size_t size, int paddingToken) {
    std::vector<int32_t> result;
    result.reserve(size);
    
    // Add prefix padding
    for (size_t i = 0; i < prefix; i++) {
        result.push_back(paddingToken);
    }
    
    // Add actual bytes (up to size - prefix)
    size_t bytes_to_add = std::min(bytes.size(), size - prefix);
    for (size_t i = 0; i < bytes_to_add; i++) {
        result.push_back(static_cast<int32_t>(bytes[i]));
    }
    
    // Add suffix padding
    size_t suffix_padding = size - prefix - bytes_to_add;
    for (size_t i = 0; i < suffix_padding; i++) {
        result.push_back(paddingToken);
    }
    
    return result;
}

static std::vector<uint8_t> safeSlice(const std::vector<uint8_t>& vec, size_t start, size_t length) {
    if (start >= vec.size()) {
        return std::vector<uint8_t>(length, 0);
    }
    
    size_t actual_length = std::min(length, vec.size() - start);
    std::vector<uint8_t> result(vec.begin() + start, vec.begin() + start + actual_length);
    
    // Pad with zeros if needed
    result.resize(length, 0);
    
    return result;
}

static std::vector<int32_t> extractOffsetFeatures(const std::vector<uint8_t>& content, size_t offset, const Config& cfg) {
    if (content.size() <= offset) {
        // 文件太小，返回填充数据
        return std::vector<int32_t>(8, cfg.PaddingToken);
    }
    
    // 从指定偏移量读取8个字节
    std::vector<uint8_t> bytes = safeSlice(content, offset, 8);
    
    // 转换为int32_t并填充到固定大小（8个元素）
    std::vector<int32_t> result;
    result.reserve(8);
    
    for (const auto& byte : bytes) {
        result.push_back(static_cast<int32_t>(byte));
    }
    
    return result;
}

// 新增：从Seekable对象提取Offset特征
static std::vector<int32_t> extractOffsetFeaturesFromSeekable(Seekable& seekable, size_t offset, const Config& cfg) {
    if (seekable.size() <= offset) {
        // 文件太小，返回填充数据
        return std::vector<int32_t>(8, cfg.PaddingToken);
    }
    
    // 从指定偏移量读取8个字节
    auto bytes = seekable.read_at(offset, 8);
    
    // 转换为int32_t并填充到固定大小（8个元素）
    std::vector<int32_t> result;
    result.reserve(8);
    
    for (const auto& byte : bytes) {
        result.push_back(static_cast<int32_t>(byte));
    }
    
    // 如果读取的字节不足8个，则用paddingToken填充
    while (result.size() < 8) {
        result.push_back(cfg.PaddingToken);
    }
    
    return result;
}

Features ExtractFeatures(const std::vector<uint8_t>& content, const Config& cfg) {
    size_t content_size = content.size();
    
    // Read beginning, middle and end blocks
    size_t beg_size = std::min(content_size, static_cast<size_t>(cfg.BlockSize));
    std::vector<uint8_t> beg_bytes(content.begin(), content.begin() + beg_size);
    
    std::vector<uint8_t> mid_bytes;
    if (content_size <= static_cast<size_t>(cfg.MidSize)) {
        // If content is smaller than or equal to MID_SIZE, use all content
        mid_bytes = content;
    } else {
        // Take middle part
        size_t mid_start = (content_size - cfg.MidSize) / 2;
        size_t mid_end = std::min(mid_start + cfg.MidSize, content_size);
        mid_bytes = std::vector<uint8_t>(content.begin() + mid_start, content.begin() + mid_end);
    }
    
    size_t block_size = std::min(content_size, static_cast<size_t>(cfg.BlockSize));
    size_t end_start = content_size - block_size;
    std::vector<uint8_t> end_bytes(content.begin() + end_start, content.end());
    
    // Build features
    std::vector<uint8_t> first_block = beg_bytes;
    
    // Trim beg and end, and truncate to BEG_SIZE and END_SIZE
    const std::string spaces = "\t\n\v\f\r ";
    
    // Remove leading whitespace characters
    size_t beg_start_pos = 0;
    while (beg_start_pos < beg_bytes.size() && 
           spaces.find(beg_bytes[beg_start_pos]) != std::string::npos) {
        beg_start_pos++;
    }
    
    // Extract valid part
    if (beg_start_pos < beg_bytes.size()) {
        beg_bytes = std::vector<uint8_t>(beg_bytes.begin() + beg_start_pos, beg_bytes.end());
    } else {
        beg_bytes.clear();
    }
    
    // Limit beg size
    if (beg_bytes.size() > static_cast<size_t>(cfg.BegSize)) {
        beg_bytes.resize(cfg.BegSize);
    }
    
    // Remove trailing whitespace characters
    while (!end_bytes.empty()) {
        char last_char = static_cast<char>(end_bytes.back());
        if (spaces.find(last_char) == std::string::npos) {
            break;
        }
        end_bytes.pop_back();
    }
    
    // Limit end size, take from the end
    if (end_bytes.size() > static_cast<size_t>(cfg.EndSize)) {
        std::vector<uint8_t> temp(end_bytes.end() - cfg.EndSize, end_bytes.end());
        end_bytes = temp;
    }
    
    // Create Features struct
    Features features;
    features.firstBlock = first_block;
    
    // Build beg features
    features.Beg = padInt32(beg_bytes, 0, cfg.BegSize, cfg.PaddingToken);
    
    // Build mid features
    if (cfg.MidSize > 0) {
        features.Mid = padInt32(mid_bytes, (cfg.MidSize - mid_bytes.size()) / 2, cfg.MidSize, cfg.PaddingToken);
    }
    // 当MidSize为0时不创建Mid特征向量，保持features.Mid为空
    
    // Build end features
    features.End = padInt32(end_bytes, cfg.EndSize - end_bytes.size(), cfg.EndSize, cfg.PaddingToken);
    
    // Extract Offset features only if use_inputs_at_offsets is true
    if (cfg.UseInputsAtOffsets) {
        features.Offset8000 = extractOffsetFeatures(content, 0x8000, cfg);
        features.Offset8800 = extractOffsetFeatures(content, 0x8800, cfg);
        features.Offset9000 = extractOffsetFeatures(content, 0x9000, cfg);
        features.Offset9800 = extractOffsetFeatures(content, 0x9800, cfg);
    }
    
    return features;
}

Features ExtractFeaturesFromSeekable(Seekable& seekable, const Config& cfg) {
    size_t content_size = seekable.size();
    
    // 读取开始、中间和结束块（按需读取）
    size_t beg_size = std::min(content_size, static_cast<size_t>(cfg.BlockSize));
    auto beg_bytes = seekable.read_at(0, beg_size);
    
    std::vector<uint8_t> mid_bytes;
    if (content_size <= static_cast<size_t>(cfg.MidSize)) {
        // 如果内容小于或等于MID_SIZE，使用所有内容
        mid_bytes = seekable.read_at(0, content_size);
    } else {
        // 取中间部分
        size_t mid_start = (content_size - cfg.MidSize) / 2;
        size_t mid_end = std::min(mid_start + cfg.MidSize, content_size);
        mid_bytes = seekable.read_at(mid_start, mid_end - mid_start);
    }
    
    size_t block_size = std::min(content_size, static_cast<size_t>(cfg.BlockSize));
    size_t end_start = content_size - block_size;
    auto end_bytes = seekable.read_at(end_start, block_size);
    
    // 构建特征
    std::vector<uint8_t> first_block = beg_bytes;
    
    // 去除前导和尾随空白字符
    const std::string spaces = "\t\n\v\f\r ";
    
    // 去除前导空白字符
    size_t beg_start_pos = 0;
    while (beg_start_pos < beg_bytes.size() && 
           spaces.find(beg_bytes[beg_start_pos]) != std::string::npos) {
        beg_start_pos++;
    }
    
    // 提取有效部分
    if (beg_start_pos < beg_bytes.size()) {
        beg_bytes = std::vector<uint8_t>(beg_bytes.begin() + beg_start_pos, beg_bytes.end());
    } else {
        beg_bytes.clear();
    }
    
    // 限制beg大小
    if (beg_bytes.size() > static_cast<size_t>(cfg.BegSize)) {
        beg_bytes.resize(cfg.BegSize);
    }
    
    // 去除尾随空白字符
    while (!end_bytes.empty()) {
        char last_char = static_cast<char>(end_bytes.back());
        if (spaces.find(last_char) == std::string::npos) {
            break;
        }
        end_bytes.pop_back();
    }
    
    // 限制end大小，从末尾取
    if (end_bytes.size() > static_cast<size_t>(cfg.EndSize)) {
        std::vector<uint8_t> temp(end_bytes.end() - cfg.EndSize, end_bytes.end());
        end_bytes = temp;
    }
    
    // 创建Features结构
    Features features;
    features.firstBlock = first_block;
    
    // 构建beg特征
    features.Beg = padInt32(beg_bytes, 0, cfg.BegSize, cfg.PaddingToken);
    
    // 构建mid特征
    if (cfg.MidSize > 0) {
        features.Mid = padInt32(mid_bytes, (cfg.MidSize - mid_bytes.size()) / 2, cfg.MidSize, cfg.PaddingToken);
    }
    // 当MidSize为0时不创建Mid特征向量，保持features.Mid为空
    
    // 构建end特征
    features.End = padInt32(end_bytes, cfg.EndSize - end_bytes.size(), cfg.EndSize, cfg.PaddingToken);
    
    // 根据use_inputs_at_offsets决定是否提取Offset特征
    if (cfg.UseInputsAtOffsets) {
        features.Offset8000 = extractOffsetFeaturesFromSeekable(seekable, 0x8000, cfg);
        features.Offset8800 = extractOffsetFeaturesFromSeekable(seekable, 0x8800, cfg);
        features.Offset9000 = extractOffsetFeaturesFromSeekable(seekable, 0x9000, cfg);
        features.Offset9800 = extractOffsetFeaturesFromSeekable(seekable, 0x9800, cfg);
    }
    
    return features;
}