#ifndef SM4_AESNI_H
#define SM4_AESNI_H

#include <cstdint>
#include <string>
#include <immintrin.h>  

//定义S盒、系统参数FJ和固定参数CK
extern const uint8_t SBOX[256];
extern const uint32_t FK[4];
extern const uint32_t CK[32];

//密钥扩展函数
void SM4_keyexpansion(const uint32_t key[4], uint32_t rk[32]);

//单块加密解密核心函数
int SM4_AESNI_encblock(const uint8_t* plaintext, uint8_t* ciphertext, const uint32_t* round_keys);
int SM4_AESNI_decblock(const uint8_t* ciphertext, uint8_t* plaintext, const uint32_t* round_keys);

//4块并行加密函数
int SM4_AESNI_enc4blocks(const uint8_t* plaintext, uint8_t* ciphertext, const uint32_t* round_keys);

//SM4_AESNI加密解密函数
int SM4_AESNI_enc(const uint8_t* plaintext, uint8_t* ciphertext, size_t length, const uint32_t* round_keys);
int SM4_AESNI_dec(const uint8_t* ciphertext, uint8_t* plaintext, size_t length, const uint32_t* round_keys);

//文件加密解密函数
bool encrypt_file(const std::string& plaintext_path, const std::string& key_path, const std::string& ciphertext_path);
bool decrypt_file(const std::string& ciphertext_path, const std::string& key_path, const std::string& plaintext_path);

#endif //SM4_AESNI_H