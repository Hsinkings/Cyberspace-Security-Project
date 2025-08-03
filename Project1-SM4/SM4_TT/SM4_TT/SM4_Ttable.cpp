#include "SM4_Ttable.h"
#include <cstring>
#include <iostream>

//S盒定义（依据GB/T32907-2016标准）
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

//系统参数FK（依据GB/T32907-2016标准）
const uint32_t FK[4] = {
    0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC
};

//固定参数CK（依据GB/T32907-2016标准）
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

//T表优化（预计算查找表）
static uint32_t T0[256];
static uint32_t T1[256];
static uint32_t T2[256];
static uint32_t T3[256];
static uint32_t TK0[256];
static uint32_t TK1[256];
static uint32_t TK2[256];
static uint32_t TK3[256];
static int tables_initialized = 0;

//初始化T表（结合S盒替换和线性变换）
static void init_ttables() {
    if (tables_initialized) return;

    for (int i = 0; i < 256; ++i) {
        uint8_t s = SBOX[i];

        //T表加密/解密（T = L∘τ）
        uint32_t s0 = (uint32_t)s << 24;
        uint32_t s1 = (uint32_t)s << 16;
        uint32_t s2 = (uint32_t)s << 8;
        uint32_t s3 = (uint32_t)s;

        uint32_t L_s0 = s0 ^ ROL(s0, 2) ^ ROL(s0, 10) ^ ROL(s0, 18) ^ ROL(s0, 24);
        uint32_t L_s1 = s1 ^ ROL(s1, 2) ^ ROL(s1, 10) ^ ROL(s1, 18) ^ ROL(s1, 24);
        uint32_t L_s2 = s2 ^ ROL(s2, 2) ^ ROL(s2, 10) ^ ROL(s2, 18) ^ ROL(s2, 24);
        uint32_t L_s3 = s3 ^ ROL(s3, 2) ^ ROL(s3, 10) ^ ROL(s3, 18) ^ ROL(s3, 24);

        T0[i] = L_s0;
        T1[i] = L_s1;
        T2[i] = L_s2;
        T3[i] = L_s3;

        //T'表密钥扩展（T' = L'∘τ）
        uint32_t sk0 = (uint32_t)s << 24;
        uint32_t sk1 = (uint32_t)s << 16;
        uint32_t sk2 = (uint32_t)s << 8;
        uint32_t sk3 = (uint32_t)s;

        uint32_t L_sk0 = sk0 ^ ROL(sk0, 13) ^ ROL(sk0, 23);
        uint32_t L_sk1 = sk1 ^ ROL(sk1, 13) ^ ROL(sk1, 23);
        uint32_t L_sk2 = sk2 ^ ROL(sk2, 13) ^ ROL(sk2, 23);
        uint32_t L_sk3 = sk3 ^ ROL(sk3, 13) ^ ROL(sk3, 23);

        TK0[i] = L_sk0;
        TK1[i] = L_sk1;
        TK2[i] = L_sk2;
        TK3[i] = L_sk3;
    }

    tables_initialized = 1;
}

//基于T表的复合变换T
static uint32_t sm4_T_ttable(uint32_t x) {
    return T0[(x >> 24) & 0xFF] ^
        T1[(x >> 16) & 0xFF] ^
        T2[(x >> 8) & 0xFF] ^
        T3[x & 0xFF];
}

//基于T表的密钥扩展复合变换T'
static uint32_t sm4_T_prime_ttable(uint32_t x) {
    return TK0[(x >> 24) & 0xFF] ^
        TK1[(x >> 16) & 0xFF] ^
        TK2[(x >> 8) & 0xFF] ^
        TK3[x & 0xFF];
}

//密钥扩展算法（使用T表优化）
static void generate_round_keys(const uint32_t key[4], uint32_t rk[32]) {
    init_ttables();
    uint32_t K[4];

    //初始化密钥
    K[0] = key[0] ^ FK[0];
    K[1] = key[1] ^ FK[1];
    K[2] = key[2] ^ FK[2];
    K[3] = key[3] ^ FK[3];

    //生成32轮轮密钥
    for (int i = 0; i < 32; ++i) {
        uint32_t temp = K[(i + 1) % 4] ^ K[(i + 2) % 4] ^ K[(i + 3) % 4] ^ CK[i];
        K[(i + 4) % 4] = K[i % 4] ^ sm4_T_prime_ttable(temp);
        rk[i] = K[(i + 4) % 4];
    }
}

//SM4加密算法（T表优化）
void sm4_encrypt(const uint32_t plaintext[4], const uint32_t key[4], uint32_t ciphertext[4]) {
    uint32_t rk[32];
    generate_round_keys(key, rk);
    init_ttables();

    uint32_t X[4] = { plaintext[0], plaintext[1], plaintext[2], plaintext[3] };

    //32轮Feistel变换
    for (int i = 0; i < 32; ++i) {
        uint32_t temp = X[1] ^ X[2] ^ X[3] ^ rk[i];
        uint32_t newX0 = X[0] ^ sm4_T_ttable(temp);

        //移位更新
        X[0] = X[1];
        X[1] = X[2];
        X[2] = X[3];
        X[3] = newX0;
    }

    //输出置换
    ciphertext[0] = X[3];
    ciphertext[1] = X[2];
    ciphertext[2] = X[1];
    ciphertext[3] = X[0];
}

//SM4解密算法（T表优化）
void sm4_decrypt(const uint32_t ciphertext[4], const uint32_t key[4], uint32_t plaintext[4]) {
    uint32_t rk[32];
    generate_round_keys(key, rk);
    init_ttables();

    //解密使用反向轮密钥
    uint32_t rk_rev[32];
    for (int i = 0; i < 32; ++i) {
        rk_rev[i] = rk[31 - i];
    }

    uint32_t X[4] = { ciphertext[0], ciphertext[1], ciphertext[2], ciphertext[3] };

    //32轮Feistel变换
    for (int i = 0; i < 32; ++i) {
        uint32_t temp = X[1] ^ X[2] ^ X[3] ^ rk_rev[i];
        uint32_t newX0 = X[0] ^ sm4_T_ttable(temp);

        //移位更新
        X[0] = X[1];
        X[1] = X[2];
        X[2] = X[3];
        X[3] = newX0;
    }

    //输出置换
    plaintext[0] = X[3];
    plaintext[1] = X[2];
    plaintext[2] = X[1];
    plaintext[3] = X[0];
}

//读取128位数据块（4个32位字）
static bool read_block(std::ifstream& file, uint32_t block[4]) {
    uint8_t buffer[16];
    file.read(reinterpret_cast<char*>(buffer), 16);
    if (file.gcount() != 16) {
        return false;  //读取失败或文件结束
    }

    //字节转32位字（大端格式）
    for (int i = 0; i < 4; ++i) {
        block[i] = (uint32_t)buffer[i * 4] << 24 |
            (uint32_t)buffer[i * 4 + 1] << 16 |
            (uint32_t)buffer[i * 4 + 2] << 8 |
            (uint32_t)buffer[i * 4 + 3];
    }
    return true;
}

//写入128位数据块（4个32位字）
static void write_block(std::ofstream& file, const uint32_t block[4]) {
    uint8_t buffer[16];
    //32位字转字节（大端格式）
    for (int i = 0; i < 4; ++i) {
        buffer[i * 4] = (block[i] >> 24) & 0xFF;
        buffer[i * 4 + 1] = (block[i] >> 16) & 0xFF;
        buffer[i * 4 + 2] = (block[i] >> 8) & 0xFF;
        buffer[i * 4 + 3] = block[i] & 0xFF;
    }
    file.write(reinterpret_cast<char*>(buffer), 16);
}

//文件加密
bool encrypt_file(const std::string& plaintext_path, const std::string& key_path, const std::string& ciphertext_path) {
    //读取密钥文件（16字节）
    std::ifstream key_file(key_path, std::ios::binary);
    if (!key_file) {
        std::cerr << "无法打开密钥文件: " << key_path << std::endl;
        return false;
    }

    uint32_t key[4];
    if (!read_block(key_file, key)) {
        std::cerr << "密钥文件格式错误，需要16字节" << std::endl;
        return false;
    }

    //打开明文和密文文件
    std::ifstream plaintext_file(plaintext_path, std::ios::binary);
    std::ofstream ciphertext_file(ciphertext_path, std::ios::binary);
    if (!plaintext_file || !ciphertext_file) {
        std::cerr << "无法打开文件" << std::endl;
        return false;
    }

    //分块加密
    uint32_t plaintext_block[4];
    uint32_t ciphertext_block[4];
    while (read_block(plaintext_file, plaintext_block)) {
        sm4_encrypt(plaintext_block, key, ciphertext_block);
        write_block(ciphertext_file, ciphertext_block);
    }

    return true;
}

//文件解密
bool decrypt_file(const std::string& ciphertext_path, const std::string& key_path, const std::string& plaintext_path) {
    //读取密钥文件（16字节）
    std::ifstream key_file(key_path, std::ios::binary);
    if (!key_file) {
        std::cerr << "无法打开密钥文件: " << key_path << std::endl;
        return false;
    }

    uint32_t key[4];
    if (!read_block(key_file, key)) {
        std::cerr << "密钥文件格式错误，需要16字节" << std::endl;
        return false;
    }

    //打开密文和明文文件
    std::ifstream ciphertext_file(ciphertext_path, std::ios::binary);
    std::ofstream plaintext_file(plaintext_path, std::ios::binary);
    if (!ciphertext_file || !plaintext_file) {
        std::cerr << "无法打开文件" << std::endl;
        return false;
    }

    //分块解密
    uint32_t ciphertext_block[4];
    uint32_t plaintext_block[4];
    while (read_block(ciphertext_file, ciphertext_block)) {
        sm4_decrypt(ciphertext_block, key, plaintext_block);
        write_block(plaintext_file, plaintext_block);
    }

    return true;
}