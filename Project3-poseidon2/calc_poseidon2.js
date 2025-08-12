//calc_poseidon2.js
//计算 Poseidon2 哈希并写入 input.json（供电路/目视验证使用）
const { poseidon2 } = require("./circuits/poseidon2_js");
const fs = require("fs");

const a = process.argv[2] || "123";
const b = process.argv[3] || "456";

try {
  const h = poseidon2([a, b]).toString();
  console.log("前像 (in0, in1)：", [a, b]);
  console.log("计算得到的 Poseidon2 哈希：", h);

  //写入 input.json，格式与 circuits/main.circom 的输入名一致：in0, in1, pubHash
  const input = { in0: a, in1: b, pubHash: h };
  fs.writeFileSync("input.json", JSON.stringify(input, null, 2));
  console.log("已生成 input.json（键：in0, in1, pubHash）。");
} catch (e) {
  console.error("计算出错：", e && e.message ? e.message : e);
  process.exit(1);
}
