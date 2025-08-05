#ifndef SM3_SIMD_H
#define SM3_SIMD_H

#include <cstdint>
#include <string>
#include <vector>
#include <immintrin.h>  //AVX2指令集头文件

//常量定义
#define SM3_DIGEST_SIZE 32
#define SM3_BLOCK_SIZE 64
#define SM3_STATE_WORDS 8
#define SIMD_LANES 8    //AVX2支持8路32位运算

//循环左移宏
#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

//置换函数宏
#define P1(x) ((x) ^ ROTL32((x), 15) ^ ROTL32((x), 23))
#define P0(x) ((x) ^ ROTL32((x), 9) ^ ROTL32((x), 17))

//布尔函数宏
#define FF(x, y, z, j) ((j) <= 15 ? ((x) ^ (y) ^ (z)) : (((x) & (y)) | ((x) & (z)) | ((y) & (z))))
#define GG(x, y, z, j) ((j) <= 15 ? ((x) ^ (y) ^ (z)) : (((x) & (y)) | (~(x) & (z))))

//常量T表
#define T(j) ((j) <= 15 ? 0x79CC4519 : 0x7A879D8A)

//AVX2向量化函数声明
static inline __m256i avx2_rol32(__m256i x, int n);
static inline __m256i avx2_p0(__m256i x);
static inline __m256i avx2_p1(__m256i x);
static inline __m256i avx2_ff(__m256i x, __m256i y, __m256i z, int round);
static inline __m256i avx2_gg(__m256i x, __m256i y, __m256i z, int round);

class SM3 {
private:
    //单路哈希状态变量
    uint32_t A, B, C, D, E, F, G, H;
    uint8_t buffer[SM3_BLOCK_SIZE];
    uint64_t total_bits;
    size_t buffer_len;

    //压缩函数
    void compress(const uint8_t* block);

    //字节序转换：小端转大端序（32位）
    static uint32_t le_to_be32(uint32_t x) {
        return (x >> 24) | ((x >> 8) & 0x0000FF00) |
            ((x << 8) & 0x00FF0000) | (x << 24);
    }

public:
    SM3();

    //单路更新和计算接口
    void update(const uint8_t* data, size_t len);
    void update(const std::string& data);
    std::vector<uint8_t> final();

    //单路数据计算哈希
    static std::vector<uint8_t> hash(const std::string& data);

    //SIMD批量计算接口
    static void batch_hash_8way(const std::vector<std::string>& data,
        std::vector<std::vector<uint8_t>>& digests);
    static void batch_hash_8way(const uint8_t data[SIMD_LANES][SM3_BLOCK_SIZE],
        uint8_t digests[SIMD_LANES][SM3_DIGEST_SIZE]);

    //原始批量哈希计算（用于性能对比）
    static void batch_hash(const std::string& data, std::vector<uint8_t>& digest, int iterations);
};

#endif //SM3_SIMD_H
