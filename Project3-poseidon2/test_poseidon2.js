//test_poseidon2.js
const { execSync } = require('child_process');
const path = require('path');
const fs = require('fs');

//解析可用的 snarkjs 命令：优先使用全局 snarkjs，不可用则回退到 npx -y snarkjs
function resolveSnarkjsCommand() {
    const candidates = [
        'snarkjs',
        'npx -y snarkjs',
        'npx snarkjs'
    ];
    for (const cmd of candidates) {
        try {
            //优先尝试 -v；如果退出码非 0 也可能输出了帮助/版本信息
            execSync(`${cmd} -v`, { stdio: 'pipe' });
            return cmd;
        } catch (err) {
            //仅当命令不存在（ENOENT）时才继续尝试下一个候选
            const code = err && err.code;
            //有输出且包含 snarkjs/Usage 也认为命令存在（部分版本会以非零码退出）
            const outStr = [err && err.stdout, err && err.stderr]
                .filter(Boolean)
                .map(b => b.toString())
                .join('');
            if (outStr && (/snarkjs/i.test(outStr) || /Usage:/i.test(outStr))) {
                return cmd;
            }
            if (code === 'ENOENT') {
                continue;
            }
            //其它错误：使用 --help 判定
            try {
                const helpOut = execSync(`${cmd} help`, { stdio: 'pipe' }).toString();
                if (helpOut && (/snarkjs/i.test(helpOut) || /Usage:/i.test(helpOut))) {
                    return cmd;
                }
            } catch (e2) {
                const outStr2 = [e2 && e2.stdout, e2 && e2.stderr]
                    .filter(Boolean)
                    .map(b => b.toString())
                    .join('');
                if (outStr2 && (/snarkjs/i.test(outStr2) || /Usage:/i.test(outStr2))) {
                    return cmd;
                }
                if ((e2 && e2.code) === 'ENOENT') {
                    continue;
                }
            }
        }
    }
    return null;
}

const snarkjsCmd = resolveSnarkjsCommand();
 
//若未检测到 snarkjs，则仅提示使用 JS 测试或安装依赖
if (!snarkjsCmd) {
    console.log('\n未检测到 snarkjs。');
    console.log('可选操作：');
    console.log('  1) 运行 `npm run test-js`，仅使用 JS 实现进行快速本地测试（不需要 circom/snarkjs）。');
    console.log('  2) 在本地安装 circom 与 snarkjs，然后再次运行 `npm test`。\n');
    process.exit(0);
}

//仅查找 set_result 下的固定命名文件
const dir = path.join(__dirname, 'set_result');
const vk = path.join(dir, 'verification_key_groth16.json');
const pub = path.join(dir, 'public_inputs.json');
const proof = path.join(dir, 'proof_groth16.json');

if (fs.existsSync(vk) && fs.existsSync(pub) && fs.existsSync(proof)) {
    try {
        console.log('=== 使用仓库根目录 set_result 文件进行验证 ===\n');
        execSync(`${snarkjsCmd} groth16 verify ${vk} ${pub} ${proof}`, { stdio: 'inherit' });
        console.log('\n验证通过。');
        process.exit(0);
    } catch (err) {
        console.error('\n验证失败：', err.message);
        process.exit(1);
    }
}

console.log('未发现仓库根目录的 `set_result/` 文件。');
console.log('你可以：\n  - 运行 `npm run test-js` 做快速校验；\n  - 或参考 README 按步骤编译电路与生成证明，再运行 `snarkjs groth16 verify`。');
process.exit(0);

