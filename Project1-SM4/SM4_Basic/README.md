# SM4算法基础实现

## 一、实验目的

1. 深入理解SM4分组密码算法的原理及国家标准（GB/T32907-2016）中规定的技术细节。
2. 基于C++语言实现SM4算法的基础加密、解密功能，包括密钥扩展、轮函数运算等核心模块。
3. 验证算法实现的正确性，通过标准测试案例及文件加密/解密实验验证功能完整性。
4. 掌握分组密码的核心原理、基本工作模式及实现方法。

## 二、实验环境

- **操作系统**：Windows 11（64位）
- **编程语言**：C++
- **开发工具**：Visual Studio 2022
- **测试工具**：Hex Edit（验证十六进制数据块格式，验证加密前后数据一致性）

## 三、算法原理概述

SM4是我国自主设计的分组密码算法，属于非平衡Feistel结构，其核心技术细节参考 GB/T32907-2016《信息安全技术 SM4分组密码算法》文献所定义的算法规范。

### 3.1 核心参数

- **分组长度**：128比特（4个32比特字）
- **密钥长度**：128比特（4个32比特字）
- **轮数**：32轮
- **轮密钥**：32个32比特字（由密钥扩展算法生成）

### 3.2 算法结构

1. **加密流程**：  
   输入128比特明文，经32轮迭代运算后，通过反序变换输出128比特密文。  
   - 迭代运算：每轮使用轮函数\( F \)对中间状态进行变换，公式为：  
     \[
     X_{i+4} = X_i \oplus T(X_{i+1} \oplus X_{i+2} \oplus X_{i+3} \oplus rk_i)
     \]  
     其中\( rk_i \)为第\( i \)轮轮密钥，\( T \)为合成置换。  
   - 反序变换：将32轮迭代后的状态\( (X_{32}, X_{33}, X_{34}, X_{35}) \)反序为\( (X_{35}, X_{34}, X_{33}, X_{32}) \)，作为密文输出。

2. **解密流程**：  
   结构与加密完全一致，仅轮密钥使用顺序相反（即\( rk_{31}, rk_{30}, ..., rk_0 \)）。

3. **合成置换\( T \)**：  
   由非线性变换\( \tau \)和线性变换\( L \)复合而成：  
   - 非线性变换\( \tau \)：将32比特输入拆分为4个8比特字节，分别通过S盒（8比特输入→8比特输出的固定置换）替换，公式为：  
     \[
     \tau(A) = (Sbox(a_0), Sbox(a_1), Sbox(a_2), Sbox(a_3)) \quad (A=(a_0,a_1,a_2,a_3))
     \]  
   - 线性变换\( L \)：对\( \tau \)的输出进行循环左移与异或运算，公式为：  
     \[
     L(B) = B \oplus (B \lll 2) \oplus (B \lll 10) \oplus (B \lll 18) \oplus (B \lll 24)
     \]  
     其中\( \lll i \)表示32位循环左移\( i \)位。

4. **密钥扩展算法**：  
   由128比特主密钥生成32轮轮密钥，步骤为：  
   - 初始化：\( K_0 = MK_0 \oplus FK_0, \, K_1 = MK_1 \oplus FK_1, \, K_2 = MK_2 \oplus FK_2, \, K_3 = MK_3 \oplus FK_3 \)（\( FK \)为系统参数）。  
   - 轮密钥生成：\( rk_i = K_{i+4} = K_i \oplus T'(K_{i+1} \oplus K_{i+2} \oplus K_{i+3} \oplus CK_i) \)，其中\( T' \)为密钥扩展专用合成置换（线性变换为\( L'(B) = B \oplus (B \lll 13) \oplus (B \lll 23) \)，\( CK \)为固定参数）。

## 四、代码实现细节

### 4.1 核心数据结构

- **S盒**：依据GB/T32907-2016定义的256字节置换表，存储于`SBOX[256]`数组。
- **系统参数**：`FK[4]`（固定值：`0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC`）。
- **固定参数**：`CK[32]`（32个32比特轮常量，由公式\( ck_{i,j} = (4i + j) \times 7 \mod 256 \)生成）。

### 4.2 核心函数实现

1. **非线性变换\( \tau \)**：  
   将32比特输入拆分为4个字节，通过S盒替换后重组，需注意小端存储系统的字节顺序调整：
   ```cpp
   static uint32_t tau(uint32_t x) {
       uint8_t* bytes = reinterpret_cast<uint8_t*>(&x);
       uint32_t result = 0;
       result |= (uint32_t)SBOX[bytes[3]] << 24;  //高8位
       result |= (uint32_t)SBOX[bytes[2]] << 16;
       result |= (uint32_t)SBOX[bytes[1]] << 8;
       result |= (uint32_t)SBOX[bytes[0]];        //低8位
       return result;
   }
   ```

2. **线性变换\( L \)和\( L' \)**：  
   实现循环左移与异或运算：
   ```cpp
   static uint32_t L(uint32_t x) {
       return x ^ ROL(x, 2) ^ ROL(x, 10) ^ ROL(x, 18) ^ ROL(x, 24);
   }
   static uint32_t L_prime(uint32_t x) {
       return x ^ ROL(x, 13) ^ ROL(x, 23);
   }
   ```
   其中`ROL`为32位循环左移宏定义：`#define ROL(x, n) ((x) << (n) | (x) >> (32 - (n)))`。

3. **加密/解密函数**：  
   - 加密：生成轮密钥后执行32轮迭代，最后反序输出：
     ```cpp
     void sm4_encrypt(const uint32_t plaintext[4], const uint32_t key[4], uint32_t ciphertext[4]) {
         uint32_t rk[32];
         generate_round_keys(key, rk);  //生成轮密钥
         uint32_t X[36];
         X[0] = plaintext[0]; X[1] = plaintext[1]; X[2] = plaintext[2]; X[3] = plaintext[3];
         for (int i = 0; i < 32; ++i) {
             X[i + 4] = X[i] ^ T(X[i + 1] ^ X[i + 2] ^ X[i + 3] ^ rk[i]);
         }
         ciphertext[0] = X[35]; ciphertext[1] = X[34];  //反序变换
         ciphertext[2] = X[33]; ciphertext[3] = X[32];
     }
     ```
   - 解密：复用加密逻辑，仅轮密钥逆序：
     ```cpp
     void sm4_decrypt(...) {
         ...
         uint32_t rk_rev[32];
         for (int i = 0; i < 32; ++i) rk_rev[i] = rk[31 - i];  //轮密钥逆序
         ...
     }
     ```

4. **文件加密/解密**：  
   采用ECB模式（电子密码本模式），按128比特块分块处理文件：
   ```cpp
   bool encrypt_file(...) {
       ...
       uint32_t plaintext_block[4], ciphertext_block[4];
       while (read_block(plaintext_file, plaintext_block)) {  //读取16字节块
           sm4_encrypt(plaintext_block, key, ciphertext_block);
           write_block(ciphertext_file, ciphertext_block);    //写入16字节密文块
       }
       ...
   }
   ```

## 五、实验结果与分析

### 5.1 功能正确性验证

使用GB/T32907-2016附录A中的标准测试案例验证：  
- **输入明文**：`0123456789ABCDEFFEDCBA9876543210`（16字节）  
- **输入密钥**：`0123456789ABCDEFFEDCBA9876543210`（16字节）  
- **预期密文**：`681EDF34D206965E86B3E94F536E4246`（16字节）  

实验结果：加密输出与预期密文完全一致，验证了基础算法的正确性。

### 5.2 多轮加密验证

对同一明文使用相同密钥连续加密1,000,000次，结果与标准示例2一致（密文为`595298C7C6FD271F0402F804C33D3F66`），证明轮函数迭代及密钥扩展的稳定性。

### 5.3 文件加密测试

对1MB文本文件进行加密和解密操作，解密后文件与原文件完全一致（MD5校验值相同），验证了文件处理逻辑的正确性。

### 5.4 性能测试结果

通过性能测试程序验证，基础SM4实现在不同数据大小下的性能表现：

| 数据大小 | 平均用时(ms) | 吞吐量(MB/s) |
|---------|-------------|-------------|
| 1KB     | 0.12        | 8.33        |
| 10KB    | 1.15        | 8.70        |
| 100KB   | 11.45       | 8.73        |
| 1MB     | 114.67      | 8.93        |

### 5.5 局限性分析

1. **工作模式**：本实现采用ECB模式，相同明文块加密后输出相同密文块，安全性较低，实际应用中需采用CBC、CTR等更安全的模式。
2. **性能**：基础实现未采用T-Table、SIMD等优化技术，加密速率约为8-9 MB/s，低于优化版本（如采用AES-NI指令可提升至20+ MB/s）。

## 六、问题与解决方案

1. **字节顺序问题**：小端存储系统中，32位字的字节顺序与算法定义的大端格式冲突，通过`read_block`和`write_block`函数中的字节重组解决。
2. **轮密钥生成错误**：初始实现中误将`CK`参数按字节处理，修正为32位字直接参与运算后解决。
3. **文件块读取不完整**：通过`file.gcount()`判断实际读取字节数，避免文件大小非16字节倍数时的错误。

## 七、实验结论

本实验基于GB/T32907-2016标准实现了SM4分组密码算法的基础功能，包括加密、解密、密钥扩展及文件处理。通过标准测试案例验证了算法的正确性，明确了基础实现的性能局限性及改进方向（如优化工作模式、采用硬件加速指令）。实验结果表明，SM4算法的基础实现能够满足商用密码产品的功能需求，为后续优化与应用奠定了基础。

## 八、项目文件结构

```
SM4_Basic/
├── SM4/
│   ├── main.cpp          # 主程序入口
│   ├── SM4.cpp           # SM4算法实现
│   ├── SM4.h             # 头文件声明
│   └── SM4.vcxproj       # Visual Studio项目文件
├── SM4_Basic.sln         # 解决方案文件
├── x64/
│   └── Debug/
│       └── SM4_Basic.exe # 编译后的可执行文件
└── README.md             # 实验报告
```

## 九、使用方法

### 编译项目
1. 使用Visual Studio打开`SM4_Basic.sln`
2. 选择Release配置和x64平台
3. 编译项目

### 运行程序
```bash
# 加密文件
./SM4_Basic.exe -e input.txt（路径） key.txt（路径） output.txt（路径）

# 解密文件
./SM4_Basic.exe -d output.txt（路径） key.txt（路径） decrypted.txt（路径）
```

## 参考文献

1. GB/T32907-2016，《信息安全技术 SM4分组密码算法》
2. Long Wen, 《Fast Software Implementation of Crypto Primitives: A Survey Of AES & SM4 Implementation》(2025)
3. SM4算法官方文档及测试向量
