#ifndef SM3_H
#define SM3_H

#include <cstdint>
#include <string>
#include <vector>

class SM3 {
private:
    //初始哈希值
    uint32_t A, B, C, D, E, F, G, H;
    //输入缓冲区
    uint8_t buffer[64];
    //已处理字节数
    uint64_t total_len;
    //缓冲区当前长度
    size_t buffer_len;

    //循环左移
    static uint32_t ROTL32(uint32_t x, uint8_t n);
    //置换函数P1
    static uint32_t P1(uint32_t x);
    //置换函数P0
    static uint32_t P0(uint32_t x);
    //布尔函数FFj
    static uint32_t FF(uint32_t x, uint32_t y, uint32_t z, uint8_t j);
    //布尔函数GGj
    static uint32_t GG(uint32_t x, uint32_t y, uint32_t z, uint8_t j);
    //常量Tj
    static uint32_t T(uint8_t j);
    //压缩函数
    void compress(const uint8_t* block);

public:
    SM3();
    //更新输入数据
    void update(const uint8_t* data, size_t len);
    void update(const std::string& data);
    //完成哈希计算并输出结果
    std::vector<uint8_t> final();
    //数据哈希计算
    static std::vector<uint8_t> hash(const std::string& data);
    //批量哈希计算（用于性能测试）
    static void batch_hash(const std::string& data, std::vector<uint8_t>& digest, int iterations);
};

#endif //SM3_H
