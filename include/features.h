#ifndef MAGIKACPP_FEATURES_H_
#define MAGIKACPP_FEATURES_H_

#include <vector>
#include <cstdint>
#include <string>
#include "config.h"
#include "seekable.h"

// Forward declaration
class Seekable;

/**
 * Features holds the features of a given slice of bytes.
 */
struct Features {
  std::vector<uint8_t> first_block;
  std::vector<int32_t> beg;
  std::vector<int32_t> mid;
  std::vector<int32_t> end;
  std::vector<int32_t> offset_8000;
  std::vector<int32_t> offset_8800;
  std::vector<int32_t> offset_9000;
  std::vector<int32_t> offset_9800;
  
  /**
   * Flatten returns a flattened array of the given features.
   * @return Flattened feature array
   */
  std::vector<int32_t> Flatten() const;
};

/**
 * ExtractFeatures extract the features from the given content.
 * @param content File content as byte vector
 * @param cfg Configuration parameters
 * @return Extracted features
 */
Features ExtractFeatures(const std::vector<uint8_t>& content, const Config& cfg);

/**
 * ExtractFeaturesFromSeekable extract the features from a seekable object.
 * @param seekable Seekable object to extract features from
 * @param cfg Configuration parameters
 * @return Extracted features
 */
Features ExtractFeaturesFromSeekable(Seekable& seekable, const Config& cfg);

// Configuration constants
const int k_beg_size = 1024;
const int k_mid_size = 0;
const int k_end_size = 1024;
const int k_padding_token = 256;
const int k_block_size = 4096;
const int k_offset_8000 = 0x8000;
const int k_offset_8800 = 0x8800;
const int k_offset_9000 = 0x9000;
const int k_offset_9800 = 0x9800;
const int k_offset_size = 8;

#endif  // MAGIKACPP_FEATURES_H_