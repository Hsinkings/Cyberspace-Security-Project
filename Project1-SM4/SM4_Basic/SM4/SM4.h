#ifndef SM4_H
#define SM4_H

#include <cstdint>
#include <fstream>
#include <string>

//SM4算法核心组件定义
//S盒（字节替换非线性变换表）
extern const uint8_t SBOX[256];

//系统参数FK用于密钥扩展的固定参数
extern const uint32_t FK[4];

//固定参数CK用于密钥扩展的轮常量
extern const uint32_t CK[32];

//32位循环左移
#define ROL(x, n) ((x) << (n) | (x) >> (32 - (n)))

//SM4加密函数
//输入：明文数据块(128位)、加密密钥(128位)
//输出：密文数据块(128位)
void sm4_encrypt(const uint32_t plaintext[4], const uint32_t key[4], uint32_t ciphertext[4]);

//SM4解密函数
//输入：密文数据块(128位)、解密密钥(128位)
//输出：明文数据块(128位)
void sm4_decrypt(const uint32_t ciphertext[4], const uint32_t key[4], uint32_t plaintext[4]);

//文件整体SM4加密函数
//输入：明文文件路径、密钥文件路径
//输出：密文文件路径
bool encrypt_file(const std::string& plaintext_path, const std::string& key_path, const std::string& ciphertext_path);

//文件整体SM4解密函数
//输入：密文文件路径、密钥文件路径
//输出：明文文件路径
bool decrypt_file(const std::string& ciphertext_path, const std::string& key_path, const std::string& plaintext_path);

#endif //SM4_H