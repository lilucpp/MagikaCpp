#ifndef MAGIKACPP_SEEKABLE_H_
#define MAGIKACPP_SEEKABLE_H_

#include <string>
#include <vector>
#include <fstream>
#include <memory>

class Seekable {
 private:
  std::unique_ptr<std::ifstream> file_stream;
  size_t file_size;

 public:
  /**
   * Constructor that creates a Seekable object from a file path
   * @param filepath Path to the file
   */
  explicit Seekable(const std::string& filepath);
  
  /**
   * Destructor
   */
  ~Seekable();
  
  /**
   * Get the file size
   * @return File size in bytes
   */
  size_t size() const;
  
  /**
   * Read data of specified size from a given offset
   * @param offset Offset to start reading from
   * @param size Number of bytes to read
   * @return Read data
   */
  std::vector<uint8_t> read_at(size_t offset, size_t size);
};

#endif  // MAGIKACPP_SEEKABLE_H_