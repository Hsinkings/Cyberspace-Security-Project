#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <ctime>

//包含SM4实现
#include "../SM4_Basic/SM4/SM4.h"
#include "../SM4_AESNI/SM4_AESNI/SM4_AESNI.h"

//测试数据大小（字节）
const size_t TEST_SIZES[] = {1024, 10240, 102400, 1048576}; //1KB, 10KB, 100KB, 1MB
const int NUM_TESTS = 5; //每个大小测试5次取平均值

//生成随机测试数据
std::vector<uint8_t> generate_test_data(size_t size) {
    std::vector<uint8_t> data(size);
    for (size_t i = 0; i < size; i++) {
        data[i] = static_cast<uint8_t>(rand() % 256);
    }
    return data;
}

//生成测试密钥
std::vector<uint8_t> generate_test_key() {
    std::vector<uint8_t> key(16); //SM4使用128位密钥
    for (int i = 0; i < 16; i++) {
        key[i] = static_cast<uint8_t>(rand() % 256);
    }
    return key;
}

//测试基本SM4加密性能
void test_basic_sm4_performance() {
    std::cout << "\n=== 基本SM4加密性能测试 ===" << std::endl;
    std::cout << std::setw(10) << "数据大小" << std::setw(15) << "平均用时(ms)" 
              << std::setw(15) << "吞吐量(MB/s)" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    for (size_t data_size : TEST_SIZES) {
        std::vector<uint8_t> test_data = generate_test_data(data_size);
        std::vector<uint8_t> key = generate_test_key();
        std::vector<uint8_t> ciphertext(data_size);
        
        //准备密钥
        uint32_t key_words[4];
        memcpy(key_words, key.data(), 16);
        
        double total_time = 0.0;
        
        for (int test = 0; test < NUM_TESTS; test++) {
            auto start = std::chrono::high_resolution_clock::now();
            
            //分块加密
            size_t blocks = data_size / 16;
            for (size_t i = 0; i < blocks; i++) {
                uint32_t plaintext_block[4], ciphertext_block[4];
                memcpy(plaintext_block, test_data.data() + i * 16, 16);
                sm4_encrypt(plaintext_block, key_words, ciphertext_block);
                memcpy(ciphertext.data() + i * 16, ciphertext_block, 16);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            total_time += duration.count() / 1000.0; //转换为毫秒
        }
        
        double avg_time = total_time / NUM_TESTS;
        double throughput = (data_size / 1024.0 / 1024.0) / (avg_time / 1000.0); //MB/s
        
        std::cout << std::setw(10) << (data_size / 1024) << "KB" 
                  << std::setw(15) << std::fixed << std::setprecision(2) << avg_time
                  << std::setw(15) << std::fixed << std::setprecision(2) << throughput << std::endl;
    }
}

//测试AESNI优化的SM4加密性能
void test_aesni_sm4_performance() {
    std::cout << "\n=== AESNI优化SM4加密性能测试 ===" << std::endl;
    std::cout << std::setw(10) << "数据大小" << std::setw(15) << "平均用时(ms)" 
              << std::setw(15) << "吞吐量(MB/s)" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    for (size_t data_size : TEST_SIZES) {
        std::vector<uint8_t> test_data = generate_test_data(data_size);
        std::vector<uint8_t> key = generate_test_key();
        std::vector<uint8_t> ciphertext(data_size);
        
        //密钥扩展
        uint32_t round_keys[32];
        uint32_t key_words[4];
        memcpy(key_words, key.data(), 16);
        SM4_keyexpansion(key_words, round_keys);
        
        double total_time = 0.0;
        
        for (int test = 0; test < NUM_TESTS; test++) {
            auto start = std::chrono::high_resolution_clock::now();
            
            //使用AESNI优化的加密函数
            SM4_AESNI_enc(test_data.data(), ciphertext.data(), data_size, round_keys);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            total_time += duration.count() / 1000.0; //转换为毫秒
        }
        
        double avg_time = total_time / NUM_TESTS;
        double throughput = (data_size / 1024.0 / 1024.0) / (avg_time / 1000.0); //MB/s
        
        std::cout << std::setw(10) << (data_size / 1024) << "KB" 
                  << std::setw(15) << std::fixed << std::setprecision(2) << avg_time
                  << std::setw(15) << std::fixed << std::setprecision(2) << throughput << std::endl;
    }
}

//测试文件加密性能
void test_file_encryption_performance() {
    std::cout << "\n=== 文件加密性能测试 ===" << std::endl;
    
    //创建测试文件
    std::string test_file = "test_data.bin";
    std::string key_file = "test_key.bin";
    std::string encrypted_file = "test_encrypted.bin";
    
    //生成测试数据文件
    std::ofstream data_file(test_file, std::ios::binary);
    std::vector<uint8_t> test_data = generate_test_data(102400); //100KB
    data_file.write(reinterpret_cast<const char*>(test_data.data()), test_data.size());
    data_file.close();
    
    //生成密钥文件
    std::ofstream key_out(key_file, std::ios::binary);
    std::vector<uint8_t> key = generate_test_key();
    key_out.write(reinterpret_cast<const char*>(key.data()), key.size());
    key_out.close();
    
    //测试基本SM4文件加密
    auto start = std::chrono::high_resolution_clock::now();
    bool success = encrypt_file(test_file, key_file, encrypted_file);
    auto end = std::chrono::high_resolution_clock::now();
    
    if (success) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double time_ms = duration.count() / 1000.0;
        double throughput = (test_data.size() / 1024.0 / 1024.0) / (time_ms / 1000.0);
        
        std::cout << "基本SM4文件加密:" << std::endl;
        std::cout << "  文件大小: " << (test_data.size() / 1024) << "KB" << std::endl;
        std::cout << "  加密用时: " << std::fixed << std::setprecision(2) << time_ms << "ms" << std::endl;
        std::cout << "  吞吐量: " << std::fixed << std::setprecision(2) << throughput << "MB/s" << std::endl;
    }
    
    //测试AESNI优化文件加密
    std::string aesni_encrypted_file = "test_aesni_encrypted.bin";
    start = std::chrono::high_resolution_clock::now();
    success = encrypt_file(test_file, key_file, aesni_encrypted_file);
    end = std::chrono::high_resolution_clock::now();
    
    if (success) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double time_ms = duration.count() / 1000.0;
        double throughput = (test_data.size() / 1024.0 / 1024.0) / (time_ms / 1000.0);
        
        std::cout << "\nAESNI优化SM4文件加密:" << std::endl;
        std::cout << "  文件大小: " << (test_data.size() / 1024) << "KB" << std::endl;
        std::cout << "  加密用时: " << std::fixed << std::setprecision(2) << time_ms << "ms" << std::endl;
        std::cout << "  吞吐量: " << std::fixed << std::setprecision(2) << throughput << "MB/s" << std::endl;
    }
    
    //清理测试文件
    remove(test_file.c_str());
    remove(key_file.c_str());
    remove(encrypted_file.c_str());
    remove(aesni_encrypted_file.c_str());
}

int main() {
    std::cout << "SM4加密性能测试程序" << std::endl;
    std::cout << "====================" << std::endl;
    
    //设置随机种子
    srand(static_cast<unsigned int>(time(nullptr)));
    
    try {
        //测试基本SM4加密性能
        test_basic_sm4_performance();
        
        //测试AESNI优化SM4加密性能
        test_aesni_sm4_performance();
        
        //测试文件加密性能
        test_file_encryption_performance();
        
        std::cout << "\n测试完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "测试过程中发生错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 