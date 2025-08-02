#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
快速SM4加密时间测试
"""

import time
import random
import os
import subprocess

def test_single_encryption(program_path, data_size=102400):
    """测试单次加密时间"""
    print(f"测试程序: {program_path}")
    print(f"数据大小: {data_size} 字节 ({data_size//1024}KB)")
    
    # 生成测试数据
    test_data = bytes(random.getrandbits(8) for _ in range(data_size))
    key = bytes(random.getrandbits(8) for _ in range(16))
    
    # 创建测试文件
    data_file = "quick_test_data.bin"
    key_file = "quick_test_key.bin"
    encrypted_file = "quick_test_encrypted.bin"
    
    with open(data_file, 'wb') as f:
        f.write(test_data)
    with open(key_file, 'wb') as f:
        f.write(key)
    
    # 测试加密时间
    start_time = time.time()
    try:
        result = subprocess.run([
            program_path,
            "-e", data_file, key_file, encrypted_file
        ], capture_output=True, text=True, timeout=30)
        
        end_time = time.time()
        encryption_time = (end_time - start_time) * 1000  # 转换为毫秒
        throughput = (data_size / 1024.0 / 1024.0) / (encryption_time / 1000.0)  # MB/s
        
        print(f"加密时间: {encryption_time:.2f}ms")
        print(f"吞吐量: {throughput:.2f}MB/s")
        
        if result.returncode == 0:
            print("加密成功")
        else:
            print(f"加密失败: {result.stderr}")
            
    except subprocess.TimeoutExpired:
        print("加密超时")
    except Exception as e:
        print(f"加密失败: {e}")
    
    # 清理文件
    for file in [data_file, key_file, encrypted_file]:
        if os.path.exists(file):
            os.remove(file)

def main():
    """主函数"""
    print("快速SM4加密时间测试")
    print("===================")
    
    # 设置随机种子
    random.seed(time.time())
    
    # 检查程序是否存在
    basic_program = "../SM4_Basic/x64/Debug/SM4_Basic.exe"
    aesni_program = "../SM4_AESNI/x64/Debug/SM4_AESNI.exe"
    
    if not os.path.exists(basic_program):
        print(f"错误: 找不到基本SM4程序 {basic_program}")
        print("请先编译SM4_Basic项目")
        return
    
    if not os.path.exists(aesni_program):
        print(f"错误: 找不到AESNI优化程序 {aesni_program}")
        print("请先编译SM4_AESNI项目")
        return
    
    # 测试基本SM4加密
    print("\n=== 基本SM4加密测试 ===")
    test_single_encryption(basic_program)
    
    # 测试AESNI优化SM4加密
    print("\n=== AESNI优化SM4加密测试 ===")
    test_single_encryption(aesni_program)
    
    print("\n测试完成！")

if __name__ == "__main__":
    main() 