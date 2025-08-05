#include "SM3_SIMD.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>

//打印哈希值
void print_hash(const std::vector<uint8_t>& hash) {
    for (uint8_t b : hash) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    std::cout << std::dec << std::endl;
}

//性能测试函数（单路哈希）
void performance_test_single(const std::string& data, int iterations) {
    std::vector<uint8_t> digest;
    auto start = std::chrono::high_resolution_clock::now();

    SM3::batch_hash(data, digest, iterations);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "单路处理 " << iterations << " 次耗时: "
        << elapsed.count() << " 秒" << std::endl;
    std::cout << "最后一次哈希结果: ";
    print_hash(digest);
}

//性能测试函数（SIMD 8路哈希）
void performance_test_simd(const std::string& data, int iterations) {
    //准备8的倍数的测试数据
    int total = ((iterations + 7) / 8) * 8;
    std::vector<std::string> test_data(total, data);
    std::vector<std::vector<uint8_t>> digests;

    auto start = std::chrono::high_resolution_clock::now();

    SM3::batch_hash_8way(test_data, digests);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "SIMD 8路处理 " << total << " 次耗时: "
        << elapsed.count() << " 秒" << std::endl;
    std::cout << "最后一次哈希结果: ";
    print_hash(digests.back());
}

int main() {
    std::cout << "SM3哈希算法SIMD优化版本性能测试\n";

    //测试用例1：空字符串
    std::vector<uint8_t> hash1 = SM3::hash("");
    std::cout << "空字符串哈希: \n";
    print_hash(hash1);  //预期：1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b

    //测试用例2："abc"
    std::vector<uint8_t> hash2 = SM3::hash("abc");
    std::cout << "abc哈希: \n";
    print_hash(hash2);  //预期：66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0

    //性能测试
    std::cout << "\n性能测试:" << std::endl;
    std::string test_data = "abcdefghijklmnopqrstuvwxyz";
    int iterations = 100000;

    performance_test_single(test_data, iterations);
    performance_test_simd(test_data, iterations);

    return 0;
}