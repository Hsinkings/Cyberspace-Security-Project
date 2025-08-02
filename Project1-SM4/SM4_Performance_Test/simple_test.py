#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简单的SM4加密时间测试程序
"""

import time
import random
import os
import subprocess

def generate_test_data(size):
    """生成随机测试数据"""
    return bytes(random.getrandbits(8) for _ in range(size))

def create_test_files(data, key, data_file="test_data.bin", key_file="test_key.bin"):
    """创建测试文件"""
    with open(data_file, 'wb') as f:
        f.write(data)
    with open(key_file, 'wb') as f:
        f.write(key)

def cleanup_files(files):
    """清理测试文件"""
    for file in files:
        if os.path.exists(file):
            os.remove(file)

def test_encryption_time(program_path, data_size, num_tests=5):
    """测试加密时间"""
    print(f"\n测试 {program_path}")
    print(f"数据大小: {data_size} 字节 ({data_size//1024}KB)")
    print("-" * 50)
    
    total_time = 0.0
    successful_tests = 0
    
    for i in range(num_tests):
        # 生成测试数据
        test_data = generate_test_data(data_size)
        key = bytes(random.getrandbits(8) for _ in range(16))  # 16字节密钥
        
        # 创建测试文件
        data_file = f"test_data_{i}.bin"
        key_file = f"test_key_{i}.bin"
        encrypted_file = f"test_encrypted_{i}.bin"
        
        create_test_files(test_data, key, data_file, key_file)
        
        # 测试加密时间
        start_time = time.time()
        try:
            result = subprocess.run([
                program_path,
                "-e", data_file, key_file, encrypted_file
            ], capture_output=True, text=True, timeout=60)
            
            end_time = time.time()
            encryption_time = (end_time - start_time) * 1000  # 转换为毫秒
            total_time += encryption_time
            successful_tests += 1
            
            print(f"测试 {i+1}: {encryption_time:.2f}ms")
            
        except subprocess.TimeoutExpired:
            print(f"测试 {i+1}: 超时")
        except Exception as e:
            print(f"测试 {i+1}: 失败 - {e}")
        
        # 清理文件
        cleanup_files([data_file, key_file, encrypted_file])
    
    if successful_tests > 0:
        avg_time = total_time / successful_tests
        throughput = (data_size / 1024.0 / 1024.0) / (avg_time / 1000.0)  # MB/s
        print(f"\n平均加密时间: {avg_time:.2f}ms")
        print(f"吞吐量: {throughput:.2f}MB/s")
        print(f"成功测试次数: {successful_tests}/{num_tests}")
    else:
        print("所有测试都失败了")

def main():
    """主函数"""
    print("SM4加密时间测试")
    print("================")
    
    # 设置随机种子
    random.seed(time.time())
    
    # 测试数据大小
    test_sizes = [1024, 10240, 102400, 1048576]  # 1KB, 10KB, 100KB, 1MB
    
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
    for size in test_sizes:
        test_encryption_time(basic_program, size)
    
    # 测试AESNI优化SM4加密
    print("\n=== AESNI优化SM4加密测试 ===")
    for size in test_sizes:
        test_encryption_time(aesni_program, size)
    
    print("\n测试完成！")

if __name__ == "__main__":
    main() 