# Project 3：使用 Circom 实现 Poseidon2 哈希电路并基于 Groth16 生成与验证零知识证明

本报告围绕山东大学网络空间安全创新创业实践课程项目要求展开，并对照给出设计与实现：
- 参数选型：参考参考文档 1（Poseidon2 论文）Table 1，选用 (n,t,d)=(256,3,5)；同时电路结构可自然收缩到 (256,2,5)。
- 接口约定：公开输入为 Poseidon2 哈希值，隐私输入为哈希原象，仅考虑单个 block。
- 证明系统：采用 Groth16 生成证明并验证。

参考文档：
- Poseidon2 哈希算法（论文）: https://eprint.iacr.org/2023/323.pdf
- Circom 文档: https://docs.circom.io/
- Circom 电路样例: https://github.com/iden3/circomlib

## 1. 项目结构与职责

- `circuits/`
  - `main.circom`：主电路。隐私输入 `in0,in1` 作为原象，公开输入 `pubHash`。电路内部调用 `Poseidon2Perm` 进行 3 元状态的置换迭代，输出状态的第 1 元与 `pubHash` 约束相等。
  - `poseidon2_perm.circom`、`poseidon2_constants.circom`：实现 Poseidon2 的轮函数与轮常数（含 MDS）以满足 (256,3,5)。
- `params/params.json`：JS 域参数、轮常数、MDS。
- `circuits/poseidon2_js.js`：Node 实现（与电路语义一致），便于快速校验哈希正确性。
- `calc_poseidon2.js`：根据输入计算哈希并生成 `input.json`（键：`in0`,`in1`,`pubHash`）。
- `test_js_only.js`：JS 快速校验脚本（不依赖 circom/snarkjs）。
- `test_poseidon2.js`：验证脚本。解析可用 `snarkjs` 命令并固定使用 `set_result/` 下三件文件完成 Groth16 验证。
- `set_result/`：预构建验证文件（便于直接验证，不必本地编译）：
  - `verification_key_groth16.json`
  - `public_inputs.json`
  - `proof_groth16.json`
- `exp_result/`：实验过程截图与图像（见该目录）。

### 1.1 项目目录结构

```
Project3-poseidon2/
├── circuits/                          # Circom 电路与 JS 参考实现
│   ├── main.circom                   # 主电路：in0,in1 私有；pubHash 公开；约束 pubHash==state'[0]
│   ├── poseidon2_perm.circom         # Poseidon2 置换：全轮/部分轮，S-box(x^5)、MDS 等
│   ├── poseidon2_constants.circom    # 轮常数、MDS 等常量
│   └── poseidon2_js.js               # JS 参考实现（与电路一致）
├── params/
│   └── params.json                   # 域参数 p、轮常数、MDS（供 JS 使用）
├── set_result/                        # 预构建验证产物（Groth16）
│   ├── verification_key_groth16.json # 验证密钥（VK）
│   ├── public_inputs.json            # 公开输入（JSON 数组形式）
│   └── proof_groth16.json            # 证明（Proof）
├── exp_result/                        # 实验过程图像及实验运行结果
├── calc_poseidon2.js                  # 计算哈希并生成 input.json
├── input.json                         # 实验输入（in0,in1,pubHash）
├── test_js_only.js                    # 仅 JS 快速校验（无需 circom/snarkjs）
├── test_poseidon2.js                  # 使用 set_result 进行 Groth16 验证（自动解析 snarkjs 命令）
├── package.json                       # NPM 脚本：calc/test/test-js
└── README.md                          # 实验报告（本文件）
```

## 2. 理论与参数选型（(n,t,d)=(256,3,5)）

本项目实现基于 Poseidon2 置换与 Sponge 海绵结构，以下为核心原理。

### 2.1 有限域与参数
- 有限域：使用 BN254/alt_bn128 椭圆曲线同一素域 \(\mathbb{F}_p\)（snarkjs 的 Groth16 默认曲线），素数 \(p\) 见 `params/params.json`。
- Poseidon2 参数：\(n=256\)（域位宽），\(t=3\)（状态长度），\(d=5\)（S-box 指数）。

### 2.2 Poseidon2 置换结构
对每一轮 \(r\) 执行：
1) 加轮常数：\(\mathbf{s} \leftarrow \mathbf{s} + \mathbf{c}_r\)
2) S-box 非线性：\(\phi(x)=x^5\)。完整轮对全部 \(t\) 个元素施加；部分轮仅对第 1 个元素施加。
3) MDS 线性层：\(\mathbf{s} \leftarrow M\cdot\mathbf{s}\)，其中 \(M\) 为满足最大距离可分离性的矩阵，保证良好扩散。

Poseidon2 相比早期 Poseidon 方案在轮数与安全分析上更紧致，使用 \(x^5\) 兼顾代数度与电路开销。轮数（完整轮 RF、部分轮 RP）按论文 Table 1 的安全参数固定在常量文件中。

### 2.3 Sponge 模式：吸收与挤压（单 block）
- 本项目只考虑单个 block（两元素）输入，状态长度 \(t=3\) 其中“速率” \(r=2\)、“容量” \(c=1\)。
- 初始状态设为 \([\text{in0},\,\text{in1},\,0]\)，即先吸收两个隐私输入，保留 1 个容量位以确保安全边界。
- 经过若干轮置换后，取状态的第一个元素 \(s_0\) 作为哈希输出。

### 2.4 选择 \(d=5\) 的动机与安全性
- \(x^5\) 兼具较高代数度与较低电路实现成本（相较于更高次幂），便于在 R1CS 中以少量乘法门实现（平方两次再乘自身）。
- 完整轮与部分轮策略在保证扩散与抗代数攻击的同时，显著减少总轮数与约束数量。

### 2.5 Circom 约束映射与复杂度
- S-box \(x^5\)：通过 \(x^2\)、\(x^4\)、\(x^5\) 的逐步构造实现，乘法门数量较少。
- MDS：线性层以常量矩阵与加法、常数乘实现，约束负担主要来自乘法操作。
- 约束规模与 \(t\)、轮数（RF、RP）成正比；采用 \(t=3\)、部分轮策略可获得较优的性能/安全折中。

### 2.6 Groth16 证明原理
1) 将 Circom 电路编译为 R1CS（Rank-1 Constraint System）。
2) 可信设置（Powers of Tau + Groth16 setup）生成 proving key/verification key。
3) Witness 由输入计算得到，证明者用 proving key 对满足电路的 witness 生成证明 \(\pi\)。
4) 验证者使用 verification key 与公开输入（本项目为哈希值）验证 \(\pi\) 的正确性，而无需获知隐私输入（原象）。

零知识性保证隐私输入不泄露，完备性与可靠性保证“真命题一定可证，假命题不可伪造证明”。

## 3. 电路设计与接口

- 隐私输入：`in0, in1`（单 block 原象，均为 Fr 元素）
- 公开输入：`pubHash`（Poseidon2 哈希结果）
- 约束关系：令初始状态 `state=[in0,in1,0]`，经过 Poseidon2 置换后得到 `state'`，有 `pubHash === state'[0]`

主电路 `circuits/main.circom`（摘要说明）：
- 声明输入 `in0,in1,pubHash`
- 构造初始状态并调用 `Poseidon2Perm`
- 断言输出第 1 元等于 `pubHash`

说明：若选 (256,2,5)，可将 `in1` 置零或调整电路模板以 t=2 的版本（状态为 2 元，吸收一个输入），思路相同。

## 4. 参考实现（JS）

`circuits/poseidon2_js.js` 提供与电路一致的哈希逻辑，便于快速本地校验：
- 域运算在 Fr(p) 中进行；
- 轮常数、MDS 由 `params.json` 给出；
- S-box 使用 \(x^5\)。

快速校验：
```bash
npm run test-js
# 或： node test_js_only.js [in0] [in1]
```

## 5. 环境与依赖

本实验在Ubuntu20.04系统下完成。

- Node.js >= 14
- Circom 2.x（建议 2.1+）
- snarkjs（全局或 npx 均可）

## 6. 实验步骤

### A. 仅用 JS 快速验证哈希
```bash
npm run test-js
```
脚本会：
- 计算 `poseidon2([in0,in1])`；
- 与 `input.json` 中的 `pubHash` 一致性检查；
- 若 `input.json` 不存在或无 `pubHash`，将写入一份便于后续电路验证。

### B. 使用预置 Groth16 产物直接验证

本项目已在 `set_result/` 准备好验证所需文件：
- `verification_key_groth16.json`
- `public_inputs.json`（注意为 JSON 数组格式）
- `proof_groth16.json`

两种方式进行验证：
1) 一键脚本（自动选择 snarkjs 命令）：
```bash
npm test
# 或： node test_poseidon2.js
```
2) 手动命令：
```bash
snarkjs groth16 verify set_result/verification_key_groth16.json \
                         set_result/public_inputs.json \
                         set_result/proof_groth16.json
```

### C. 编译与生成证明

遵循 circom/snarkjs 标准流程：
```bash
# 1) 编译电路（生成 r1cs/wasm/sym）
circom circuits/main.circom --r1cs --wasm --sym -o build

# 2) 计算witness
node build/main_js/generate_witness.js build/main.wasm input.json build/witness.wtns

# 3) Powers of Tau（示例操作）
snarkjs powersoftau new bn128 12 1.ptau -v
snarkjs powersoftau contribute 1.ptau 2.ptau --name="c1" -v
snarkjs powersoftau prepare phase2 build/1.ptau build/final.ptau -v

# 4) Groth16 setup + 第二次贡献
snarkjs groth16 setup build/main.r1cs build/final.ptau build/1.zkey
snarkjs zkey contribute build/1.zkey build/2.zkey --name="c2" -v

# 5) 导出验证密钥
snarkjs zkey export verificationkey build/1.zkey build/verification_key.json

# 6) 生成证明
snarkjs groth16 prove build/1.zkey build/witness.wtns build/proof.json build/public.json

# 7) 验证证明
snarkjs groth16 verify build/verification_key.json build/public.json build/proof.json
```

## 7. 实验结果与分析

# 实验流程与结果图像保存在exp_result文件夹下

- JS 快速校验：对默认 `in0=123,in1=456`，计算得到的哈希与 `input.json` 中 `pubHash` 一致，验证通过。
- 预置 Groth16 验证：`set_result/` 下三件套可在安装好 `snarkjs` 的环境中直接验证。
- 过程截图与更多实验图像见 `exp_result/` 文件夹（包含环境配置、指令执行、验证通过/失败等关键节点）。

讨论：
- 安全性与性能由 (t,d) 与轮数共同决定；采用 t=3, d=5 的 Poseidon2 在电路规模与安全性间取得平衡。
- 生产环境应严格使用论文附录中的标准常数与 MDS；本项目常数随附于源码并可替换为审计版参数。
- 若切换到 (256,2,5)，可调整状态大小与吸收策略，证明流程保持一致。

## 8. 复现与脚本速览

- 生成输入：
```bash
npm run calc
# 或： node calc_poseidon2.js [in0] [in1]
```
- 仅 JS 校验：`npm run test-js`
- Groth16 验证：`npm test`（或上文手动命令）

## 9. 参考文献

[1] Poseidon2 论文: https://eprint.iacr.org/2023/323.pdf

[2] Circom 文档: https://docs.circom.io/

[3] circomlib: https://github.com/iden3/circomlib

[4] Circom 2.0: A Scalable Circuit Compiler https://blog.csdn.net/mutourend/article/details/125953309?ops_request_misc=%257B%2522request%255Fid%2522%253A%252209155b71a380425cf82458ffbaf8866d%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=09155b71a380425cf82458ffbaf8866d&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-125953309-null-null.142^v102^control&utm_term=circom&spm=1018.2226.3001.4187
