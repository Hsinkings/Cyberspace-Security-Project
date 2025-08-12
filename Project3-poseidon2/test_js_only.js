//test_js_only.js - 仅使用本仓库内 JS 实现进行快速测试（无需 circom/snarkjs）
//用法： node test_js_only.js [in0] [in1]
const { poseidon2 } = require("./circuits/poseidon2_js");
const fs = require("fs");

const a = process.argv[2] || "123";
const b = process.argv[3] || "456";

try {
  const h = poseidon2([a, b]).toString();
  console.log("前像 (in0, in1)：", [a, b]);
  console.log("Poseidon2 哈希：", h);

  const inputPath = "input.json";
  if (fs.existsSync(inputPath)) {
    const data = JSON.parse(fs.readFileSync(inputPath));
    //如果 input.json 包含 pubHash，则进行比较
    if (data.pubHash !== undefined) {
      if (String(data.pubHash) === h) {
        console.log("input.json 中的 pubHash 与计算结果一致。");
        process.exit(0);
      } else {
        console.error("input.json 中的 pubHash 与计算结果不一致。");
        console.error("input.json pubHash:", data.pubHash);
        console.error("计算得到的 pubHash:", h);
        process.exit(2);
      }
    } else {
      //若 input.json 存在但没有 pubHash，则覆盖写入
      const out = { in0: a, in1: b, pubHash: h };
      fs.writeFileSync(inputPath, JSON.stringify(out, null, 2));
      console.log("input.json 中未包含 pubHash，已写入新的 input.json。");
      process.exit(0);
    }
  } else {
    //如果没有 input.json，则生成一个，便于后续使用 circom/snarkjs 流程
    const out = { in0: a, in1: b, pubHash: h };
    fs.writeFileSync(inputPath, JSON.stringify(out, null, 2));
    console.log("生成了 input.json（用于后续电路证明流程）。");
    process.exit(0);
  }
} catch (e) {
  console.error("执行失败：", e && e.message ? e.message : e);
  process.exit(1);
}
