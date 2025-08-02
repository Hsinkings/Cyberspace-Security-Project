#ifndef SM4_TTABLE_H
#define SM4_TTABLE_H

#include <cstdint>
#include <fstream>
#include <string>

//SM4算法常量定义
extern const uint8_t SBOX[256];               //S盒标准定义
extern const uint32_t FK[4];                  //系统参数FK
extern const uint32_t CK[32];                 //固定参数CK

//32位循环左移宏
#define ROL(x, n) ((x) << (n) | (x) >> (32 - (n)))

//SM4加密函数（T-Table优化）
void sm4_encrypt(const uint32_t plaintext[4], const uint32_t key[4], uint32_t ciphertext[4]);

//SM4解密函数（T-Table优化）
void sm4_decrypt(const uint32_t ciphertext[4], const uint32_t key[4], uint32_t plaintext[4]);

//文件加密/解密函数
bool encrypt_file(const std::string& plaintext_path, const std::string& key_path, const std::string& ciphertext_path);
bool decrypt_file(const std::string& ciphertext_path, const std::string& key_path, const std::string& plaintext_path);

#endif //SM4_TTABLE_H
