#include "filefeatures.h"
#include "seekable.h"
#include <algorithm>
#include <cstring>

std::vector<int32_t> Features::Flatten() const {
  // Basic size is Beg + End features
  size_t reserved_size = beg.size() + end.size();
  
  // Only include Mid features if not empty
  if (!mid.empty()) {
    reserved_size += mid.size();
  }
  
  // Only include Offset features if not empty
  if (!offset_8000.empty()) {
    reserved_size += offset_8000.size() + offset_8800.size() + 
                    offset_9000.size() + offset_9800.size();
  }
  
  std::vector<int32_t> result;
  result.reserve(reserved_size);
  
  result.insert(result.end(), beg.begin(), beg.end());
  
  // Only include Mid features if not empty
  if (!mid.empty()) {
    result.insert(result.end(), mid.begin(), mid.end());
  }
  
  result.insert(result.end(), end.begin(), end.end());
  
  // Only include Offset features if not empty
  if (!offset_8000.empty()) {
    result.insert(result.end(), offset_8000.begin(), offset_8000.end());
    result.insert(result.end(), offset_8800.begin(), offset_8800.end());
    result.insert(result.end(), offset_9000.begin(), offset_9000.end());
    result.insert(result.end(), offset_9800.begin(), offset_9800.end());
  }
  
  return result;
}

static std::vector<int32_t> PadInt32(const std::vector<uint8_t>& bytes, size_t prefix, size_t size, int padding_token) {
  std::vector<int32_t> result;
  result.reserve(size);
  
  // Add prefix padding
  for (size_t i = 0; i < prefix; i++) {
    result.push_back(padding_token);
  }
  
  // Add actual bytes (up to size - prefix)
  size_t bytes_to_add = std::min(bytes.size(), size - prefix);
  for (size_t i = 0; i < bytes_to_add; i++) {
    result.push_back(static_cast<int32_t>(bytes[i]));
  }
  
  // Add suffix padding
  size_t suffix_padding = size - prefix - bytes_to_add;
  for (size_t i = 0; i < suffix_padding; i++) {
    result.push_back(padding_token);
  }
  
  return result;
}

static std::vector<uint8_t> SafeSlice(const std::vector<uint8_t>& vec, size_t start, size_t length) {
  if (start >= vec.size()) {
    return std::vector<uint8_t>(length, 0);
  }
  
  size_t actual_length = std::min(length, vec.size() - start);
  std::vector<uint8_t> result(vec.begin() + start, vec.begin() + start + actual_length);
  
  // Pad with zeros if needed
  result.resize(length, 0);
  
  return result;
}

static std::vector<int32_t> ExtractOffsetFeatures(const std::vector<uint8_t>& content, size_t offset, const Config& cfg) {
  if (content.size() <= offset) {
    // File too small, return padding data
    return std::vector<int32_t>(8, cfg.padding_token);
  }
  
  // Read 8 bytes from the specified offset
  std::vector<uint8_t> bytes = SafeSlice(content, offset, 8);
  
  // Convert to int32_t and pad to fixed size (8 elements)
  std::vector<int32_t> result;
  result.reserve(8);
  
  for (const auto& byte : bytes) {
    result.push_back(static_cast<int32_t>(byte));
  }
  
  return result;
}

// New: Extract Offset features from Seekable object
static std::vector<int32_t> ExtractOffsetFeaturesFromSeekable(Seekable& seekable, size_t offset, const Config& cfg) {
  if (seekable.size() <= offset) {
    // File too small, return padding data
    return std::vector<int32_t>(8, cfg.padding_token);
  }
  
  // Read 8 bytes from the specified offset
  auto bytes = seekable.read_at(offset, 8);
  
  // Convert to int32_t and pad to fixed size (8 elements)
  std::vector<int32_t> result;
  result.reserve(8);
  
  for (const auto& byte : bytes) {
    result.push_back(static_cast<int32_t>(byte));
  }
  
  // If fewer than 8 bytes were read, pad with padding_token
  while (result.size() < 8) {
    result.push_back(cfg.padding_token);
  }
  
  return result;
}

Features ExtractFeatures(const std::vector<uint8_t>& content, const Config& cfg) {
  size_t content_size = content.size();
  
  // Read beginning, middle and end blocks
  size_t beg_size = std::min(content_size, static_cast<size_t>(cfg.block_size));
  std::vector<uint8_t> beg_bytes(content.begin(), content.begin() + beg_size);
  
  std::vector<uint8_t> mid_bytes;
  if (content_size <= static_cast<size_t>(cfg.mid_size)) {
    // If content is smaller than or equal to MID_SIZE, use all content
    mid_bytes = content;
  } else {
    // Take middle part
    size_t mid_start = (content_size - cfg.mid_size) / 2;
    size_t mid_end = std::min(mid_start + cfg.mid_size, content_size);
    mid_bytes = std::vector<uint8_t>(content.begin() + mid_start, content.begin() + mid_end);
  }
  
  size_t block_size = std::min(content_size, static_cast<size_t>(cfg.block_size));
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
  if (beg_bytes.size() > static_cast<size_t>(cfg.beg_size)) {
    beg_bytes.resize(cfg.beg_size);
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
  if (end_bytes.size() > static_cast<size_t>(cfg.end_size)) {
    std::vector<uint8_t> temp(end_bytes.end() - cfg.end_size, end_bytes.end());
    end_bytes = temp;
  }
  
  // Create Features struct
  Features features;
  features.first_block = first_block;
  
  // Build beg features
  features.beg = PadInt32(beg_bytes, 0, cfg.beg_size, cfg.padding_token);
  
  // Build mid features
  if (cfg.mid_size > 0) {
    features.mid = PadInt32(mid_bytes, (cfg.mid_size - mid_bytes.size()) / 2, cfg.mid_size, cfg.padding_token);
  }
  // When MidSize is 0, do not create Mid feature vector, keep features.Mid empty
  
  // Build end features
  features.end = PadInt32(end_bytes, cfg.end_size - end_bytes.size(), cfg.end_size, cfg.padding_token);
  
  // Extract Offset features only if use_inputs_at_offsets is true
  if (cfg.use_inputs_at_offsets) {
    features.offset_8000 = ExtractOffsetFeatures(content, 0x8000, cfg);
    features.offset_8800 = ExtractOffsetFeatures(content, 0x8800, cfg);
    features.offset_9000 = ExtractOffsetFeatures(content, 0x9000, cfg);
    features.offset_9800 = ExtractOffsetFeatures(content, 0x9800, cfg);
  }
  
  return features;
}

Features ExtractFeaturesFromSeekable(Seekable& seekable, const Config& cfg) {
  size_t content_size = seekable.size();
  
  // Read beginning, middle and end blocks (on-demand)
  size_t beg_size = std::min(content_size, static_cast<size_t>(cfg.block_size));
  auto beg_bytes = seekable.read_at(0, beg_size);
  
  std::vector<uint8_t> mid_bytes;
  if (content_size <= static_cast<size_t>(cfg.mid_size)) {
    // If content is smaller than or equal to MID_SIZE, use all content
    mid_bytes = seekable.read_at(0, content_size);
  } else {
    // Take middle part
    size_t mid_start = (content_size - cfg.mid_size) / 2;
    size_t mid_end = std::min(mid_start + cfg.mid_size, content_size);
    mid_bytes = seekable.read_at(mid_start, mid_end - mid_start);
  }
  
  size_t block_size = std::min(content_size, static_cast<size_t>(cfg.block_size));
  size_t end_start = content_size - block_size;
  auto end_bytes = seekable.read_at(end_start, block_size);
  
  // Build features
  std::vector<uint8_t> first_block = beg_bytes;
  
  // Remove leading and trailing whitespace characters
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
  if (beg_bytes.size() > static_cast<size_t>(cfg.beg_size)) {
    beg_bytes.resize(cfg.beg_size);
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
  if (end_bytes.size() > static_cast<size_t>(cfg.end_size)) {
    std::vector<uint8_t> temp(end_bytes.end() - cfg.end_size, end_bytes.end());
    end_bytes = temp;
  }
  
  // Create Features struct
  Features features;
  features.first_block = first_block;
  
  // Build beg features
  features.beg = PadInt32(beg_bytes, 0, cfg.beg_size, cfg.padding_token);
  
  // Build mid features
  if (cfg.mid_size > 0) {
    features.mid = PadInt32(mid_bytes, (cfg.mid_size - mid_bytes.size()) / 2, cfg.mid_size, cfg.padding_token);
  }
  // When MidSize is 0, do not create Mid feature vector, keep features.Mid empty
  
  // Build end features
  features.end = PadInt32(end_bytes, cfg.end_size - end_bytes.size(), cfg.end_size, cfg.padding_token);
  
  // Extract Offset features based on use_inputs_at_offsets
  if (cfg.use_inputs_at_offsets) {
    features.offset_8000 = ExtractOffsetFeaturesFromSeekable(seekable, 0x8000, cfg);
    features.offset_8800 = ExtractOffsetFeaturesFromSeekable(seekable, 0x8800, cfg);
    features.offset_9000 = ExtractOffsetFeaturesFromSeekable(seekable, 0x9000, cfg);
    features.offset_9800 = ExtractOffsetFeaturesFromSeekable(seekable, 0x9800, cfg);
  }
  
  return features;
}