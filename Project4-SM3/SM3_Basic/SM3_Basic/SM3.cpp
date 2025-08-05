#include "SM3.h"
#include <cstring>
#include <algorithm>

/**
 * SM3哈希算法初始向量（IV）
 * 根据GB/T 32905-2016标准定义的8个32位初始值
 * 这些值经过精心设计，确保算法的安全性和随机性
 */
const uint32_t IV[8] = {
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

/**
 * SM3构造函数
 * 初始化哈希状态变量，设置初始向量值
 * 重置所有计数器和缓冲区
 */
SM3::SM3() {
    //将初始向量值复制到状态变量中
    std::memcpy(&A, &IV[0], 4);
    std::memcpy(&B, &IV[1], 4);
    std::memcpy(&C, &IV[2], 4);
    std::memcpy(&D, &IV[3], 4);
    std::memcpy(&E, &IV[4], 4);
    std::memcpy(&F, &IV[5], 4);
    std::memcpy(&G, &IV[6], 4);
    std::memcpy(&H, &IV[7], 4);
    
    //初始化计数器和缓冲区
    total_len = 0;      //已处理的总比特数
    buffer_len = 0;     //当前缓冲区中的字节数
    std::memset(buffer, 0, 64);  //清空缓冲区
}

/**
 * 32位循环左移函数
 * 实现原理：(x << n) | (x >> (32 - n))
 * 确保移出的位从右侧重新进入
 */
uint32_t SM3::ROTL32(uint32_t x, uint8_t n) {
    return (x << n) | (x >> (32 - n));
}

/**
 * 置换函数P1
 * 公式：P1(x) = x ⊕ (x <<< 15) ⊕ (x <<< 23)
 * 用于消息扩展阶段，增加非线性性
 */
uint32_t SM3::P1(uint32_t x) {
    return x ^ ROTL32(x, 15) ^ ROTL32(x, 23);
}

/**
 * 置换函数P0
 * 公式：P0(x) = x ⊕ (x <<< 9) ⊕ (x <<< 17)
 * 用于压缩函数中，增加状态更新的非线性性
 */
uint32_t SM3::P0(uint32_t x) {
    return x ^ ROTL32(x, 9) ^ ROTL32(x, 17);
}

/**
 * 布尔函数FF
 * 前16轮：FF(x,y,z) = x ⊕ y ⊕ z
 * 后48轮：FF(x,y,z) = (x ∧ y) ∨ (x ∧ z) ∨ (y ∧ z)
 * 提供非线性变换
 */
uint32_t SM3::FF(uint32_t x, uint32_t y, uint32_t z, uint8_t j) {
    if (j <= 15) return x ^ y ^ z;
    else return (x & y) | (x & z) | (y & z);
}

/**
 * 布尔函数GG
 * 前16轮：GG(x,y,z) = x ⊕ y ⊕ z
 * 后48轮：GG(x,y,z) = (x ∧ y) ∨ (¬x ∧ z)
 * 提供非线性变换
 */
uint32_t SM3::GG(uint32_t x, uint32_t y, uint32_t z, uint8_t j) {
    if (j <= 15) return x ^ y ^ z;
    else return (x & y) | (~x & z);
}

/**
 * 常量T函数
 * 前16轮：T(j) = 0x79CC4519
 * 后48轮：T(j) = 0x7A879D8A
 * 这些常量经过精心选择，确保算法的安全性
 */
uint32_t SM3::T(uint8_t j) {
    if (j <= 15) return 0x79CC4519;
    else return 0x7A879D8A;
}

/**
 * SM3压缩函数
 * 压缩函数将512位输入块压缩为256位输出
 * 包含两个主要步骤：消息扩展和状态压缩
 */
void SM3::compress(const uint8_t* block) {
    //第一步：消息扩展
    //将512位（64字节）输入扩展为2048位（256字节）
    uint32_t W[68], W1[64];  //W数组用于存储扩展后的消息字
    uint32_t* p = (uint32_t*)block;  //将字节数组转换为32位字数组

    //前16个字：直接复制输入块，并进行字节序转换
    //由于大多数系统使用小端序，需要转换为大端序
    for (int i = 0; i < 16; i++) {
        W[i] = (p[i] >> 24) | ((p[i] >> 8) & 0x0000FF00) |
            ((p[i] << 8) & 0x00FF0000) | (p[i] << 24);
    }

    //后52个字：使用线性反馈移位寄存器生成
    //公式：W[j] = P1(W[j-16] ⊕ W[j-9] ⊕ (W[j-3] <<< 15)) ⊕ (W[j-13] <<< 7) ⊕ W[j-6]
    for (int j = 16; j < 68; j++) {
        W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTL32(W[j - 3], 15)) ^
            ROTL32(W[j - 13], 7) ^ W[j - 6];
    }

    //生成W'数组：W'[j] = W[j] ⊕ W[j+4]
    //用于增加消息扩展的复杂性
    for (int j = 0; j < 64; j++) {
        W1[j] = W[j] ^ W[j + 4];
    }

    //第二步：状态压缩
    //使用64轮迭代更新8个状态变量
    uint32_t a = A, b = B, c = C, d = D;
    uint32_t e = E, f = F, g = G, h = H;
    uint32_t SS1, SS2, TT1, TT2;

    //64轮迭代，每轮更新所有状态变量
    for (int j = 0; j < 64; j++) {
        //计算中间变量SS1和SS2
        SS1 = ROTL32(ROTL32(a, 12) + e + ROTL32(T(j), j), 7);
        SS2 = SS1 ^ ROTL32(a, 12);
        
        //计算中间变量TT1和TT2
        TT1 = FF(a, b, c, j) + d + SS2 + W1[j];
        TT2 = GG(e, f, g, j) + h + SS1 + W[j];
        
        //更新状态变量（类似Feistel结构）
        d = c;
        c = ROTL32(b, 9);
        b = a;
        a = TT1;
        h = g;
        g = ROTL32(f, 19);
        f = e;
        e = P0(TT2);
    }

    //将迭代结果与原始状态进行异或操作
    //这是Davies-Meyer结构的特征
    A ^= a;
    B ^= b;
    C ^= c;
    D ^= d;
    E ^= e;
    F ^= f;
    G ^= g;
    H ^= h;
}

/**
 * 更新函数 - 处理输入数据
 * 将输入数据分块处理，每64字节调用一次压缩函数
 * 支持任意长度的输入数据
 */
void SM3::update(const uint8_t* data, size_t len) {
    total_len += len * 8;  //累计已处理的比特数（用于最终长度填充）
    
    while (len > 0) {
        //计算当前缓冲区还能容纳多少字节
        size_t fill = 64 - buffer_len;
        //确定本次要复制的字节数
        size_t copy = std::min(len, fill);
        
        //将数据复制到缓冲区
        std::memcpy(buffer + buffer_len, data, copy);
        buffer_len += copy;
        data += copy;
        len -= copy;

        //如果缓冲区满了（64字节），调用压缩函数处理
        if (buffer_len == 64) {
            compress(buffer);
            buffer_len = 0;  //重置缓冲区长度
            std::memset(buffer, 0, 64);  //清空缓冲区
        }
    }
}

/**
 * 字符串版本的更新函数
 * 将字符串转换为字节数组后调用字节版本
 */
void SM3::update(const std::string& data) {
    update((const uint8_t*)data.c_str(), data.size());
}

/**
 * 完成哈希计算并返回结果
 * 执行Merkle-Damgård填充，处理最后一个数据块
 * 返回最终的256位哈希值
 */
std::vector<uint8_t> SM3::final() {
    //步骤1：添加填充位0x80（10000000）
    //这是Merkle-Damgård填充的标准做法
    buffer[buffer_len++] = 0x80;

    //步骤2：处理长度字段
    //如果剩余空间不足8字节（64位长度字段），需要额外的填充块
    if (buffer_len > 56) {
        //填充当前块到64字节
        while (buffer_len < 64) buffer[buffer_len++] = 0x00;
        compress(buffer);  //压缩当前块
        buffer_len = 0;
        std::memset(buffer, 0, 64);  //准备下一个块
    }

    //步骤3：填充0直到56字节位置
    //为64位长度字段预留空间
    while (buffer_len < 56) buffer[buffer_len++] = 0x00;

    //步骤4：添加64位消息长度（大端序）
    //这是Merkle-Damgård构造的关键，防止长度扩展攻击
    uint64_t len = total_len;
    for (int i = 0; i < 8; i++) {
        buffer[56 + i] = (len >> (8 * (7 - i))) & 0xFF;
    }
    
    //步骤5：压缩最后一个块
    compress(buffer);

    //步骤6：构造最终结果
    //将8个32位状态变量转换为32字节输出（大端序）
    std::vector<uint8_t> result(32);
    uint32_t* hash = (uint32_t*)result.data();
    
    //字节序转换：内部状态（小端）-> 输出（大端）
    hash[0] = (A >> 24) | ((A >> 8) & 0x0000FF00) | ((A << 8) & 0x00FF0000) | (A << 24);
    hash[1] = (B >> 24) | ((B >> 8) & 0x0000FF00) | ((B << 8) & 0x00FF0000) | (B << 24);
    hash[2] = (C >> 24) | ((C >> 8) & 0x0000FF00) | ((C << 8) & 0x00FF0000) | (C << 24);
    hash[3] = (D >> 24) | ((D >> 8) & 0x0000FF00) | ((D << 8) & 0x00FF0000) | (D << 24);
    hash[4] = (E >> 24) | ((E >> 8) & 0x0000FF00) | ((E << 8) & 0x00FF0000) | (E << 24);
    hash[5] = (F >> 24) | ((F >> 8) & 0x0000FF00) | ((F << 8) & 0x00FF0000) | (F << 24);
    hash[6] = (G >> 24) | ((G >> 8) & 0x0000FF00) | ((G << 8) & 0x00FF0000) | (G << 24);
    hash[7] = (H >> 24) | ((H >> 8) & 0x0000FF00) | ((H << 8) & 0x00FF0000) | (H << 24);

    return result;
}

/**
 * 静态哈希函数
 * 创建SM3实例，处理输入数据，返回哈希结果
 * 适用于一次性哈希计算
 */
std::vector<uint8_t> SM3::hash(const std::string& data) {
    SM3 sm3;
    sm3.update(data);
    return sm3.final();
}

//批量哈希处理函数（用于性能测试）
void SM3::batch_hash(const std::string& data, std::vector<uint8_t>& digest, int iterations) {
    if (iterations <= 0) return;
    SM3 sm3;
    for (int i = 0; i < iterations; ++i) {
        sm3.update(data);
        digest = sm3.final();
        if (i != iterations - 1) {
            sm3 = SM3(); //重置上下文
        }
    }
}