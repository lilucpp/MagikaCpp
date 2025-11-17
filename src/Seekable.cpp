#include "Seekable.h"
#include <stdexcept>
#include <algorithm>
#include <iterator>

Seekable::Seekable(const std::string& filepath) : file_size(0) {
  // Open file stream
  file_stream = std::make_unique<std::ifstream>(filepath, std::ios::binary);
  if (!file_stream->is_open()) {
    throw std::runtime_error("Cannot open file: " + filepath);
  }
  
  // Get file size
  file_stream->seekg(0, std::ios::end);
  file_size = static_cast<size_t>(file_stream->tellg());
  file_stream->seekg(0, std::ios::beg);
}

Seekable::~Seekable() {
  if (file_stream && file_stream->is_open()) {
    file_stream->close();
  }
}

size_t Seekable::size() const {
  return file_size;
}

std::vector<uint8_t> Seekable::read_at(size_t offset, size_t size) {
  if (size == 0) {
    return std::vector<uint8_t>();
  }
  
  if (offset + size > file_size) {
    throw std::out_of_range("Read request exceeds file boundaries");
  }
  
  // Position to the specified offset
  file_stream->seekg(static_cast<std::streamoff>(offset));
  
  // Read data
  std::vector<uint8_t> buffer(size);
  file_stream->read(reinterpret_cast<char*>(buffer.data()), size);
  
  return buffer;
}