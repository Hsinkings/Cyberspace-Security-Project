#SM4加密性能测试程序
#测试基本SM4实现和AESNI优化版本的性能对比


import time
import random
import os
import struct
from typing import List, Tuple
import subprocess
import sys

class SM4PerformanceTest:
    def __init__(self, executables: List[str], test_files: List[str]):
        self.executables = executables
        self.test_files = test_files
        self.num_tests = 5  #每个大小测试5次取平均值
        
    def run_tests(self):
        for i, executable in enumerate(self.executables):
            print(f"Testing {executable}...")
            for j, test_file in enumerate(self.test_files):
                self.test_file_encryption(executable, test_file, f"Test{i+1}_{j+1}.bin")
    
    def test_file_encryption(self, executable: str, test_file: str, output_file: str):
        start_time = time.time()
        try:
            result = subprocess.run([
                executable,
                "-e", test_file, "G:\\Cyber_practice\\Project1\\Tests\\key.txt", output_file
            ], capture_output=True, text=True, timeout=30)
            end_time = time.time()
            time_ms = (end_time - start_time) * 1000
            print(f"Encryption time: {time_ms:.2f}ms")
        except Exception as e:
            print(f"Error: {e}")
    
    def generate_test_data(self, size: int) -> bytes:
        #生成随机测试数据
        return bytes(random.getrandbits(8) for _ in range(size))
    
    def generate_test_key(self) -> bytes:
        #生成测试密钥
        return bytes(random.getrandbits(8) for _ in range(16))  #SM4-128位密钥
    
    def create_test_file(self, filename: str, data: bytes):
        #创建测试文件
        with open(filename, 'wb') as f:
            f.write(data)


executables = ["SM4_Basic.exe", "SM4_TT.exe", "SM4_AESNI.exe"]
test_files = ["G:\\Cyber_practice\\Project1\\Tests\\plaintext.txt"]
test = SM4PerformanceTest(executables, test_files)
test.run_tests()