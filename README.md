## 个人信息

姓名：秦川

专业：网络空间安全

学号：202200460058

班级：22级网安1班

## 山东大学网络空间安全创新创业实践课程项目总览

本仓库汇总了山东大学网络空间安全创新创业实践课程的六大实践项目，围绕国密算法、数字水印技术、密钥检测协议与哈希算法等工程实现与优化展开，覆盖算法标准实现、性能优化、测试评测与综合应用等各方面，系统化提升密码工程能力与安全实践素养。

### 目录结构

```text
Project/
├── Project1-SM4/                     # 项目一：SM4分组密码实现与优化
│   ├── SM4_Basic/                    # 基础实现
│   ├── SM4_TT/                       # T-Table优化实现
│   ├── SM4_AESNI/                    # AES-NI/SIMD硬件加速实现
│   ├── Tests/                        # 测试用明文/密钥/样例结果
│   ├── Test_results/                 # 正确性与性能截图
│   ├── testall.py                    # 性能测试脚本
│   └── README.md                     # 项目一实验报告
├── Project2-Watermark/               # 项目二：数字水印技术实现与应用
│   ├── Watermark/                    # 水印嵌入/提取核心
│   ├── Watermark_Attacks/            # 常见攻击集合
│   ├── Robustness_Test/              # 鲁棒性评测
│   ├── Samples/                      # 样例载体/水印
│   ├── output/                       # 输出与鲁棒性报告
│   ├── main.py                       # 运行入口
│   ├── test_watermark.py             # 测试脚本
│   └── README.md                     # 项目二实验报告
├── Project3-Poseidon2/               # 项目三：Poseidon2哈希算法实现与优化
│   ├── circuits/                     # Circom电路
│   ├── params/                       # 参数文件
│   ├── set_result/                   # 证明与验证工件
│   ├── exp_result/                   # 运行截图
│   ├── calc_poseidon2.js             # JS计算脚本
│   ├── test_js_only.js
│   ├── test_poseidon2.js
│   ├── input.json
│   ├── package.json
│   └── README.md                     # 项目三实验报告
├── Project4-SM3/                     # 项目四：SM3杂凑算法实现与优化
│   ├── SM3_Basic/                    # 基础实现
│   ├── SM3_Opti1/                    # 循环展开优化实现
│   ├── SM3_Opti2/                    # AVX2/SIMD硬件加速实现
│   ├── Test_Results/                 # 性能测试结果截图
│   └── README.md                     # 项目四实验报告
├── Project5-SM2/                     # 项目五：SM2椭圆曲线公钥密码算法实现与优化
│   ├── SM2_Basic/                    # 基础实现
│   ├── SM2_Opti/                     # 多技术融合优化实现
│   ├── Test_Results/                 # 性能测试结果
│   └── README.md                     # 项目五实验报告
├── Project6-GPC/                     # 项目六：Google Password Checkup协议实现
│   ├── GPC/                          # GPC协议核心构建
│   ├── exp_result/                   # 运行结果示例
│   ├── GPC_main.py                   # 演示入口
│   └── README.md                     # 项目六实验报告
└── README.md                         # 个人信息与课程项目总览
```


