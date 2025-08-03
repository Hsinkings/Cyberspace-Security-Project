#SM4加密性能测试程序
#测试基本SM4、Ttable优化以及AESNI优化算法的性能对比
import time
import random
import os
from typing import List
import subprocess

class SM4PerformanceTest:
    def __init__(self, executables: List[str], test_files: List[str]):
        self.executables = executables
        self.test_files = test_files
        self.num_tests = 5  #每个大小测试5次取平均值
        self.generated_files = []  #记录生成的临时文件
        
    def run_tests(self):
        for i, executable in enumerate(self.executables):
            print(f"Testing {executable}...")
            for j, test_file in enumerate(self.test_files):
                output_file = f"Test{i+1}_{j+1}.bin"
                self.test_file_encryption(executable, test_file, output_file)
        
        #测试完成后清理生成的临时文件
        self.cleanup_files()
    
    def test_file_encryption(self, executable: str, test_file: str, output_file: str):
        start_time = time.time()
        try:
            #使用相对路径构建完整路径
            executable_path = os.path.join(os.path.dirname(__file__), executable)
            test_file_path = os.path.join(os.path.dirname(__file__), "Tests", test_file)
            key_file_path = os.path.join(os.path.dirname(__file__), "Tests", "key.txt")
            output_file_path = os.path.join(os.path.dirname(__file__), output_file)
            
            #检查文件是否存在
            if not os.path.exists(executable_path):
                print(f"Error: Executable {executable} not found at {executable_path}")
                return
            if not os.path.exists(test_file_path):
                print(f"Error: Test file {test_file} not found at {test_file_path}")
                return
            if not os.path.exists(key_file_path):
                print(f"Error: Key file not found at {key_file_path}")
                return
            
            result = subprocess.run([
                executable_path,
                "-e", test_file_path, key_file_path, output_file_path
            ], capture_output=True, text=True, timeout=30)
            
            end_time = time.time()
            time_ms = (end_time - start_time) * 1000
            print(f"Encryption time: {time_ms:.2f}ms")
            
            #记录生成的输出文件以便后续删除
            if os.path.exists(output_file_path):
                self.generated_files.append(output_file_path)
                
        except subprocess.TimeoutExpired:
            print(f"Error: Timeout while running {executable}")
        except Exception as e:
            print(f"Error: {e}")
    
    def cleanup_files(self):
        #清理生成的临时文件
        for file_path in self.generated_files:
            try:
                if os.path.exists(file_path):
                    os.remove(file_path)
                    print(f"Cleaned up: {file_path}")
            except Exception as e:
                print(f"Error cleaning up {file_path}: {e}")
    
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


#使用相对路径的文件列表便于github代码迁移
executables = ["SM4_Basic.exe", "SM4_TT.exe", "SM4_AESNI.exe"]
test_files = ["plaintext.txt"]  #Tests文件夹相对路径（参考github代码库。能够正常运行）
test = SM4PerformanceTest(executables, test_files)
test.run_tests()
