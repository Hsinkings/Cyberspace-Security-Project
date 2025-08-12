//circuits/poseidon2_js.js
//实现思路：使用 params/params.json 中的素数 p、轮次和常量，按照标准 Poseidon 轮结构执行
'use strict';

const params = require("../params/params.json");
const MOD_P = BigInt(params.p);

//将整数缩减到域 [0, p-1]
function modP(x) {
  const r = x % MOD_P;
  return r >= 0n ? r : r + MOD_P;
}

//S-box：x -> x^5（使用逐步求幂以减少中间溢出）
function sboxPow5(x) {
  const x2 = modP(x * x);
  const x4 = modP(x2 * x2);
  return modP(x4 * x);
}

//使用 MDS 矩阵对状态做线性混合（矩阵可能在 params 中被指定）
function applyMDS(stateArr, mds) {
  const out = [0n, 0n, 0n];
  for (let i = 0; i < 3; i++) {
    let acc = 0n;
    for (let j = 0; j < 3; j++) {
      acc = modP(acc + BigInt(mds[i][j]) * stateArr[j]);
    }
    out[i] = acc;
  }
  return out;
}

//主函数：接受长度为 2 的数组 inputs（两个前像），返回一个 BigInt（哈希值）
function poseidon2(inputs) {
  if (!Array.isArray(inputs) || inputs.length !== 2) throw new Error("poseidon2: 需要 2 个输入");
  const in0 = BigInt(inputs[0]);
  const in1 = BigInt(inputs[1]);

  //初始化状态：state = [0, in0, in1]
  let state = [0n, in0, in1];

  //从 params 读取轮次与常量
  const RF = Number(params.rf || 2);  //全轮（总数）
  const RP = Number(params.rp || 1);  //部分轮次数
  const roundConstsRaw = params.round_constants_flat || [];
  //把常数转换为 BigInt，长度不足时用 0 补齐
  const expectedConsts = (RF + RP) * 3;
  const roundConsts = new Array(expectedConsts).fill(0n);
  for (let i = 0; i < Math.min(roundConstsRaw.length, expectedConsts); i++) {
    roundConsts[i] = BigInt(roundConstsRaw[i]);
  }

  const mds = params.mds || [[1,0,0],[0,1,0],[0,0,1]];

  let rcIndex = 0;
  const halfFull = Math.floor(RF / 2);

  //前半段的全轮（对每个状态元素都执行 S-box）
  for (let r = 0; r < halfFull; r++) {
    for (let i = 0; i < 3; i++) state[i] = modP(state[i] + roundConsts[rcIndex + i]);
    rcIndex += 3;
    for (let i = 0; i < 3; i++) state[i] = sboxPow5(state[i]);
    state = applyMDS(state, mds);
  }

  //中间的部分轮（仅对 state[0] 执行 S-box）
  for (let r = 0; r < RP; r++) {
    for (let i = 0; i < 3; i++) state[i] = modP(state[i] + roundConsts[rcIndex + i]);
    rcIndex += 3;
    state[0] = sboxPow5(state[0]);
    state = applyMDS(state, mds);
  }

  //后半段的全轮
  for (let r = 0; r < halfFull; r++) {
    for (let i = 0; i < 3; i++) state[i] = modP(state[i] + roundConsts[rcIndex + i]);
    rcIndex += 3;
    for (let i = 0; i < 3; i++) state[i] = sboxPow5(state[i]);
    state = applyMDS(state, mds);
  }

  //返回哈希（选择 state[0] 作为输出——与电路约定一致）
  return state[0];
}

module.exports = { poseidon2 };
