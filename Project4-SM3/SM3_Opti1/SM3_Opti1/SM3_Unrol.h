#ifndef SM3_H
#define SM3_H

#include <cstdint>
#include <string>
#include <vector>

//常量定义
#define SM3_DIGEST_SIZE 32
#define SM3_BLOCK_SIZE 64
#define SM3_STATE_WORDS 8

//循环左移宏（减少函数调用开销）
#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

//置换函数宏
#define P1(x) ((x) ^ ROTL32((x), 15) ^ ROTL32((x), 23))
#define P0(x) ((x) ^ ROTL32((x), 9) ^ ROTL32((x), 17))

//布尔函数宏（避免函数调用开销）
#define FF(x, y, z, j) ((j) <= 15 ? ((x) ^ (y) ^ (z)) : (((x) & (y)) | ((x) & (z)) | ((y) & (z))))
#define GG(x, y, z, j) ((j) <= 15 ? ((x) ^ (y) ^ (z)) : (((x) & (y)) | (~(x) & (z))))

//常量T宏
#define T(j) ((j) <= 15 ? 0x79CC4519 : 0x7A879D8A)

class SM3 {
private:
    //哈希状态变量（直接使用变量而非数组，提升访问速度）
    uint32_t A, B, C, D, E, F, G, H;
    //输入缓冲区
    uint8_t buffer[SM3_BLOCK_SIZE];
    //已处理比特数
    uint64_t total_bits;
    //缓冲区当前长度（字节）
    size_t buffer_len;

    //压缩单分组（核心优化点）
    void compress(const uint8_t* block);

    //字节序转换：小端转大端（32位）
    static uint32_t le_to_be32(uint32_t x) {
        return (x >> 24) | ((x >> 8) & 0x0000FF00) |
            ((x << 8) & 0x00FF0000) | (x << 24);
    }

public:
    SM3();
    //更新输入数据
    void update(const uint8_t* data, size_t len);
    void update(const std::string& data);
    //完成哈希计算并输出结果
    std::vector<uint8_t> final();
    //快捷计算哈希
    static std::vector<uint8_t> hash(const std::string& data);
    //批量哈希计算（用于性能测试）
    static void batch_hash(const std::string& data, std::vector<uint8_t>& digest, int iterations);
};

#endif //SM3_H