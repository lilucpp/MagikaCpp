#ifndef FEATURES_H
#define FEATURES_H

#include <vector>
#include <cstdint>
#include <string>
#include "config.h"

// Forward declaration
class Seekable;

/**
 * Features holds the features of a given slice of bytes.
 */
struct Features {
    std::vector<uint8_t> firstBlock;
    std::vector<int32_t> Beg;
    std::vector<int32_t> Mid;
    std::vector<int32_t> End;
    std::vector<int32_t> Offset8000;
    std::vector<int32_t> Offset8800;
    std::vector<int32_t> Offset9000;
    std::vector<int32_t> Offset9800;
    
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
const int BEG_SIZE = 1024;
const int MID_SIZE = 0;
const int END_SIZE = 1024;
const int PADDING_TOKEN = 256;
const int BLOCK_SIZE = 4096;
const int OFFSET_8000 = 0x8000;
const int OFFSET_8800 = 0x8800;
const int OFFSET_9000 = 0x9000;
const int OFFSET_9800 = 0x9800;
const int OFFSET_SIZE = 8;

#endif // FEATURES_H