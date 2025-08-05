#include "SM3_Unrol.h"
#include <cstring>
#include <algorithm>

/**
 * SM3哈希算法初始向量（IV）- 优化版本
 * 根据GB/T 32905-2016标准定义的8个32位初始值
 * 使用常量数组存储，便于编译器优化
 */
const uint32_t SM3_IV[SM3_STATE_WORDS] = {
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

/**
 * SM3构造函数 - 优化版本
 * 直接初始化状态变量，避免memcpy函数调用开销
 * 提升构造函数的执行效率
 */
SM3::SM3() {
    //直接赋值，避免函数调用开销
    //编译器可以更好地优化这种直接赋值操作
    A = SM3_IV[0];
    B = SM3_IV[1];
    C = SM3_IV[2];
    D = SM3_IV[3];
    E = SM3_IV[4];
    F = SM3_IV[5];
    G = SM3_IV[6];
    H = SM3_IV[7];
    
    //初始化计数器和缓冲区
    total_bits = 0;     //已处理的总比特数
    buffer_len = 0;     //当前缓冲区中的字节数
    std::memset(buffer, 0, SM3_BLOCK_SIZE);  //清空缓冲区
}

/**
 * 压缩函数优化版本
 * 主要优化策略：
 * 1. 循环展开：减少循环控制开销
 * 2. 减少内存访问：使用局部变量缓存
 * 3. 内联函数：避免函数调用开销
 * 4. 字节序转换优化：使用专用函数
 */
void SM3::compress(const uint8_t* block) {
    //消息扩展数组：W[68]用于存储扩展后的消息字
    uint32_t W[68], W1[64];
    const uint32_t* p = reinterpret_cast<const uint32_t*>(block);

    //第一步：消息扩展（前16字）
    //优化字节序转换，使用专用函数减少重复代码
    for (int i = 0; i < 16; ++i) {
        W[i] = le_to_be32(p[i]);
    }

    //第二步：消息扩展（16-67字）
    //使用宏定义的P1函数，避免函数调用开销
    //公式：W[j] = P1(W[j-16] ⊕ W[j-9] ⊕ (W[j-3] <<< 15)) ⊕ (W[j-13] <<< 7) ⊕ W[j-6]
    for (int j = 16; j < 68; ++j) {
        W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTL32(W[j - 3], 15)) ^
            ROTL32(W[j - 13], 7) ^ W[j - 6];
    }

    //第三步：生成W'数组
    //W'[j] = W[j] ⊕ W[j+4]，增加消息扩展的复杂性
    for (int j = 0; j < 64; ++j) {
        W1[j] = W[j] ^ W[j + 4];
    }

    //第四步：状态压缩 - 核心优化区域
    //使用局部变量缓存状态，减少内存访问
    uint32_t a = A, b = B, c = C, d = D;
    uint32_t e = E, f = F, g = G, h = H;

    //64轮迭代：部分循环展开优化
    //每8轮展开一次，平衡代码大小和执行效率
    int j;
    for (j = 0; j < 64; ) {
        //轮j=0 - 使用宏定义的函数，避免函数调用开销
        uint32_t SS1 = ROTL32(ROTL32(a, 12) + e + ROTL32(T(j), j), 7);
        uint32_t SS2 = SS1 ^ ROTL32(a, 12);
        uint32_t TT1 = FF(a, b, c, j) + d + SS2 + W1[j];
        uint32_t TT2 = GG(e, f, g, j) + h + SS1 + W[j];
        //状态更新：使用紧凑的赋值形式
        d = c; c = ROTL32(b, 9); b = a; a = TT1;
        h = g; g = ROTL32(f, 19); f = e; e = P0(TT2);
        j++;

        //轮j=1 - 重复相同的计算模式
        SS1 = ROTL32(ROTL32(a, 12) + e + ROTL32(T(j), j), 7);
        SS2 = SS1 ^ ROTL32(a, 12);
        TT1 = FF(a, b, c, j) + d + SS2 + W1[j];
        TT2 = GG(e, f, g, j) + h + SS1 + W[j];
        d = c; c = ROTL32(b, 9); b = a; a = TT1;
        h = g; g = ROTL32(f, 19); f = e; e = P0(TT2);
        j++;

        //轮j=2 - 继续展开
        SS1 = ROTL32(ROTL32(a, 12) + e + ROTL32(T(j), j), 7);
        SS2 = SS1 ^ ROTL32(a, 12);
        TT1 = FF(a, b, c, j) + d + SS2 + W1[j];
        TT2 = GG(e, f, g, j) + h + SS1 + W[j];
        d = c; c = ROTL32(b, 9); b = a; a = TT1;
        h = g; g = ROTL32(f, 19); f = e; e = P0(TT2);
        j++;

        //轮j=3 - 完成当前展开块
        SS1 = ROTL32(ROTL32(a, 12) + e + ROTL32(T(j), j), 7);
        SS2 = SS1 ^ ROTL32(a, 12);
        TT1 = FF(a, b, c, j) + d + SS2 + W1[j];
        TT2 = GG(e, f, g, j) + h + SS1 + W[j];
        d = c; c = ROTL32(b, 9); b = a; a = TT1;
        h = g; g = ROTL32(f, 19); f = e; e = P0(TT2);
        j++;

        //注意：此处为简化展示，实际实现应展开完整64轮
        //完整的展开版本会包含16个类似的代码块，每块处理4轮
        //这样可以最大化减少循环控制开销
    }

    //第五步：更新状态变量
    //使用异或操作更新最终状态（Davies-Meyer结构）
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
 * 更新函数优化版本
 * 优化策略：
 * 1. 减少分支判断：优化数据流控制
 * 2. 批量处理：减少函数调用次数
 * 3. 内存访问优化：减少不必要的内存操作
 */
void SM3::update(const uint8_t* data, size_t len) {
    //快速路径：空数据直接返回
    if (len == 0) return;
    
    //累计已处理的比特数
    total_bits += len * 8;

    //第一步：处理缓冲区中已有数据
    //如果缓冲区不为空，优先填满当前块
    if (buffer_len > 0) {
        size_t fill = SM3_BLOCK_SIZE - buffer_len;
        if (len >= fill) {
            //填满当前块并压缩
            std::memcpy(buffer + buffer_len, data, fill);
            compress(buffer);
            data += fill;
            len -= fill;
            buffer_len = 0;
        }
        else {
            //数据不足以填满块，直接添加到缓冲区
            std::memcpy(buffer + buffer_len, data, len);
            buffer_len += len;
            return;
        }
    }

    //第二步：处理完整块
    //批量处理完整的数据块，减少函数调用开销
    while (len >= SM3_BLOCK_SIZE) {
        compress(data);
        data += SM3_BLOCK_SIZE;
        len -= SM3_BLOCK_SIZE;
    }

    //第三步：保存剩余数据
    //将不足一个块的数据保存到缓冲区
    if (len > 0) {
        std::memcpy(buffer, data, len);
        buffer_len = len;
    }
}

/**
 * 字符串版本的更新函数
 * 使用reinterpret_cast进行类型转换，提高类型安全性
 */
void SM3::update(const std::string& data) {
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

/**
 * 最终处理优化版本
 * 优化策略：
 * 1. 合并填充逻辑：减少条件判断
 * 2. 使用常量：避免魔法数字
 * 3. 优化内存操作：减少不必要的清零操作
 */
std::vector<uint8_t> SM3::final() {
    //步骤1：添加填充位0x80（10000000）
    //这是Merkle-Damgård填充的标准做法
    buffer[buffer_len++] = 0x80;

    //步骤2：处理填充和长度字段
    //优化填充逻辑，减少分支判断
    if (buffer_len > 56) {
        //情况1：剩余空间不足8字节，需要额外的填充块
        //填充当前块到64字节并压缩
        std::memset(buffer + buffer_len, 0, SM3_BLOCK_SIZE - buffer_len);
        compress(buffer);
        //准备下一个块，填充至56字节
        std::memset(buffer, 0, 56);
        buffer_len = 56;
    }
    else {
        //情况2：剩余空间足够，直接填充至56字节
        std::memset(buffer + buffer_len, 0, 56 - buffer_len);
        buffer_len = 56;
    }

    //步骤3：添加64位长度字段（大端序）
    //这是Merkle-Damgård构造的关键，防止长度扩展攻击
    uint64_t len = total_bits;
    for (int i = 0; i < 8; ++i) {
        buffer[56 + i] = static_cast<uint8_t>(len >> (8 * (7 - i)));
    }
    
    //步骤4：压缩最后一个块
    compress(buffer);

    //步骤5：构造最终结果
    //使用优化的字节序转换函数
    std::vector<uint8_t> result(SM3_DIGEST_SIZE);
    uint32_t* hash = reinterpret_cast<uint32_t*>(result.data());
    
    //将8个32位状态变量转换为32字节输出（大端序）
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

/**
 * 快捷哈希函数 - 便捷接口
 * 创建SM3实例，处理输入数据，返回哈希结果
 * 适用于一次性哈希计算
 */
std::vector<uint8_t> SM3::hash(const std::string& data) {
    SM3 sm3;
    sm3.update(data);
    return sm3.final();
}

/**
 * 批量哈希处理函数
 * 优化策略：
 * 1. 重用SM3实例：减少构造开销
 * 2. 引用传递：避免不必要的拷贝
 * 3. 条件重置：只在需要时重置状态
 * 
 * 适用于性能测试和批量哈希计算场景
 */
void SM3::batch_hash(const std::string& data, std::vector<uint8_t>& digest, int iterations) {
    if (iterations <= 0) return;
    
    //创建SM3实例
    SM3 sm3;
    
    //执行指定次数的哈希计算
    for (int i = 0; i < iterations; ++i) {
        sm3.update(data);
        digest = sm3.final();
        
        //优化：最后一次不需要重置
        if (i != iterations - 1) {
            sm3 = SM3();  //重置上下文
        }
    }
}