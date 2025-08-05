#include "SM3_SIMD.h"
#include <cstring>
#include <algorithm>

//AVX2指令集优化的核心函数实现
static inline __m256i avx2_rol32(__m256i x, int n) {
    return _mm256_or_si256(_mm256_slli_epi32(x, n), _mm256_srli_epi32(x, 32 - n));
}

static inline __m256i avx2_p0(__m256i x) {
    __m256i rot9 = avx2_rol32(x, 9);
    __m256i rot17 = avx2_rol32(x, 17);
    return _mm256_xor_si256(_mm256_xor_si256(x, rot9), rot17);
}

static inline __m256i avx2_p1(__m256i x) {
    __m256i rot15 = avx2_rol32(x, 15);
    __m256i rot23 = avx2_rol32(x, 23);
    return _mm256_xor_si256(_mm256_xor_si256(x, rot15), rot23);
}

static inline __m256i avx2_ff(__m256i x, __m256i y, __m256i z, int round) {
    if (round <= 15) {
        return _mm256_xor_si256(_mm256_xor_si256(x, y), z);
    }
    else {
        __m256i xy = _mm256_and_si256(x, y);
        __m256i xz = _mm256_and_si256(x, z);
        __m256i yz = _mm256_and_si256(y, z);
        return _mm256_or_si256(_mm256_or_si256(xy, xz), yz);
    }
}

static inline __m256i avx2_gg(__m256i x, __m256i y, __m256i z, int round) {
    if (round <= 15) {
        return _mm256_xor_si256(_mm256_xor_si256(x, y), z);
    }
    else {
        __m256i xy = _mm256_and_si256(x, y);
        __m256i not_x = _mm256_xor_si256(x, _mm256_set1_epi32(0xFFFFFFFF));
        __m256i not_x_z = _mm256_and_si256(not_x, z);
        return _mm256_or_si256(xy, not_x_z);
    }
}

//SM3初始向量
const uint32_t SM3_IV[SM3_STATE_WORDS] = {
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

//预计算T值常量表
static const uint32_t T_TABLE[64] = {
    0x79cc4519, 0xf3988a32, 0xe7311465, 0xce6228cb,
    0x9cc45197, 0x3988a32f, 0x7311465e, 0xe6228cbc,
    0xcc451979, 0x988a32f3, 0x311465e7, 0x6228cbce,
    0xc451979c, 0x88a32f39, 0x11465e73, 0x228cbce6,
    0x7a879d8a, 0xf50f3b14, 0xea1e7628, 0xd43cec51,
    0xa879d8a2, 0x50f3b145, 0xa1e7628b, 0x43cec516,
    0x879d8a2c, 0x0f3b1459, 0x1e7628b2, 0x3cec5164,
    0x79d8a2c9, 0xf3b14592, 0xe7628b24, 0xcec51648,
    0x9d8a2c91, 0x3b145923, 0x7628b246, 0xec51648d,
    0xd8a2c91a, 0xb1459234, 0x628b2468, 0xc51648d1,
    0x8a2c91a2, 0x14592345, 0x28b2468a, 0x51648d15,
    0xa2c91a2a, 0x45923454, 0x8b2468a9, 0x1648d152,
    0x2c91a2a5, 0x5923454a, 0xb2468a95, 0x648d152a,
    0xc91a2a55, 0x923454aa, 0x2468a954, 0x48d152a9,
    0x91a2a552, 0x23454aa5, 0x468a954a, 0x8d152a95,
    0x1a2a552a, 0x3454aa54, 0x68a954a9, 0xd152a952
};

//SIMD消息扩展函数
static void sm3_simd_expand(const uint32_t X[SIMD_LANES][16],
    uint32_t W[SIMD_LANES][68],
    uint32_t W1[SIMD_LANES][64]) {
    __m256i w[68];

    //加载前16个字
    for (int j = 0; j < 16; j++) {
        w[j] = _mm256_loadu_si256((__m256i*) & X[0][j]);
    }

    //SIMD并行消息扩展
    for (int j = 16; j < 68; j++) {
        __m256i temp1 = _mm256_xor_si256(w[j - 16], w[j - 9]);
        __m256i temp2 = _mm256_xor_si256(temp1, avx2_rol32(w[j - 3], 15));
        __m256i temp3 = avx2_p1(temp2);
        __m256i temp4 = _mm256_xor_si256(temp3, avx2_rol32(w[j - 13], 7));
        w[j] = _mm256_xor_si256(temp4, w[j - 6]);
    }

    //存储扩展结果
    for (int j = 0; j < 68; j++) {
        _mm256_storeu_si256((__m256i*) & W[0][j], w[j]);
    }

    //计算W1
    for (int j = 0; j < 64; j++) {
        __m256i w_j = _mm256_loadu_si256((__m256i*) & W[0][j]);
        __m256i w_j4 = _mm256_loadu_si256((__m256i*) & W[0][j + 4]);
        __m256i w1_j = _mm256_xor_si256(w_j, w_j4);
        _mm256_storeu_si256((__m256i*) & W1[0][j], w1_j);
    }
}

//8路并行压缩函数
static void sm3_simd_compress_8way(uint32_t state[SIMD_LANES][8],
    const uint8_t blocks[SIMD_LANES][SM3_BLOCK_SIZE]) {
    uint32_t W[SIMD_LANES][68], W1[SIMD_LANES][64];
    uint32_t X[SIMD_LANES][16];
    __m256i A, B, C, D, E, F, G, H;
    __m256i SS1, SS2, TT1, TT2, T;
    int i, j;

    //将8个64字节块转换为16个32位字
    for (i = 0; i < SIMD_LANES; i++) {
        for (j = 0; j < 16; j++) {
            X[i][j] = ((uint32_t)blocks[i][j * 4] << 24) |
                ((uint32_t)blocks[i][j * 4 + 1] << 16) |
                ((uint32_t)blocks[i][j * 4 + 2] << 8) |
                ((uint32_t)blocks[i][j * 4 + 3]);
        }
    }

    //SIMD消息扩展
    sm3_simd_expand(X, W, W1);

    //加载初始状态
    A = _mm256_loadu_si256((__m256i*) & state[0][0]);
    B = _mm256_loadu_si256((__m256i*) & state[0][1]);
    C = _mm256_loadu_si256((__m256i*) & state[0][2]);
    D = _mm256_loadu_si256((__m256i*) & state[0][3]);
    E = _mm256_loadu_si256((__m256i*) & state[0][4]);
    F = _mm256_loadu_si256((__m256i*) & state[0][5]);
    G = _mm256_loadu_si256((__m256i*) & state[0][6]);
    H = _mm256_loadu_si256((__m256i*) & state[0][7]);

    //64轮SIMD并行压缩
    for (j = 0; j < 64; j++) {
        //加载常量T
        T = _mm256_set1_epi32(T_TABLE[j]);

        //加载W和W1
        __m256i W_j = _mm256_loadu_si256((__m256i*) & W[0][j]);
        __m256i W1_j = _mm256_loadu_si256((__m256i*) & W1[0][j]);

        //计算SS1和SS2
        __m256i temp = _mm256_add_epi32(avx2_rol32(A, 12), E);
        temp = _mm256_add_epi32(temp, avx2_rol32(T, j % 32));
        SS1 = avx2_rol32(temp, 7);
        SS2 = _mm256_xor_si256(SS1, avx2_rol32(A, 12));

        //计算TT1和TT2
        TT1 = _mm256_add_epi32(avx2_ff(A, B, C, j), D);
        TT1 = _mm256_add_epi32(TT1, SS2);
        TT1 = _mm256_add_epi32(TT1, W1_j);

        TT2 = _mm256_add_epi32(avx2_gg(E, F, G, j), H);
        TT2 = _mm256_add_epi32(TT2, SS1);
        TT2 = _mm256_add_epi32(TT2, W_j);

        //更新工作变量
        D = C;
        C = avx2_rol32(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = avx2_rol32(F, 19);
        F = E;
        E = avx2_p0(TT2);
    }

    //存储更新后状态
    __m256i state0 = _mm256_loadu_si256((__m256i*) & state[0][0]);
    __m256i state1 = _mm256_loadu_si256((__m256i*) & state[0][1]);
    __m256i state2 = _mm256_loadu_si256((__m256i*) & state[0][2]);
    __m256i state3 = _mm256_loadu_si256((__m256i*) & state[0][3]);
    __m256i state4 = _mm256_loadu_si256((__m256i*) & state[0][4]);
    __m256i state5 = _mm256_loadu_si256((__m256i*) & state[0][5]);
    __m256i state6 = _mm256_loadu_si256((__m256i*) & state[0][6]);
    __m256i state7 = _mm256_loadu_si256((__m256i*) & state[0][7]);

    _mm256_storeu_si256((__m256i*) & state[0][0], _mm256_xor_si256(state0, A));
    _mm256_storeu_si256((__m256i*) & state[0][1], _mm256_xor_si256(state1, B));
    _mm256_storeu_si256((__m256i*) & state[0][2], _mm256_xor_si256(state2, C));
    _mm256_storeu_si256((__m256i*) & state[0][3], _mm256_xor_si256(state3, D));
    _mm256_storeu_si256((__m256i*) & state[0][4], _mm256_xor_si256(state4, E));
    _mm256_storeu_si256((__m256i*) & state[0][5], _mm256_xor_si256(state5, F));
    _mm256_storeu_si256((__m256i*) & state[0][6], _mm256_xor_si256(state6, G));
    _mm256_storeu_si256((__m256i*) & state[0][7], _mm256_xor_si256(state7, H));
}

//构造函数
SM3::SM3() {
    A = SM3_IV[0];
    B = SM3_IV[1];
    C = SM3_IV[2];
    D = SM3_IV[3];
    E = SM3_IV[4];
    F = SM3_IV[5];
    G = SM3_IV[6];
    H = SM3_IV[7];

    total_bits = 0;
    buffer_len = 0;
    std::memset(buffer, 0, SM3_BLOCK_SIZE);
}

//单路压缩函数
void SM3::compress(const uint8_t* block) {
    uint32_t W[68], W1[64];
    const uint32_t* p = reinterpret_cast<const uint32_t*>(block);

    //消息扩展：前16个字
    for (int i = 0; i < 16; ++i) {
        W[i] = le_to_be32(p[i]);
    }

    //消息扩展：16-67个字
    for (int j = 16; j < 68; ++j) {
        W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTL32(W[j - 3], 15)) ^
            ROTL32(W[j - 13], 7) ^ W[j - 6];
    }

    //计算W'数组
    for (int j = 0; j < 64; ++j) {
        W1[j] = W[j] ^ W[j + 4];
    }

    //状态压缩 - 循环展开优化
    uint32_t a = A, b = B, c = C, d = D;
    uint32_t e = E, f = F, g = G, h = H;

    //64轮压缩，8轮一组展开
    for (int j = 0; j < 64; j += 8) {
        //展开8轮计算，减少循环分支开销
#define ROUND(i) \
        do { \
            uint32_t SS1 = ROTL32(ROTL32(a, 12) + e + ROTL32(T(i), i), 7); \
            uint32_t SS2 = SS1 ^ ROTL32(a, 12); \
            uint32_t TT1 = FF(a, b, c, i) + d + SS2 + W1[i]; \
            uint32_t TT2 = GG(e, f, g, i) + h + SS1 + W[i]; \
            d = c; c = ROTL32(b, 9); b = a; a = TT1; \
            h = g; g = ROTL32(f, 19); f = e; e = P0(TT2); \
        } while(0)

        ROUND(j);
        ROUND(j + 1);
        ROUND(j + 2);
        ROUND(j + 3);
        ROUND(j + 4);
        ROUND(j + 5);
        ROUND(j + 6);
        ROUND(j + 7);

#undef ROUND
    }

    //更新状态变量
    A ^= a;
    B ^= b;
    C ^= c;
    D ^= d;
    E ^= e;
    F ^= f;
    G ^= g;
    H ^= h;
}

//更新函数
void SM3::update(const uint8_t* data, size_t len) {
    if (len == 0) return;

    total_bits += len * 8;

    //处理缓冲区中的剩余数据
    if (buffer_len > 0) {
        size_t fill = SM3_BLOCK_SIZE - buffer_len;
        if (len >= fill) {
            std::memcpy(buffer + buffer_len, data, fill);
            compress(buffer);
            data += fill;
            len -= fill;
            buffer_len = 0;
        }
        else {
            std::memcpy(buffer + buffer_len, data, len);
            buffer_len += len;
            return;
        }
    }

    //处理完整块
    while (len >= SM3_BLOCK_SIZE) {
        compress(data);
        data += SM3_BLOCK_SIZE;
        len -= SM3_BLOCK_SIZE;
    }

    //保存剩余数据
    if (len > 0) {
        std::memcpy(buffer, data, len);
        buffer_len = len;
    }
}

void SM3::update(const std::string& data) {
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

//最终处理
std::vector<uint8_t> SM3::final() {
    //添加填充位0x80
    buffer[buffer_len++] = 0x80;

    //处理长度和填充字段
    if (buffer_len > 56) {
        std::memset(buffer + buffer_len, 0, SM3_BLOCK_SIZE - buffer_len);
        compress(buffer);
        std::memset(buffer, 0, 56);
        buffer_len = 56;
    }
    else {
        std::memset(buffer + buffer_len, 0, 56 - buffer_len);
        buffer_len = 56;
    }

    //添加64位长度字段（大端序）
    uint64_t len = total_bits;
    for (int i = 0; i < 8; ++i) {
        buffer[56 + i] = static_cast<uint8_t>(len >> (8 * (7 - i)));
    }

    //压缩最后一个块
    compress(buffer);

    //返回最终结果
    std::vector<uint8_t> result(SM3_DIGEST_SIZE);
    uint32_t* hash = reinterpret_cast<uint32_t*>(result.data());

    hash[0] = le_to_be32(A);
    hash[1] = le_to_be32(B);
    hash[2] = le_to_be32(C);
    hash[3] = le_to_be32(D);
    hash[4] = le_to_be32(E);
    hash[5] = le_to_be32(F);
    hash[6] = le_to_be32(G);
    hash[7] = le_to_be32(H);

    return result;
}

//数据哈希计算
std::vector<uint8_t> SM3::hash(const std::string& data) {
    SM3 sm3;
    sm3.update(data);
    return sm3.final();
}

//8路并行哈希计算（字符串输入）
void SM3::batch_hash_8way(const std::vector<std::string>& data,
    std::vector<std::vector<uint8_t>>& digests) {
    //确保数据数量为8的倍数
    size_t count = data.size();
    size_t batches = (count + SIMD_LANES - 1) / SIMD_LANES;
    digests.resize(count);

    uint8_t input_blocks[SIMD_LANES][SM3_BLOCK_SIZE];
    uint8_t output_digests[SIMD_LANES][SM3_DIGEST_SIZE];
    uint32_t states[SIMD_LANES][8];

    for (size_t batch = 0; batch < batches; ++batch) {
        //准备当前批次的8个输入
        size_t start_idx = batch * SIMD_LANES;
        size_t end_idx = std::min(start_idx + SIMD_LANES, count);

        //初始化状态和输入
        for (size_t i = 0; i < SIMD_LANES; ++i) {
            memcpy(states[i], SM3_IV, sizeof(SM3_IV));
            memset(input_blocks[i], 0, SM3_BLOCK_SIZE);

            size_t data_idx = start_idx + i;
            if (data_idx < count) {
                const std::string& str = data[data_idx];
                size_t copy_len = std::min(str.size(), (size_t)SM3_BLOCK_SIZE);
                memcpy(input_blocks[i], str.data(), copy_len);
            }
        }

        //执行8路并行压缩
        sm3_simd_compress_8way(states, input_blocks);

        //转换输出格式
        for (size_t i = 0; i < SIMD_LANES; ++i) {
            size_t data_idx = start_idx + i;
            if (data_idx >= count) break;

            digests[data_idx].resize(SM3_DIGEST_SIZE);
            for (int j = 0; j < 8; ++j) {
                output_digests[i][j * 4] = (uint8_t)(states[i][j] >> 24);
                output_digests[i][j * 4 + 1] = (uint8_t)(states[i][j] >> 16);
                output_digests[i][j * 4 + 2] = (uint8_t)(states[i][j] >> 8);
                output_digests[i][j * 4 + 3] = (uint8_t)(states[i][j]);
            }
            memcpy(digests[data_idx].data(), output_digests[i], SM3_DIGEST_SIZE);
        }
    }
}

//8路并行哈希计算（原始数据输入）
void SM3::batch_hash_8way(const uint8_t data[SIMD_LANES][SM3_BLOCK_SIZE],
    uint8_t digests[SIMD_LANES][SM3_DIGEST_SIZE]) {
    uint32_t states[SIMD_LANES][8];

    //初始化状态
    for (int i = 0; i < SIMD_LANES; ++i) {
        memcpy(states[i], SM3_IV, sizeof(SM3_IV));
    }

    //执行8路并行压缩
    sm3_simd_compress_8way(states, data);

    //转换输出格式
    for (int i = 0; i < SIMD_LANES; ++i) {
        for (int j = 0; j < 8; ++j) {
            digests[i][j * 4] = (uint8_t)(states[i][j] >> 24);
            digests[i][j * 4 + 1] = (uint8_t)(states[i][j] >> 16);
            digests[i][j * 4 + 2] = (uint8_t)(states[i][j] >> 8);
            digests[i][j * 4 + 3] = (uint8_t)(states[i][j]);
        }
    }
}

//原始批量哈希计算（用于性能对比）
void SM3::batch_hash(const std::string& data, std::vector<uint8_t>& digest, int iterations) {
    if (iterations <= 0) return;

    SM3 sm3;

    for (int i = 0; i < iterations; ++i) {
        sm3.update(data);
        digest = sm3.final();

        if (i != iterations - 1) {
            sm3 = SM3();  //重新初始化
        }
    }
}