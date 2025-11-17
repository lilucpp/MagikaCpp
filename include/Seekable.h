#ifndef SEEKABLE_H
#define SEEKABLE_H

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
     * 构造函数，从文件路径创建Seekable对象
     * @param filepath 文件路径
     */
    Seekable(const std::string& filepath);
    
    /**
     * 析构函数
     */
    ~Seekable();
    
    /**
     * 获取文件大小
     * @return 文件大小（字节）
     */
    size_t size() const;
    
    /**
     * 从指定偏移量读取指定大小的数据
     * @param offset 偏移量
     * @param size 要读取的字节数
     * @return 读取的数据
     */
    std::vector<uint8_t> read_at(size_t offset, size_t size);
};

#endif // SEEKABLE_H