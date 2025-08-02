#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
SM4加密性能测试程序
测试基本SM4实现和AESNI优化版本的性能对比
"""

import time
import random
import os
import struct
from typing import List, Tuple
import subprocess
import sys

class SM4PerformanceTest:
    def __init__(self):
        self.test_sizes = [1024, 10240, 102400, 1048576]  # 1KB, 10KB, 100KB, 1MB
        self.num_tests = 5  # 每个大小测试5次取平均值
        
    def generate_test_data(self, size: int) -> bytes:
        """生成随机测试数据"""
        return bytes(random.getrandbits(8) for _ in range(size))
    
    def generate_test_key(self) -> bytes:
        """生成测试密钥"""
        return bytes(random.getrandbits(8) for _ in range(16))  # SM4使用128位密钥
    
    def create_test_file(self, filename: str, data: bytes):
        """创建测试文件"""
        with open(filename, 'wb') as f:
            f.write(data)
    
    def create_key_file(self, filename: str, key: bytes):
        """创建密钥文件"""
        with open(filename, 'wb') as f:
            f.write(key)
    
    def test_basic_sm4_performance(self):
        """测试基本SM4加密性能"""
        print("\n=== 基本SM4加密性能测试 ===")
        print(f"{'数据大小':>10} {'平均用时(ms)':>15} {'吞吐量(MB/s)':>15}")
        print("-" * 50)
        
        for data_size in self.test_sizes:
            test_data = self.generate_test_data(data_size)
            key = self.generate_test_key()
            
            # 创建测试文件
            test_file = "test_data.bin"
            key_file = "test_key.bin"
            encrypted_file = "test_encrypted.bin"
            
            self.create_test_file(test_file, test_data)
            self.create_key_file(key_file, key)
            
            total_time = 0.0
            
            for test in range(self.num_tests):
                start_time = time.time()
                
                # 调用基本SM4加密程序
                try:
                    result = subprocess.run([
                        "../SM4_Basic/x64/Debug/SM4_Basic.exe",
                        "-e", test_file, key_file, encrypted_file
                    ], capture_output=True, text=True, timeout=30)
                    
                    end_time = time.time()
                    total_time += (end_time - start_time) * 1000  # 转换为毫秒
                    
                except subprocess.TimeoutExpired:
                    print(f"测试超时: {data_size}字节")
                    break
                except Exception as e:
                    print(f"测试失败: {e}")
                    break
            
            # 清理测试文件
            for file in [test_file, key_file, encrypted_file]:
                if os.path.exists(file):
                    os.remove(file)
            
            if total_time > 0:
                avg_time = total_time / self.num_tests
                throughput = (data_size / 1024.0 / 1024.0) / (avg_time / 1000.0)  # MB/s
                
                print(f"{data_size//1024:>10}KB {avg_time:>15.2f} {throughput:>15.2f}")
    
    def test_aesni_sm4_performance(self):
        """测试AESNI优化SM4加密性能"""
        print("\n=== AESNI优化SM4加密性能测试 ===")
        print(f"{'数据大小':>10} {'平均用时(ms)':>15} {'吞吐量(MB/s)':>15}")
        print("-" * 50)
        
        for data_size in self.test_sizes:
            test_data = self.generate_test_data(data_size)
            key = self.generate_test_key()
            
            # 创建测试文件
            test_file = "test_data.bin"
            key_file = "test_key.bin"
            encrypted_file = "test_aesni_encrypted.bin"
            
            self.create_test_file(test_file, test_data)
            self.create_key_file(key_file, key)
            
            total_time = 0.0
            
            for test in range(self.num_tests):
                start_time = time.time()
                
                # 调用AESNI优化SM4加密程序
                try:
                    result = subprocess.run([
                        "../SM4_AESNI/x64/Debug/SM4_AESNI.exe",
                        "-e", test_file, key_file, encrypted_file
                    ], capture_output=True, text=True, timeout=30)
                    
                    end_time = time.time()
                    total_time += (end_time - start_time) * 1000  # 转换为毫秒
                    
                except subprocess.TimeoutExpired:
                    print(f"测试超时: {data_size}字节")
                    break
                except Exception as e:
                    print(f"测试失败: {e}")
                    break
            
            # 清理测试文件
            for file in [test_file, key_file, encrypted_file]:
                if os.path.exists(file):
                    os.remove(file)
            
            if total_time > 0:
                avg_time = total_time / self.num_tests
                throughput = (data_size / 1024.0 / 1024.0) / (avg_time / 1000.0)  # MB/s
                
                print(f"{data_size//1024:>10}KB {avg_time:>15.2f} {throughput:>15.2f}")
    
    def test_file_encryption_performance(self):
        """测试文件加密性能"""
        print("\n=== 文件加密性能测试 ===")
        
        # 生成100KB测试数据
        test_data = self.generate_test_data(102400)  # 100KB
        key = self.generate_test_key()
        
        # 创建测试文件
        test_file = "test_data.bin"
        key_file = "test_key.bin"
        encrypted_file = "test_encrypted.bin"
        
        self.create_test_file(test_file, test_data)
        self.create_key_file(key_file, key)
        
        # 测试基本SM4文件加密
        start_time = time.time()
        try:
            result = subprocess.run([
                "../SM4_Basic/x64/Debug/SM4_Basic.exe",
                "-e", test_file, key_file, encrypted_file
            ], capture_output=True, text=True, timeout=30)
            
            end_time = time.time()
            time_ms = (end_time - start_time) * 1000
            throughput = (test_data.size() / 1024.0 / 1024.0) / (time_ms / 1000.0)
            
            print("基本SM4文件加密:")
            print(f"  文件大小: {len(test_data)//1024}KB")
            print(f"  加密用时: {time_ms:.2f}ms")
            print(f"  吞吐量: {throughput:.2f}MB/s")
            
        except Exception as e:
            print(f"基本SM4文件加密测试失败: {e}")
        
        # 测试AESNI优化文件加密
        aesni_encrypted_file = "test_aesni_encrypted.bin"
        start_time = time.time()
        try:
            result = subprocess.run([
                "../SM4_AESNI/x64/Debug/SM4_AESNI.exe",
                "-e", test_file, key_file, aesni_encrypted_file
            ], capture_output=True, text=True, timeout=30)
            
            end_time = time.time()
            time_ms = (end_time - start_time) * 1000
            throughput = (test_data.size() / 1024.0 / 1024.0) / (time_ms / 1000.0)
            
            print("\nAESNI优化SM4文件加密:")
            print(f"  文件大小: {len(test_data)//1024}KB")
            print(f"  加密用时: {time_ms:.2f}ms")
            print(f"  吞吐量: {throughput:.2f}MB/s")
            
        except Exception as e:
            print(f"AESNI优化文件加密测试失败: {e}")
        
        # 清理测试文件
        for file in [test_file, key_file, encrypted_file, aesni_encrypted_file]:
            if os.path.exists(file):
                os.remove(file)
    
    def test_single_block_performance(self):
        """测试单块加密性能"""
        print("\n=== 单块加密性能测试 ===")
        
        # 生成16字节测试数据（一个SM4块）
        test_data = self.generate_test_data(16)
        key = self.generate_test_key()
        
        # 创建测试文件
        test_file = "test_block.bin"
        key_file = "test_key.bin"
        encrypted_file = "test_block_encrypted.bin"
        
        self.create_test_file(test_file, test_data)
        self.create_key_file(key_file, key)
        
        # 测试基本SM4单块加密
        total_time = 0.0
        num_iterations = 10000  # 重复10000次以获得可测量的时间
        
        for i in range(num_iterations):
            start_time = time.time()
            
            try:
                result = subprocess.run([
                    "../SM4_Basic/x64/Debug/SM4_Basic.exe",
                    "-e", test_file, key_file, encrypted_file
                ], capture_output=True, text=True, timeout=5)
                
                end_time = time.time()
                total_time += (end_time - start_time) * 1000  # 转换为毫秒
                
            except Exception as e:
                print(f"单块加密测试失败: {e}")
                break
        
        if total_time > 0:
            avg_time = total_time / num_iterations
            print(f"基本SM4单块加密平均用时: {avg_time:.6f}ms")
            print(f"单块加密速度: {1000/avg_time:.2f}块/秒")
        
        # 测试AESNI优化单块加密
        total_time = 0.0
        
        for i in range(num_iterations):
            start_time = time.time()
            
            try:
                result = subprocess.run([
                    "../SM4_AESNI/x64/Debug/SM4_AESNI.exe",
                    "-e", test_file, key_file, encrypted_file
                ], capture_output=True, text=True, timeout=5)
                
                end_time = time.time()
                total_time += (end_time - start_time) * 1000  # 转换为毫秒
                
            except Exception as e:
                print(f"AESNI单块加密测试失败: {e}")
                break
        
        if total_time > 0:
            avg_time = total_time / num_iterations
            print(f"AESNI优化单块加密平均用时: {avg_time:.6f}ms")
            print(f"单块加密速度: {1000/avg_time:.2f}块/秒")
        
        # 清理测试文件
        for file in [test_file, key_file, encrypted_file]:
            if os.path.exists(file):
                os.remove(file)
    
    def run_all_tests(self):
        """运行所有测试"""
        print("SM4加密性能测试程序")
        print("====================")
        
        # 设置随机种子
        random.seed(time.time())
        
        try:
            # 检查可执行文件是否存在
            basic_exe = "../SM4_Basic/x64/Debug/SM4_Basic.exe"
            aesni_exe = "../SM4_AESNI/x64/Debug/SM4_AESNI.exe"
            
            if not os.path.exists(basic_exe):
                print(f"错误: 找不到基本SM4程序 {basic_exe}")
                print("请先编译SM4_Basic项目")
                return
            
            if not os.path.exists(aesni_exe):
                print(f"错误: 找不到AESNI优化程序 {aesni_exe}")
                print("请先编译SM4_AESNI项目")
                return
            
            # 运行测试
            self.test_basic_sm4_performance()
            self.test_aesni_sm4_performance()
            self.test_file_encryption_performance()
            self.test_single_block_performance()
            
            print("\n测试完成！")
            
        except Exception as e:
            print(f"测试过程中发生错误: {e}")

def main():
    """主函数"""
    test = SM4PerformanceTest()
    test.run_all_tests()

if __name__ == "__main__":
    main() 