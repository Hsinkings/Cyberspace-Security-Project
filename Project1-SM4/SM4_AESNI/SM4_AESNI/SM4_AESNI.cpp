#include "sm4_aesni.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <immintrin.h>

//S盒定义（依据：GB/T32907-2016标准）
const uint8_t SBOX[256] = {
    0xD6, 0x90, 0xE9, 0xFE, 0xCC, 0xE1, 0x3D, 0xB7, 0x16, 0xB6, 0x14, 0xC2, 0x28, 0xFB, 0x2C, 0x05,
    0x2B, 0x67, 0x9A, 0x76, 0x2A, 0xBE, 0x04, 0xC3, 0xAA, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9C, 0x42, 0x50, 0xF4, 0x91, 0xEF, 0x98, 0x7A, 0x33, 0x54, 0x0B, 0x43, 0xED, 0xCF, 0xAC, 0x62,
    0xE4, 0xB3, 0x1C, 0xA9, 0xC9, 0x08, 0xE8, 0x95, 0x80, 0xDF, 0x94, 0xFA, 0x75, 0x8F, 0x3F, 0xA6,
    0x47, 0x07, 0xA7, 0xFC, 0xF3, 0x73, 0x17, 0xBA, 0x83, 0x59, 0x3C, 0x19, 0xE6, 0x85, 0x4F, 0xA8,
    0x68, 0x6B, 0x81, 0xB2, 0x71, 0x64, 0xDA, 0x8B, 0xF8, 0xEB, 0x0F, 0x4B, 0x70, 0x56, 0x9D, 0x35,
    0x1E, 0x24, 0x0E, 0x5E, 0x63, 0x58, 0xD1, 0xA2, 0x25, 0x22, 0x7C, 0x3B, 0x01, 0x21, 0x78, 0x87,
    0xD4, 0x00, 0x46, 0x57, 0x9F, 0xD3, 0x27, 0x52, 0x4C, 0x36, 0x02, 0xE7, 0xA0, 0xC4, 0xC8, 0x9E,
    0xEA, 0xBF, 0x8A, 0xD2, 0x40, 0xC7, 0x38, 0xB5, 0xA3, 0xF7, 0xF2, 0xCE, 0xF9, 0x61, 0x15, 0xA1,
    0xE0, 0xAE, 0x5D, 0xA4, 0x9B, 0x34, 0x1A, 0x55, 0xAD, 0x93, 0x32, 0x30, 0xF5, 0x8C, 0xB1, 0xE3,
    0x1D, 0xF6, 0xE2, 0x2E, 0x82, 0x66, 0xCA, 0x60, 0xC0, 0x29, 0x23, 0xAB, 0x0D, 0x53, 0x4E, 0x6F,
    0xD5, 0xDB, 0x37, 0x45, 0xDE, 0xFD, 0x8E, 0x2F, 0x03, 0xFF, 0x6A, 0x72, 0x6D, 0x6C, 0x5B, 0x51,
    0x8D, 0x1B, 0xAF, 0x92, 0xBB, 0xDD, 0xBC, 0x7F, 0x11, 0xD9, 0x5C, 0x41, 0x1F, 0x10, 0x5A, 0xD8,
    0x0A, 0xC1, 0x31, 0x88, 0xA5, 0xCD, 0x7B, 0xBD, 0x2D, 0x74, 0xD0, 0x12, 0xB8, 0xE5, 0xB4, 0xB0,
    0x89, 0x69, 0x97, 0x4A, 0x0C, 0x96, 0x77, 0x7E, 0x65, 0xB9, 0xF1, 0x09, 0xC5, 0x6E, 0xC6, 0x84,
    0x18, 0xF0, 0x7D, 0xEC, 0x3A, 0xDC, 0x4D, 0x20, 0x79, 0xEE, 0x5F, 0x3E, 0xD7, 0xCB, 0x39, 0x48
};

//系统参数FK（依据：GB/T32907-2016标准）
const uint32_t FK[4] = { 0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC };
const uint32_t CK[32] = {
    0x00070E15, 0x1C232A31, 0x383F464D, 0x545B6269,
    0x70777E85, 0x8C939AA1, 0xA8AFB6BD, 0xC4CBD2D9,
    0xE0E7EEF5, 0xFC030A11, 0x181F262D, 0x343B4249,
    0x50575E65, 0x6C737A81, 0x888F969D, 0xA4ABB2B9,
    0xC0C7CED5, 0xDCE3EAF1, 0xF8FF060D, 0x141B2229,
    0x30373E45, 0x4C535A61, 0x686F767D, 0x848B9299,
    0xA0A7AEB5, 0xBCC3CAD1, 0xD8DFE6ED, 0xF4FB0209,
    0x10171E25, 0x2C333A41, 0x484F565D, 0x646B7279
};

//AES-NI优化的S盒变换
static inline __m128i sm4_sbox_aesni(__m128i x) {
    __m128i mask = _mm_set1_epi8(0x52);
    __m128i temp = _mm_xor_si128(x, mask);
    temp = _mm_aeskeygenassist_si128(temp, 0x00);
    temp = _mm_shuffle_epi32(temp, 0xFF);
    return _mm_xor_si128(temp, _mm_set1_epi8(0x63));
}

//循环左移
static inline __m128i mm_rotl_epi32(__m128i x, int n) {
    return _mm_or_si128(
        _mm_slli_epi32(x, n),
        _mm_srli_epi32(x, 32 - n)
    );
}

//轮函数T（S盒+线性变换）
static inline __m128i sm4_T_aesni(__m128i x, __m128i rk) {
    //轮密钥加
    __m128i temp = _mm_xor_si128(x, rk);
    //S盒变换
    temp = sm4_sbox_aesni(temp);
    //线性变换L
    return _mm_xor_si128(temp, _mm_xor_si128(
        mm_rotl_epi32(temp, 2),
        _mm_xor_si128(mm_rotl_epi32(temp, 10),
            _mm_xor_si128(mm_rotl_epi32(temp, 18), mm_rotl_epi32(temp, 24)))
    ));
}

//密钥扩展
static void SM4_keyexpansion(const uint32_t key[4], uint32_t rk[32]) {
    uint32_t K[4];
    K[0] = key[0] ^ FK[0];
    K[1] = key[1] ^ FK[1];
    K[2] = key[2] ^ FK[2];
    K[3] = key[3] ^ FK[3];

    for (int i = 0; i < 32; ++i) {
        K[(i + 4) % 4] = K[i % 4] ^ (
            [&]() {
                uint32_t t = K[(i + 1) % 4] ^ K[(i + 2) % 4] ^ K[(i + 3) % 4] ^ CK[i];
                //T'变换
                uint8_t b[4];
                b[0] = (t >> 24) & 0xFF;
                b[1] = (t >> 16) & 0xFF;
                b[2] = (t >> 8) & 0xFF;
                b[3] = t & 0xFF;
                t = (SBOX[b[0]] << 24) | (SBOX[b[1]] << 16) | (SBOX[b[2]] << 8) | SBOX[b[3]];
                return t ^ ((t << 13) | (t >> 19)) ^ ((t << 23) | (t >> 9));
            }()
                );
        rk[i] = K[(i + 4) % 4];
    }
}

//单块加密
int SM4_AESNI_encblock(const uint8_t* plaintext, uint8_t* ciphertext, const uint32_t* round_keys) {
    //字节序转换（从大端序开始）
    __m128i state = _mm_loadu_si128((const __m128i*)plaintext);
    static const __m128i shuffle_mask = _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    );
    state = _mm_shuffle_epi8(state, shuffle_mask);

    //32轮加密
    for (int i = 0; i < 32; ++i) {
        __m128i rk = _mm_set1_epi32(round_keys[i]);
        state = sm4_T_aesni(state, rk);
    }

    //逆变换输出
    static const __m128i reverse_mask = _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    state = _mm_shuffle_epi8(state, reverse_mask);
    _mm_storeu_si128((__m128i*)ciphertext, state);
    return 0;
}

//单块解密
int SM4_AESNI_decblock(const uint8_t* ciphertext, uint8_t* plaintext, const uint32_t* round_keys) {
    __m128i state = _mm_loadu_si128((const __m128i*)ciphertext);
    static const __m128i shuffle_mask = _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    );
    state = _mm_shuffle_epi8(state, shuffle_mask);

    //逆序使用轮密钥
    for (int i = 31; i >= 0; --i) {
        __m128i rk = _mm_set1_epi32(round_keys[i]);
        state = sm4_T_aesni(state, rk);
    }

    state = _mm_shuffle_epi8(state, _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    ));
    _mm_storeu_si128((__m128i*)plaintext, state);
    return 0;
}

//4块并行加密
int SM4_AESNI_enc4blocks(const uint8_t* plaintext, uint8_t* ciphertext, const uint32_t* round_keys) {
    //并行加载4个块
    __m128i blocks[4] = {
        _mm_loadu_si128((const __m128i*)(plaintext + 0)),
        _mm_loadu_si128((const __m128i*)(plaintext + 16)),
        _mm_loadu_si128((const __m128i*)(plaintext + 32)),
        _mm_loadu_si128((const __m128i*)(plaintext + 48))
    };

    //统一字节序转换
    static const __m128i shuffle_mask = _mm_set_epi8(
        12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
    );
    for (int i = 0; i < 4; ++i) {
        blocks[i] = _mm_shuffle_epi8(blocks[i], shuffle_mask);
    }

    //32轮并行加密
    for (int i = 0; i < 32; ++i) {
        __m128i rk = _mm_set1_epi32(round_keys[i]);
        for (int j = 0; j < 4; ++j) {
            blocks[j] = sm4_T_aesni(blocks[j], rk);
        }
    }

    //逆转换并存储
    static const __m128i reverse_mask = _mm_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    for (int i = 0; i < 4; ++i) {
        blocks[i] = _mm_shuffle_epi8(blocks[i], reverse_mask);
        _mm_storeu_si128((__m128i*)(ciphertext + i * 16), blocks[i]);
    }
    return 0;
}

//SM4_AESNI加密
int SM4_AESNI_enc(const uint8_t* plaintext, uint8_t* ciphertext, size_t length, const uint32_t* round_keys) {
    if (length % 16 != 0) return -2;
    size_t blocks = length / 16;
    size_t i = 0;

    //4块并行处理
    for (; i + 4 <= blocks; i += 4) {
        SM4_AESNI_enc4blocks(plaintext + i * 16, ciphertext + i * 16, round_keys);
    }

    //处理剩余块
    for (; i < blocks; ++i) {
        SM4_AESNI_encblock(plaintext + i * 16, ciphertext + i * 16, round_keys);
    }
    return 0;
}

//SM4_AESNI解密
int SM4_AESNI_dec(const uint8_t* ciphertext, uint8_t* plaintext, size_t length, const uint32_t* round_keys) {
    if (length % 16 != 0) return -2;
    size_t blocks = length / 16;
    for (size_t i = 0; i < blocks; ++i) {
        SM4_AESNI_decblock(ciphertext + i * 16, plaintext + i * 16, round_keys);
    }
    return 0;
}

//文件加密
bool encrypt_file(const std::string& plaintext_path, const std::string& key_path, const std::string& ciphertext_path) {
    std::ifstream key_file(key_path, std::ios::binary);
    if (!key_file) return false;

    uint8_t key_buf[16];
    key_file.read((char*)key_buf, 16);
    if (key_file.gcount() != 16) return false;

    uint32_t key[4], rk[32];
    for (int i = 0; i < 4; ++i) {
        key[i] = (key_buf[i * 4] << 24) | (key_buf[i * 4 + 1] << 16) | (key_buf[i * 4 + 2] << 8) | key_buf[i * 4 + 3];
    }
    SM4_keyexpansion(key, rk);

    std::ifstream plaintext_file(plaintext_path, std::ios::binary);
    std::ofstream ciphertext_file(ciphertext_path, std::ios::binary);
    if (!plaintext_file || !ciphertext_file) return false;

    uint8_t buffer[64];
    while (plaintext_file) {
        plaintext_file.read((char*)buffer, 64);
        size_t bytes_read = plaintext_file.gcount();
        if (bytes_read == 0) break;

        //不足64字节时补零
        if (bytes_read < 64) memset(buffer + bytes_read, 0, 64 - bytes_read);

        uint8_t out[64];
        SM4_AESNI_enc4blocks(buffer, out, rk);
        ciphertext_file.write((char*)out, bytes_read);
    }
    return true;
}

//文件解密
bool decrypt_file(const std::string& ciphertext_path, const std::string& key_path, const std::string& plaintext_path) {
    std::ifstream key_file(key_path, std::ios::binary);
    if (!key_file) return false;

    uint8_t key_buf[16];
    key_file.read((char*)key_buf, 16);
    if (key_file.gcount() != 16) return false;

    uint32_t key[4], rk[32];
    for (int i = 0; i < 4; ++i) {
        key[i] = (key_buf[i * 4] << 24) | (key_buf[i * 4 + 1] << 16) | (key_buf[i * 4 + 2] << 8) | key_buf[i * 4 + 3];
    }
    SM4_keyexpansion(key, rk);

    std::ifstream ciphertext_file(ciphertext_path, std::ios::binary);
    std::ofstream plaintext_file(plaintext_path, std::ios::binary);
    if (!ciphertext_file || !plaintext_file) return false;

    uint8_t buffer[64];
    while (ciphertext_file) {
        ciphertext_file.read((char*)buffer, 64);
        size_t bytes_read = ciphertext_file.gcount();
        if (bytes_read == 0) break;

        uint8_t out[64];
        SM4_AESNI_enc4blocks(buffer, out, rk); 
        plaintext_file.write((char*)out, bytes_read);
    }
    return true;
}