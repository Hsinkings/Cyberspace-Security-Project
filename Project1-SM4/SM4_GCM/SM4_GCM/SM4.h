#ifndef SM4_GCM_H
#define SM4_GCM_H

#include <cstdint>
#include <string>
#include <immintrin.h>  //用于SIMD指令

//128位数据块结构
typedef struct {
    uint64_t high;  //高64位
    uint64_t low;   //低64位
} block128_t;

//SM4算法核心组件定义
extern const uint8_t SBOX[256];
extern const uint32_t FK[4];
extern const uint32_t CK[32];

//32位循环左移
#define ROL(x, n) ((x) << (n) | (x) >> (32 - (n)))

//128位块异或
#define XOR128(dst, a, b) \
    do { \
        (dst).high = (a).high ^ (b).high; \
        (dst).low = (a).low ^ (b).low; \
    } while(0)

//SM4单块加密（用于CTR和生成H）
void sm4_encrypt_block(const block128_t* plaintext, const block128_t* key, block128_t* ciphertext);

//GCM模式加密（同时提供机密性和完整性）
//参数：明文、明文长度、AAD、AAD长度、密钥、IV、IV长度、密文、标签(16字节)
bool sm4_gcm_encrypt(const uint8_t* plaintext, size_t plaintext_len,
    const uint8_t* aad, size_t aad_len,
    const block128_t* key,
    const uint8_t* iv, size_t iv_len,
    uint8_t* ciphertext, uint8_t* tag);

//GCM模式解密（验证完整性并解密）
bool sm4_gcm_decrypt(const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t* aad, size_t aad_len,
    const block128_t* key,
    const uint8_t* iv, size_t iv_len,
    const uint8_t* tag, uint8_t* plaintext);

//文件GCM加密
bool encrypt_file_gcm(const std::string& plaintext_path, const std::string& key_path,
    const std::string& iv_path, const std::string& aad_path,
    const std::string& ciphertext_path, const std::string& tag_path);

//文件GCM解密
bool decrypt_file_gcm(const std::string& ciphertext_path, const std::string& key_path,
    const std::string& iv_path, const std::string& aad_path,
    const std::string& tag_path, const std::string& plaintext_path);

#endif //SM4_GCM_H
