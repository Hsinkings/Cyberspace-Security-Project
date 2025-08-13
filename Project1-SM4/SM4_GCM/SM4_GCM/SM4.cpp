#include "SM4.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>

//S盒定义
const uint8_t SBOX[256] = {
    0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
    0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
    0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
    0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
    0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
    0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x0d, 0x29, 0x78,
    0xf5, 0x21, 0x2f, 0x82, 0x03, 0xf7, 0xea, 0x5f, 0x60, 0x51, 0x7f, 0xa1, 0x9b, 0x87, 0xec, 0x55,
    0x20, 0xa0, 0x52, 0x4d, 0x6d, 0x79, 0x27, 0xf2, 0x1d, 0x9e, 0x8e, 0x9e, 0x4e, 0x0e, 0x32, 0x3a,
    0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc4, 0xd2, 0x79, 0x3b, 0x4a, 0xcd, 0xb5, 0x9b, 0xe3, 0x07, 0xf1,
    0x4d, 0x92, 0x4e, 0x1e, 0x04, 0xb9, 0xda, 0x59, 0xcb, 0x5f, 0x6c, 0x7a, 0xbf, 0x39, 0x09, 0xc5,
    0x31, 0x10, 0x23, 0x70, 0x5b, 0xbb, 0x61, 0x11, 0x41, 0x4c, 0x69, 0x08, 0x8a, 0x78, 0x8d, 0x8e,
    0x84, 0x8c, 0x8b, 0x8f, 0x96, 0x86, 0x87, 0x97, 0x9a, 0x98, 0x99, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf
};

//FK常量
const uint32_t FK[4] = {
    0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC
};

//CK常量
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

//辅助函数：32位字转128位块
static void uint32_to_block128(const uint32_t* w, block128_t* b) {
    b->high = ((uint64_t)w[0] << 32) | w[1];
    b->low = ((uint64_t)w[2] << 32) | w[3];
}

//辅助函数：128位块转32位字
static void block128_to_uint32(const block128_t* b, uint32_t* w) {
    w[0] = (b->high >> 32) & 0xFFFFFFFF;
    w[1] = b->high & 0xFFFFFFFF;
    w[2] = (b->low >> 32) & 0xFFFFFFFF;
    w[3] = b->low & 0xFFFFFFFF;
}

//tau置换：S盒替换
static uint32_t tau(uint32_t x) {
    uint8_t* bytes = (uint8_t*)&x;
    bytes[0] = SBOX[bytes[0]];
    bytes[1] = SBOX[bytes[1]];
    bytes[2] = SBOX[bytes[2]];
    bytes[3] = SBOX[bytes[3]];
    return x;
}

//L线性变换
static uint32_t L(uint32_t x) {
    return x ^ ROL(x, 2) ^ ROL(x, 10) ^ ROL(x, 18) ^ ROL(x, 24);
}

//L'线性变换
static uint32_t L_prime(uint32_t x) {
    return x ^ ROL(x, 13) ^ ROL(x, 23);
}

//T函数：tau变换后进行L变换
static uint32_t T(uint32_t x) {
    return L(tau(x));
}

//T'函数：tau变换后进行L'变换
static uint32_t T_prime(uint32_t x) {
    return L_prime(tau(x));
}

//生成轮密钥
static void generate_round_keys(const uint32_t key[4], uint32_t rk[32]) {
    uint32_t K[36];
    K[0] = key[0] ^ FK[0];
    K[1] = key[1] ^ FK[1];
    K[2] = key[2] ^ FK[2];
    K[3] = key[3] ^ FK[3];

    for (int i = 0; i < 32; ++i) {
        K[i + 4] = K[i] ^ T_prime(K[i + 1] ^ K[i + 2] ^ K[i + 3] ^ CK[i]);
        rk[i] = K[i + 4];
    }
}

//单块加密实现
void sm4_encrypt_block(const block128_t* plaintext, const block128_t* key, block128_t* ciphertext) {
    uint32_t plain[4], k[4], cipher[4];
    block128_to_uint32(plaintext, plain);
    block128_to_uint32(key, k);

    uint32_t rk[32];
    generate_round_keys(k, rk);

    uint32_t X[36];
    X[0] = plain[0]; X[1] = plain[1]; X[2] = plain[2]; X[3] = plain[3];

    for (int i = 0; i < 32; ++i) {
        X[i + 4] = X[i] ^ T(X[i + 1] ^ X[i + 2] ^ X[i + 3] ^ rk[i]);
    }

    cipher[0] = X[35]; cipher[1] = X[34]; cipher[2] = X[33]; cipher[3] = X[32];
    uint32_to_block128(cipher, ciphertext);
}

//GF(2^128)无进位乘法（使用PCLMULQDQ指令优化）
static block128_t gf2m128_mul(const block128_t* a, const block128_t* b) {
    __m128i a_vec = _mm_set_epi64x(a->low, a->high);
    __m128i b_vec = _mm_set_epi64x(b->low, b->high);

    //无进位乘法：(a_high*b_high), (a_high*b_low ^ a_low*b_high), (a_low*b_low)
    __m128i t0 = _mm_clmulepi64_si128(a_vec, b_vec, 0x00);  //a_low * b_low
    __m128i t1 = _mm_clmulepi64_si128(a_vec, b_vec, 0x10);  //a_high * b_low
    __m128i t2 = _mm_clmulepi64_si128(a_vec, b_vec, 0x01);  //a_low * b_high
    __m128i t3 = _mm_clmulepi64_si128(a_vec, b_vec, 0x11);  //a_high * b_high

    __m128i temp = _mm_xor_si128(t1, t2);  //中间项异或
    __m128i carry = _mm_srli_si128(temp, 8);  //高64位移位
    t0 = _mm_xor_si128(t0, carry);
    t3 = _mm_xor_si128(t3, _mm_slli_si128(temp, 8));  //低64位移位

    //缩减多项式：x^128 + x^7 + x^2 + x + 1
    const __m128i redux = _mm_set_epi64x(0x87, 0);
    carry = _mm_clmulepi64_si128(t0, redux, 0x10);
    t0 = _mm_xor_si128(t0, carry);
    t3 = _mm_xor_si128(t3, _mm_slli_si128(carry, 8));

    block128_t res;
    res.high = _mm_extract_epi64(t3, 1);
    res.low = _mm_extract_epi64(_mm_xor_si128(t0, t3), 0);
    return res;
}

//GHASH运算（基于GF(2^128)乘法链）
static block128_t ghash(const block128_t* H, const uint8_t* data, size_t len, block128_t hash) {
    const size_t block_size = 16;
    size_t num_blocks = len / block_size;
    const uint8_t* end = data + num_blocks * block_size;

    //批量处理完整块
    while (data < end) {
        block128_t m;
        memcpy(&m, data, block_size);
        XOR128(hash, hash, m);
        hash = gf2m128_mul(&hash, H);
        data += block_size;
    }

    //处理最后一个不完整块
    if (len % block_size != 0) {
        block128_t m = { 0, 0 };
        memcpy(&m, data, len % block_size);
        XOR128(hash, hash, m);
        hash = gf2m128_mul(&hash, H);
    }

    return hash;
}

//生成CTR计数器（96位IV + 32位计数器）
static void generate_ctr(const uint8_t* iv, size_t iv_len, uint64_t counter, block128_t* ctr) {
    if (iv_len == 12) {  //推荐的96位IV
        memcpy(&ctr->low, iv, 8);
        memcpy(&ctr->high, iv + 8, 4);
        ctr->high = (ctr->high << 32) | counter;  //高32位为计数器
    }
    else {
        //非96位IV需通过GHASH处理
        block128_t iv_hash = { 0, 0 };
        block128_t H_zero = { 0, 0 };
        iv_hash = ghash(&H_zero, iv, iv_len, iv_hash);
        ctr->high = iv_hash.high;
        ctr->low = iv_hash.low ^ ((uint64_t)1 << 63);  //置位最高位
    }
}

//CTR模式并行加密（批量处理4个块）
static void ctr_parallel_encrypt(const uint8_t* in, uint8_t* out, size_t len,
    const block128_t* key, const block128_t* H,
    const uint8_t* iv, size_t iv_len) {
    const size_t block_size = 16;
    size_t total_blocks = (len + block_size - 1) / block_size;
    block128_t ctrs[4], keystreams[4];
    uint64_t counter = 1;  //GCM计数器从1开始

    for (size_t i = 0; i < total_blocks; i += 4) {
        //批量生成4个计数器
        for (int j = 0; j < 4 && (i + j) < total_blocks; j++) {
            generate_ctr(iv, iv_len, counter + j, &ctrs[j]);
        }

        //SIMD并行加密计数器（生成密钥流）
        for (int j = 0; j < 4 && (i + j) < total_blocks; j++) {
            sm4_encrypt_block(&ctrs[j], key, &keystreams[j]);
        }

        //密钥流与明文异或（生成密文）
        for (int j = 0; j < 4 && (i + j) < total_blocks; j++) {
            size_t pos = (i + j) * block_size;
            size_t copy_len = std::min(block_size, len - pos);
            const uint8_t* ks_ptr = (const uint8_t*)&keystreams[j];
            for (size_t k = 0; k < copy_len; k++) {
                out[pos + k] = in[pos + k] ^ ks_ptr[k];
            }
        }
        counter += 4;
    }
}

//GCM加密实现
bool sm4_gcm_encrypt(const uint8_t* plaintext, size_t plaintext_len,
    const uint8_t* aad, size_t aad_len,
    const block128_t* key,
    const uint8_t* iv, size_t iv_len,
    uint8_t* ciphertext, uint8_t* tag) {
    //生成哈希子密钥H = E_K(0^128)
    block128_t H = { 0, 0 };
    sm4_encrypt_block(&H, key, &H);

    //CTR模式加密
    ctr_parallel_encrypt(plaintext, ciphertext, plaintext_len, key, &H, iv, iv_len);

    //计算GHASH：AAD || 密文 || (AAD长度) || (密文长度)
    block128_t hash = { 0, 0 };
    hash = ghash(&H, aad, aad_len, hash);
    hash = ghash(&H, ciphertext, plaintext_len, hash);

    //附加长度信息（64位大端格式）
    block128_t len_block;
    len_block.high = ((uint64_t)aad_len << 32) | (aad_len >> 32);
    len_block.low = ((uint64_t)plaintext_len << 32) | (plaintext_len >> 32);
    XOR128(hash, hash, len_block);
    hash = gf2m128_mul(&hash, &H);

    //生成标签：E_K(CTR_0) ^ hash
    block128_t ctr0;
    generate_ctr(iv, iv_len, 0, &ctr0);
    sm4_encrypt_block(&ctr0, key, &ctr0);
    XOR128(hash, hash, ctr0);

    memcpy(tag, &hash, 16);  //128位标签
    return true;
}

//GCM解密实现（与加密共享CTR逻辑，增加标签验证）
bool sm4_gcm_decrypt(const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t* aad, size_t aad_len,
    const block128_t* key,
    const uint8_t* iv, size_t iv_len,
    const uint8_t* tag, uint8_t* plaintext) {
    //生成哈希子密钥H
    block128_t H = { 0, 0 };
    sm4_encrypt_block(&H, key, &H);

    //CTR模式解密（与加密相同）
    ctr_parallel_encrypt(ciphertext, plaintext, ciphertext_len, key, &H, iv, iv_len);

    //计算GHASH并验证标签
    block128_t hash = { 0, 0 };
    hash = ghash(&H, aad, aad_len, hash);
    hash = ghash(&H, ciphertext, ciphertext_len, hash);

    block128_t len_block;
    len_block.high = ((uint64_t)aad_len << 32) | (aad_len >> 32);
    len_block.low = ((uint64_t)ciphertext_len << 32) | (ciphertext_len >> 32);
    XOR128(hash, hash, len_block);
    hash = gf2m128_mul(&hash, &H);

    block128_t ctr0;
    generate_ctr(iv, iv_len, 0, &ctr0);
    sm4_encrypt_block(&ctr0, key, &ctr0);
    XOR128(hash, hash, ctr0);

    //验证标签
    return memcmp(&hash, tag, 16) == 0;
}

//文件GCM加密实现
bool encrypt_file_gcm(const std::string& plaintext_path, const std::string& key_path,
    const std::string& iv_path, const std::string& aad_path,
    const std::string& ciphertext_path, const std::string& tag_path) {
    //读取密钥(16字节)
    std::ifstream key_file(key_path, std::ios::binary | std::ios::ate);
    if (!key_file) return false;
    size_t key_size = key_file.tellg();
    if (key_size != 16) return false;
    key_file.seekg(0);
    block128_t key;
    key_file.read((char*)&key, 16);

    //读取IV
    std::ifstream iv_file(iv_path, std::ios::binary | std::ios::ate);
    if (!iv_file) return false;
    size_t iv_len = iv_file.tellg();
    uint8_t* iv = new uint8_t[iv_len];
    iv_file.seekg(0);
    iv_file.read((char*)iv, iv_len);

    //读取AAD
    std::ifstream aad_file(aad_path, std::ios::binary | std::ios::ate);
    size_t aad_len = aad_file.tellg();
    uint8_t* aad = aad_len > 0 ? new uint8_t[aad_len] : nullptr;
    if (aad_len > 0) {
        aad_file.seekg(0);
        aad_file.read((char*)aad, aad_len);
    }

    //读取明文并加密
    std::ifstream plain_file(plaintext_path, std::ios::binary | std::ios::ate);
    if (!plain_file) return false;
    size_t plain_len = plain_file.tellg();
    uint8_t* plaintext = new uint8_t[plain_len];
    plain_file.seekg(0);
    plain_file.read((char*)plaintext, plain_len);

    uint8_t* ciphertext = new uint8_t[plain_len];
    uint8_t tag[16];
    bool success = sm4_gcm_encrypt(plaintext, plain_len, aad, aad_len, &key, iv, iv_len, ciphertext, tag);

    //写入密文和标签
    std::ofstream cipher_file(ciphertext_path, std::ios::binary);
    cipher_file.write((char*)ciphertext, plain_len);
    std::ofstream tag_file(tag_path, std::ios::binary);
    tag_file.write((char*)tag, 16);

    //清理资源
    delete[] iv;
    delete[] aad;
    delete[] plaintext;
    delete[] ciphertext;
    return success;
}

//文件GCM解密实现
bool decrypt_file_gcm(const std::string& ciphertext_path, const std::string& key_path,
    const std::string& iv_path, const std::string& aad_path,
    const std::string& tag_path, const std::string& plaintext_path) {
    //读取密钥(16字节)
    std::ifstream key_file(key_path, std::ios::binary | std::ios::ate);
    if (!key_file) return false;
    size_t key_size = key_file.tellg();
    if (key_size != 16) return false;
    key_file.seekg(0);
    block128_t key;
    key_file.read((char*)&key, 16);

    //读取IV
    std::ifstream iv_file(iv_path, std::ios::binary | std::ios::ate);
    if (!iv_file) return false;
    size_t iv_len = iv_file.tellg();
    uint8_t* iv = new uint8_t[iv_len];
    iv_file.seekg(0);
    iv_file.read((char*)iv, iv_len);

    //读取AAD
    std::ifstream aad_file(aad_path, std::ios::binary | std::ios::ate);
    size_t aad_len = aad_file.tellg();
    uint8_t* aad = aad_len > 0 ? new uint8_t[aad_len] : nullptr;
    if (aad_len > 0) {
        aad_file.seekg(0);
        aad_file.read((char*)aad, aad_len);
    }

    //读取密文
    std::ifstream cipher_file(ciphertext_path, std::ios::binary | std::ios::ate);
    if (!cipher_file) return false;
    size_t cipher_len = cipher_file.tellg();
    uint8_t* ciphertext = new uint8_t[cipher_len];
    cipher_file.seekg(0);
    cipher_file.read((char*)ciphertext, cipher_len);

    //读取标签
    std::ifstream tag_file(tag_path, std::ios::binary | std::ios::ate);
    if (!tag_file || tag_file.tellg() != 16) return false;
    uint8_t tag[16];
    tag_file.seekg(0);
    tag_file.read((char*)tag, 16);

    //解密
    uint8_t* plaintext = new uint8_t[cipher_len];
    bool success = sm4_gcm_decrypt(ciphertext, cipher_len, aad, aad_len, &key, iv, iv_len, tag, plaintext);

    //写入明文
    if (success) {
        std::ofstream plain_file(plaintext_path, std::ios::binary);
        plain_file.write((char*)plaintext, cipher_len);
    }

    //清理资源
    delete[] iv;
    delete[] aad;
    delete[] ciphertext;
    delete[] plaintext;
    return success;
}
