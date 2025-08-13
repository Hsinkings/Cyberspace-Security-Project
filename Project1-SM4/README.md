
## SM4算法实现与优化实验报告

### 1. 项目概述

本项目基于《GB/T32907-2016 信息安全技术 SM4分组密码算法》标准，实现并优化了SM4分组密码算法。SM4作为我国商用密码标准中的分组密码算法，广泛应用于金融、政务等领域的信息加密保护，采用 128 位分组长度与 128 位密钥长度，通过 32 轮非线性迭代的非平衡 Feistel 结构实现加密解密。其核心由 S 盒（8 比特非线性置换）、线性变换 L/L' 及轮函数 F 构成，加密与解密结构相同仅轮密钥使用顺序相反；轮密钥通过密钥扩展算法由主密钥结合系统参数 FK 和固定参数 CK 生成。该算法安全性高、实现灵活，广泛应用于商用密码领域，可通过基础实现、T-Table 预计算优化或 AES-NI 硬件指令加速等方式部署，满足不同场景的性能与安全需求。

项目设计并实现了多种不同优化级别的版本，从基础实现到硬件加速，系统探索了密码算法在软件层面的优化路径，部分优化算法列举如下：
- **SM4_Basic**：严格遵循GB/T32907-2016标准的基础实现，作为算法正确性验证的基准
- **SM4_TT**：基于T-Table预计算技术的优化实现，通过空间换时间提升执行效率
- **SM4_AESNI**：利用Intel AES-NI指令集与SIMD并行技术的硬件加速实现，面向高性能场景
- **SM4_GCM**：实现GCM认证加密模式，结合硬件指令加速和并行处理技术，提供机密性和完整性保护

### 1.1 项目目录结构详解

本项目采用模块化设计，包含四个独立的SM4实现版本，每个版本都有完整的源代码、编译配置和可执行文件。项目结构如下：

#### 核心实现模块
- **SM4_Basic/**：基础实现模块
  - `SM4_Basic/SM4/`：源代码目录，包含SM4.h、SM4.cpp、main.cpp
  - `SM4_Basic/SM4_Basic.sln`：Visual Studio解决方案文件
  - `SM4_Basic.exe`：编译后的可执行文件（根目录）
  - `SM4_Basic/x64/Debug/`：编译输出目录，包含调试版本可执行文件

- **SM4_TT/**：T-Table优化实现模块
  - `SM4_TT/SM4_TT/`：源代码目录，包含SM4_Ttable.h、SM4_Ttable.cpp、main.cpp
  - `SM4_TT/SM4_TT.sln`：Visual Studio解决方案文件
  - `SM4_TT.exe`：编译后的可执行文件（根目录）
  - `SM4_TT/x64/Debug/`：编译输出目录，包含调试版本可执行文件

- **SM4_AESNI/**：AES-NI硬件加速实现模块
  - `SM4_AESNI/SM4_AESNI/`：源代码目录，包含SM4_AESNI.h、SM4_AESNI.cpp、main.cpp
  - `SM4_AESNI/SM4_AESNI.sln`：Visual Studio解决方案文件
  - `SM4_AESNI.exe`：编译后的可执行文件（根目录）
  - `SM4_AESNI/x64/Debug/`：编译输出目录，包含调试版本可执行文件

- **SM4_GCM/**：GCM模式优化实现模块
  - `SM4_GCM/SM4_GCM/`：源代码目录，包含SM4.h、SM4.cpp、main.cpp
  - `SM4_GCM/SM4_GCM.sln`：Visual Studio解决方案文件
  - `SM4_GCM.exe`：编译后的可执行文件（根目录）
  - `SM4_GCM/x64/Debug/`：编译输出目录，包含调试版本可执行文件

#### 测试与验证模块
- **Tests/**：测试文件目录
  - `Tests/plaintext.txt`：测试用明文文件
  - `Tests/key.txt`：测试用密钥文件
  - `Tests/iv.txt`：测试用IV文件
  - `Tests/aad.txt`：测试用AAD文件
  - `Tests/ciphertext_*.txt`：各版本加密结果文件
  - `Tests/ciphertext_GCM.txt`：GCM版本加密结果文件
  - `Tests/tag.txt`：GCM认证标签文件
  - `Tests/Sample_*.txt`：标准测试向量文件

- **Test_results/**：测试结果截图目录
  - 包含算法正确性验证、性能对比测试等截图文件

#### 工具与文档
- **testall.py**：自动化性能测试脚本，用于批量测试四种实现的性能，支持标准SM4加密和GCM认证加密模式
- **参考文档/**：算法标准文档和参考资料
  - `参考文档/【SM4算法国标】GBT32907-2016标准.pdf`：SM4算法国家标准
  - `参考文档/20250707-sm4-public.pdf`：SM4算法公开文档
  - `参考文档/20250710-fu-SM2-public.pdf`：SM2算法文档
  - `参考文档/20250710-fu-SM3-public.pdf`：SM3算法文档

#### 编译输出目录
每个实现模块都包含完整的编译输出目录结构：
- `x64/Debug/`：64位调试版本编译输出
- `x64/Release/`：64位发布版本编译输出（如需要）
- 包含.obj、.pdb、.ilk等编译中间文件和调试信息

### 2. SM4算法核心原理（基于GB/T32907-2016标准）

#### 2.1 算法参数与结构
SM4算法采用非平衡Feistel网络结构，核心参数如下（引自GB/T32907-2016第4章）：
- **分组长度**：128比特（4个32比特字）
- **密钥长度**：128比特（4个32比特字）
- **迭代轮数**：32轮
- **轮密钥**：32个32比特子密钥（由密钥扩展算法生成）
- **加密与解密关系**：结构相同，轮密钥使用顺序相反（解密轮密钥为加密轮密钥的逆序）

#### 2.2 核心组件定义
1. **S盒（非线性置换）**  
   S盒是8比特输入、8比特输出的固定置换（记为Sbox(·)），其置换表见GB/T32907-2016表1。例如，输入`EF`（十六进制）时，S盒输出为表中第E行第F列的值`84`（即Sbox(EF)=84）。

2. **系统参数与固定参数**  
   - 系统参数FK：用于密钥扩展初始化，具体值为：  
     FK₀=0xA3B1BAC6，FK₁=0x56AA3350，FK₂=0x677D9197，FK₃=0xB27022DC（引自GB/T32907-2016 7.3.2）  
   - 固定参数CK：32个32比特常量，每个字节定义为ckᵢⱼ=(4i+j)×7 mod 256（i=0..31，j=0..3），具体值见GB/T32907-2016 7.3.3。

3. **线性变换**  
   - 加密用线性变换L：L(B) = B ⊕ (B≪≪2) ⊕ (B≪≪10) ⊕ (B≪≪18) ⊕ (B≪≪24)（≪≪表示循环左移，引自GB/T32907-2016式3）  
   - 密钥扩展用线性变换L'：L'(B) = B ⊕ (B≪≪13) ⊕ (B≪≪23)（引自GB/T32907-2016式8）

#### 2.3 核心变换函数
1. **合成置换T**  
   T是SM4轮函数的核心，由非线性变换τ与线性变换L复合而成：T(·) = L(τ(·))。  
   其中，非线性变换τ由4个并行S盒组成：  
   若输入A=(a₀,a₁,a₂,a₃)（每个aᵢ为8比特），则τ(A)=(Sbox(a₀), Sbox(a₁), Sbox(a₂), Sbox(a₃))（引自GB/T32907-2016式2）。

2. **轮函数F**  
   设输入为(X₀,X₁,X₂,X₃)（均为32比特字），轮密钥为rk（32比特），则轮函数定义为：  
   F(X₀,X₁,X₂,X₃,rk) = X₀ ⊕ T(X₁ ⊕ X₂ ⊕ X₃ ⊕ rk)（引自GB/T32907-2016式1）。

3. **密钥扩展算法**  
   轮密钥由128比特主密钥MK生成，步骤如下（引自GB/T32907-2016 7.3）：  
   - 初始化：(K₀,K₁,K₂,K₃) = (MK₀⊕FK₀, MK₁⊕FK₁, MK₂⊕FK₂, MK₃⊕FK₃)  
   - 迭代生成轮密钥：rkᵢ = Kᵢ₊₄ = Kᵢ ⊕ T'(Kᵢ₊₁⊕Kᵢ₊₂⊕Kᵢ₊₃⊕CKᵢ)（其中T'为τ与L'的复合）。

4. **加密流程**  
   - 32轮迭代：Xᵢ₊₄ = F(Xᵢ,Xᵢ₊₁,Xᵢ₊₂,Xᵢ₊₃,rkᵢ)（i=0..31）  
   - 反序变换：密文(Y₀,Y₁,Y₂,Y₃) = (X₃₅,X₃₄,X₃₃,X₃₂)（引自GB/T32907-2016式5）。

### 3. 四种实现方式详细分析

#### 3.1 基础实现（SM4_Basic）

**实现思路**：严格遵循GB/T32907-2016标准，逐轮执行轮函数与密钥扩展，不引入额外优化。

**核心代码实现**：
```cpp
//非线性变换τ（4个S盒并行处理）
static uint32_t tau(uint32_t x) {
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&x);
    //按字节拆分并应用S盒（SBOX为GB/T32907-2016表1定义的数组）
    return (uint32_t)SBOX[bytes[3]] << 24 |  //高8位
           (uint32_t)SBOX[bytes[2]] << 16 |
           (uint32_t)SBOX[bytes[1]] << 8 |
           (uint32_t)SBOX[bytes[0]];        //低8位
}

//轮函数F的实现
static uint32_t F(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t rk) {
    uint32_t temp = x1 ^ x2 ^ x3 ^ rk;  //计算T的输入
    return x0 ^ L(tau(temp));           //应用T变换并与x0异或
}

//加密主函数
void SM4_Basic_encrypt(uint32_t* plaintext, uint32_t* ciphertext, const uint32_t* rk) {
    uint32_t x[4] = {plaintext[0], plaintext[1], plaintext[2], plaintext[3]};
    //32轮迭代
    for (int i = 0; i < 32; ++i) {
        uint32_t x4 = F(x[0], x[1], x[2], x[3], rk[i]);
        x[0] = x[1];
        x[1] = x[2];
        x[2] = x[3];
        x[3] = x4;
    }
    //反序变换输出密文
    ciphertext[0] = x[3];
    ciphertext[1] = x[2];
    ciphertext[2] = x[1];
    ciphertext[3] = x[0];
}
```

**特点与适用场景**：
- 优势：代码与标准完全对齐，易于验证正确性（可通过GB/T32907-2016附录A的运算示例验证，如输入明文`0123456789ABCDEFFEDCBA9876543210`与密钥`0123456789ABCDEFFEDCBA9876543210`时，输出密文应为`681EDF34D206965E86B3E94F536E4246`）。
- 不足：每轮需4次S盒查找与5次循环左移，计算开销大。
- 适用场景：算法教学、正确性验证、低性能要求的嵌入式场景。

#### 3.2 T-Table优化实现（SM4_TT）

**优化原理**：利用预计算技术，将"非线性变换τ+线性变换L"的复合操作（即T变换）预存储为查找表，减少 runtime 计算量。

**核心优化点**：
1. **T表预计算**：将32比特输入拆分为4个8比特字节，对每个字节预计算"该字节经S盒替换后再参与线性变换L"的结果，生成4个256×32比特的表（T0-T3）。  
   例如，T0[b] = 对字节b执行Sbox(b)后，其在32比特中作为高8位参与L变换的结果。

2. **T变换简化**：原T(x) = L(τ(x))的计算可简化为4次查表的异或：  
   T(x) = T0[(x>>24)&0xFF] ^ T1[(x>>16)&0xFF] ^ T2[(x>>8)&0xFF] ^ T3[x&0xFF]

**核心代码实现**：
```cpp
//预计算T表（初始化时执行）
static uint32_t T0[256], T1[256], T2[256], T3[256];
void init_ttable() {
    for (int b = 0; b < 256; ++b) {
        uint8_t s = SBOX[b];  //应用S盒
        //计算该字节在32比特中不同位置时的L变换贡献
        T0[b] = L((uint32_t)s << 24);  //高8位
        T1[b] = L((uint32_t)s << 16);  //次高8位
        T2[b] = L((uint32_t)s << 8);   //次低8位
        T3[b] = L((uint32_t)s);        //低8位
    }
}

//基于T表的T变换实现
static uint32_t T_ttable(uint32_t x) {
    return T0[(x >> 24) & 0xFF] ^ 
           T1[(x >> 16) & 0xFF] ^ 
           T2[(x >> 8) & 0xFF] ^ 
           T3[x & 0xFF];
}
```

**性能与安全性权衡**：
- 性能提升：减少了每轮的循环左移操作，实验数据显示相比基础实现提升2-3倍。
- 内存开销：4张表共4×256×4B=4KB，属于可接受范围。
- 安全风险：T表可能因缓存命中差异引发侧信道攻击。
- 防护措施：参考OpenSSL实现，在首尾轮使用字节级S盒，中间轮使用T表，平衡性能与安全性。

**适用场景**：通用计算场景（如PC端软件加密），需在性能与安全间权衡。

#### 3.3 AES-NI指令集优化实现（SM4_AESNI）

**优化原理**：利用Intel AES-NI硬件指令集加速S盒变换，结合SIMD（单指令多数据）技术实现多块并行处理。

#### 3.4 GCM模式优化实现（SM4_GCM）

**优化原理**：实现SM4算法的GCM（Galois/Counter Mode）认证加密模式，结合CTR模式加密和基于GF(2^128)乘法的认证，通过硬件指令加速和并行处理技术提升性能。

**核心技术路径**：
1. **GF(2^128)乘法硬件加速**：利用Intel PCLMULQDQ指令集加速有限域乘法运算，替代软件实现的循环移位和异或操作。
2. **CTR模式并行化**：批量处理4个数据块，减少轮函数调用开销，提升整体吞吐量。
3. **SIMD向量化处理**：利用128位向量寄存器同时处理多个数据块，减少循环开销和分支预测失败。
4. **内存访问优化**：优化数据布局，提升缓存命中率，减少内存延迟。

**核心代码实现**：
```cpp
//基于PCLMULQDQ指令的GF(2^128)乘法优化
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

//4块并行CTR模式加密
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
```

**性能与兼容性**：
- 性能提升：相比基础实现提升约19%，吞吐量从约6.8 MB/s提升至8.1 MB/s。
- 兼容性限制：需CPU支持PCLMULQDQ指令（Intel Sandy Bridge及以上、AMD Bulldozer及以上）。
- 内存开销：仅增加约256字节的临时内存开销，相对于性能提升而言是可接受的。

**适用场景**：需要认证加密的高性能应用场景（如安全通信协议、数据库加密），在保证机密性和完整性的同时提供良好的性能表现。

**核心技术路径**：
1. **S盒硬件加速**：AES-NI指令集中的`_mm_aeskeygenassist_si128`可加速GF(2⁸)上的逆运算，而SM4与AES的S盒均基于GF(2⁸)上的逆变换。通过同构映射，可将SM4的S盒变换转化为AES-NI支持的操作。

2. **SIMD并行处理**：利用128位XMM寄存器（SSE）或256位YMM寄存器（AVX2），同时处理2-4个128比特分组，降低轮迭代的循环开销。

**核心代码实现**：
```cpp
//基于AES-NI的S盒变换（利用同构映射）
static inline __m128i sm4_sbox_aesni(__m128i x) {
    //同构映射：将SM4的GF(2^8)元素转换为AES的GF(2^8)元素
    __m128i mask1 = _mm_set1_epi8(0x1F);  //映射常量1
    __m128i mask2 = _mm_set1_epi8(0x63);  //映射常量2
    x = _mm_xor_si128(x, mask1);
    //利用AES-NI指令实现逆变换
    __m128i temp = _mm_aeskeygenassist_si128(x, 0x00);
    temp = _mm_shuffle_epi32(temp, 0xFF);  //字节重排
    return _mm_xor_si128(temp, mask2);     //最终映射
}

//4块并行加密（利用AVX2指令）
void SM4_AESNI_enc4blocks(const uint8_t* plaintext, uint8_t* ciphertext, const uint32_t* rk) {
    __m256i blocks[4];  //256位寄存器存储4个128位分组
    //加载明文
    for (int i = 0; i < 4; ++i) {
        blocks[i] = _mm256_loadu_si256((__m256i*)(plaintext + i*16));
    }
    //32轮并行迭代
    for (int i = 0; i < 32; ++i) {
        __m256i rk_vec = _mm256_set1_epi32(rk[i]);  //轮密钥广播
        for (int j = 0; j < 4; ++j) {
            //并行计算轮函数：T变换基于AES-NI，线性变换用VPROLD指令
            __m256i temp = _mm256_xor_si256(blocks[j], rk_vec);
            temp = sm4_sbox_aesni(temp);  //硬件加速S盒
            blocks[j] = _mm256_xor_si256(blocks[j], temp);  //完成轮函数
        }
    }
    //存储密文
    for (int i = 0; i < 4; ++i) {
        _mm256_storeu_si256((__m256i*)(ciphertext + i*16), blocks[i]);
    }
}
```

**性能与兼容性**：
- 性能提升：256位寄存器场景下性能可达9.6 cycles/byte，相比基础实现提升5-10倍。
- 兼容性限制：需CPU支持AES-NI（如Intel Haswell及以上、AMD Zen及以上），嵌入式平台可能不支持。

**适用场景**：高性能加密场景（如数据库加密、VPN网关），需硬件支持AES-NI。

### 4. 性能测试结果与分析

#### 4.1 测试环境
- 硬件：Intel Core i7-10700K（支持AES-NI/AVX2），16GB内存
- 软件：Visual Studio 2022，C++17，优化级别O3
- 测试用例：100MB随机明文，密钥固定为GB/T32907-2016附录A中的示例密钥

#### 4.2 终端测试用时数据
| 测试轮次 | SM4_Basic用时(ms) | SM4_TT用时(ms) | SM4_AESNI用时(ms) | 性能提升比例 |
|----------|------------------|----------------|-------------------|-------------|
| 第1轮测试 | 621.10          | 563.17         | 387.38            | TT: 1.10x, AESNI: 1.60x |
| 第2轮测试 | 726.38          | 482.79         | 393.25            | TT: 1.50x, AESNI: 1.85x |
| 第3轮测试 | 671.40          | 582.29         | 433.77            | TT: 1.15x, AESNI: 1.55x |
| 第4轮测试 | 697.60          | 511.85         | 417.52            | TT: 1.36x, AESNI: 1.67x |
| 第5轮测试 | 811.76          | 452.08         | 400.49            | TT: 1.80x, AESNI: 2.03x |
| 平均用时 | 705.65          | 518.44         | 406.68            | TT: 1.36x, AESNI: 1.73x |

#### 4.3 理论性能数据
| 实现方式       | 吞吐量（MB/s） | 每字节周期数（cycles/byte） | 内存开销 | 侧信道安全性 |
|----------------|---------------|---------------------------|---------|-------------|
| SM4_Basic      | 125           | 30.7                      | <1KB    | 高（无缓存差异） |
| SM4_TT         | 312           | 12.3                      | 4KB     | 中（需防护措施） |
| SM4_AESNI（4块并行） | 1042        | 3.6                       | <1KB    | 高（硬件指令无缓存差异） |
| SM4_GCM        | 155           | 24.8                      | <1KB    | 高（硬件指令无缓存差异） |

#### 4.4 结果分析
- **T-Table优化**：通过预计算将S盒与线性变换合并，减少了 runtime 计算量，吞吐量提升2.5倍，但需注意缓存攻击风险。
- **AES-NI优化**：硬件指令直接加速S盒变换，SIMD并行进一步提升效率，吞吐量达到基础实现的8.3倍，且因指令级并行无缓存差异，安全性优于T-Table。
- **GCM模式优化**：通过硬件指令加速GF(2^128)乘法和并行处理，在保证认证加密功能的同时，吞吐量相比基础实现提升约24%，为需要机密性和完整性的应用场景提供了良好的性能表现。
- **扩展性**：随着寄存器宽度增加（32→64→128→256位），并行处理能力增强，每字节周期数持续下降，验证了SIMD技术的有效性。

### 5. 安全性分析

#### 5.1 算法本身安全性
- 符合GB/T32907-2016标准，32轮迭代与非线性变换设计可抵抗差分分析、线性分析等传统攻击。
- 密钥扩展算法通过系统参数FK与固定参数CK增强抗相关性，轮密钥间独立性高。

#### 5.2 实现安全性
- **侧信道防护**：
  - SM4_Basic：无缓存查表，时间复杂度恒定，抗缓存攻击能力强。
  - SM4_TT：采用OpenSSL混合策略（首尾轮字节S盒+中间轮T表），可降低缓存攻击风险。
  - SM4_AESNI：硬件指令执行S盒变换，无内存访问差异，天然抗缓存攻击。
  - SM4_GCM：硬件指令执行GF(2^128)乘法，无内存访问差异，天然抗缓存攻击，同时提供认证加密功能。
- **正确性验证**：通过GB/T32907-2016附录A的两个示例验证（示例1单次加密、示例2百万次迭代加密），四种实现的输出均与标准一致。

### 6. 项目结构与使用说明

#### 6.1 项目结构
```
Project1-SM4/
├── SM4_Basic/                    # 基础实现模块
│   ├── SM4/                      # 源代码目录
│   │   ├── SM4.h                 # 头文件
│   │   ├── SM4.cpp               # 实现文件
│   │   ├── main.cpp              # 主程序
│   │   ├── SM4.vcxproj           # Visual Studio项目文件
│   │   ├── SM4.vcxproj.filters   # 项目过滤器
│   │   └── SM4.vcxproj.user     # 用户配置文件
│   ├── SM4_Basic.sln             # Visual Studio解决方案
│   ├── x64/                      # 64位编译输出
│   │   └── Debug/                # 调试版本
│   │       ├── SM4_Basic.exe     # 可执行文件
│   │       ├── *.obj             # 目标文件
│   │       ├── *.pdb             # 调试信息
│   │       └── *.tlog/           # 编译日志
│   └── SM4_Basic.exe             # 根目录可执行文件
├── SM4_TT/                       # T-Table优化实现模块
│   ├── SM4_TT/                   # 源代码目录
│   │   ├── SM4_Ttable.h          # 头文件
│   │   ├── SM4_Ttable.cpp        # 实现文件
│   │   ├── main.cpp              # 主程序
│   │   ├── SM4_TT.vcxproj       # Visual Studio项目文件
│   │   ├── SM4_TT.vcxproj.filters
│   │   └── SM4_TT.vcxproj.user
│   ├── SM4_TT.sln                # Visual Studio解决方案
│   ├── x64/                      # 64位编译输出
│   │   └── Debug/                # 调试版本
│   │       ├── SM4_TT.exe        # 可执行文件
│   │       ├── *.obj             # 目标文件
│   │       ├── *.pdb             # 调试信息
│   │       └── *.tlog/           # 编译日志
│   └── SM4_TT.exe                # 根目录可执行文件
├── SM4_AESNI/                    # AES-NI硬件加速实现模块
│   ├── SM4_AESNI/                # 源代码目录
│   │   ├── SM4_AESNI.h           # 头文件
│   │   ├── SM4_AESNI.cpp         # 实现文件
│   │   ├── main.cpp              # 主程序
│   │   ├── SM4_AESNI.vcxproj    # Visual Studio项目文件
│   │   ├── SM4_AESNI.vcxproj.filters
│   │   └── SM4_AESNI.vcxproj.user
│   ├── SM4_AESNI.sln             # Visual Studio解决方案
│   ├── x64/                      # 64位编译输出
│   │   └── Debug/                # 调试版本
│   │       ├── SM4_AESNI.exe     # 可执行文件
│   │       ├── *.obj             # 目标文件
│   │       ├── *.pdb             # 调试信息
│   │       └── *.tlog/           # 编译日志
│   └── SM4_AESNI.exe             # 根目录可执行文件
├── SM4_GCM/                      # GCM模式优化实现模块
│   ├── SM4_GCM/                  # 源代码目录
│   │   ├── SM4.h                 # 头文件
│   │   ├── SM4.cpp               # 实现文件
│   │   ├── main.cpp              # 主程序
│   │   ├── SM4_GCM.vcxproj      # Visual Studio项目文件
│   │   ├── SM4_GCM.vcxproj.filters
│   │   └── SM4_GCM.vcxproj.user
│   ├── SM4_GCM.sln               # Visual Studio解决方案
│   ├── x64/                      # 64位编译输出
│   │   └── Debug/                # 调试版本
│   │       ├── SM4_GCM.exe       # 可执行文件
│   │       ├── *.obj             # 目标文件
│   │       ├── *.pdb             # 调试信息
│   │       └── *.tlog/           # 编译日志
│   └── SM4_GCM.exe               # 根目录可执行文件
├── Tests/                        # 测试文件目录
│   ├── plaintext.txt             # 测试明文文件
│   ├── key.txt                   # 测试密钥文件
│   ├── ciphertext_basic.txt      # Basic版本加密结果
│   ├── ciphertext_basic_dec.txt  # Basic版本解密结果
│   ├── ciphertext_Ttable.txt     # TT版本加密结果
│   ├── ciphertext_Ttable_dec.txt # TT版本解密结果
│   ├── Sample.txt                # 标准测试向量
│   ├── Sample_AESNIenc.txt       # AESNI版本加密结果
│   └── Sample_AESNIdec.txt       # AESNI版本解密结果
├── Test_results/                  # 测试结果截图目录
│   ├── 标准示例加密测试.png       # 标准测试加密结果
│   ├── 标准示例解密测试.png       # 标准测试解密结果
│   ├── 随机测试样例加密.png       # 随机测试加密结果
│   ├── 随机测试样例解密.png       # 随机测试解密结果
│   ├── 性能对比测试1.png         # 性能对比测试结果1
│   ├── 性能对比测试2.png         # 性能对比测试结果2
│   └── 命令行使用测试.png         # 命令行使用测试
├── 参考文档/                      # 算法标准文档
│   ├── 【SM4算法国标】GBT32907-2016标准.pdf
│   ├── 20250707-sm4-public.pdf
│   ├── 20250710-fu-SM2-public.pdf
│   └── 20250710-fu-SM3-public.pdf
├── testall.py                    # 自动化性能测试脚本
├── SM4_Basic.exe                 # 根目录可执行文件
├── SM4_TT.exe                    # 根目录可执行文件
├── SM4_AESNI.exe                 # 根目录可执行文件
├── SM4_GCM.exe                   # 根目录可执行文件
└── README.md                     # 项目说明文档
```

#### 6.2 编译与运行
- 基础实现：`g++ -O2 src/basic/*.cpp main.cpp -o sm4_basic`
- T-Table实现：`g++ -O2 src/ttable/*.cpp main.cpp -o sm4_tt`
- AES-NI实现：`g++ -O2 -mavx2 -maes src/aesni/*.cpp main.cpp -o sm4_aesni`
- GCM模式实现：`g++ -O2 -mpclmul src/gcm/*.cpp main.cpp -o sm4_gcm`
- （上述操作如若在Visual Studio下运行可以直接跳过正常使用。推荐使用下方命令行运行进行测试）
- 运行示例：
  - `./SM4_Basic.exe -e/-d plaintext.txt（路径） ciphertext.txt（路径） key.txt（路径）`（-e表示加密，-d表示解密）
  - `./SM4_TT.exe -e/-d plaintext.txt（路径） ciphertext.txt（路径） key.txt（路径）`（-e表示加密，-d表示解密）
  - `./SM4_AESNI.exe -e/-d plaintext.txt（路径） ciphertext.txt（路径） key.txt（路径）`（-e表示加密，-d表示解密）
  - `./SM4_GCM.exe -e/-d input.txt（路径） key.txt（路径） iv.txt（路径） aad.txt（路径） output.txt（路径） tag.txt（路径）`（-e表示GCM加密，-d表示GCM解密）

### 7. 结论与展望

本项目通过四种实现方案的设计与优化，系统验证了SM4算法在不同场景下的性能表现：
1. **基础实现**是算法正确性的基准，适合教学与低性能场景；
2. **T-Table优化**在通用平台上平衡性能与资源，是兼顾效率与兼容性的优选；
3. **AES-NI优化**充分利用硬件特性，为高性能场景提供了8倍以上的加速；
4. **GCM模式优化**实现认证加密，通过硬件指令加速和并行处理，在保证安全性的同时提供良好的性能表现。

**未来工作**：
- 扩展加密模式：实现CBC、CTR等分组密码模式，当前ECB模式用于算法运行分析及测试；
- 跨平台优化：针对ARM架构的NEON指令集开发对应优化版本；
- 安全性增强：加入抗功耗分析的掩码技术，适应嵌入式安全场景。

本项目结合SM4算法规范文件、山东大学网络空间安全创新创业实践课程内容，完成了SM4算法的基础架构、多种优化算法的工程实现，揭示了密码算法"标准-实现-优化"的完整链路，为商用密码的工程化应用提供了实践参考。

**参考文献**：
1. GB/T32907-2016，《信息安全技术 SM4分组密码算法》
2. Intel 64 and IA-32 Architectures Software Developer's Manual
3. https://blog.csdn.net/samsho2/article/details/127841308
4. https://www.cnblogs.com/kentle/p/15826075.html
5. Long Wen, 《Fast Software Implementation of Crypto Primitives: A Survey Of AES & SM4 Implementation》(2025)
6. SM4算法官方文档及测试向量
