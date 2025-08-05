#include "SM3_Unrol.h"
#include <iostream>
#include <iomanip>
#include <chrono>

//打印哈希值
void print_hash(const std::vector<uint8_t>& hash) {
    for (uint8_t b : hash) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    std::cout << std::dec << std::endl;
}

//性能测试函数
void performance_test(const std::string& data, int iterations) {
    std::vector<uint8_t> digest;
    auto start = std::chrono::high_resolution_clock::now();

    SM3::batch_hash(data, digest, iterations);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "批量处理 " << iterations << " 次耗时: "
        << elapsed.count() << " 秒" << std::endl;
    std::cout << "最后一次哈希结果: ";
    print_hash(digest);
}

int main() {
    std::cout << "SM3哈希算法优化（循环展开、宏定义优化版本）性能测试\n";
    //测试向量1：空字符串
    std::vector<uint8_t> hash1 = SM3::hash("");
    std::cout << "空字符串哈希: \n";
    print_hash(hash1);  //预期：1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b

    //测试向量2："abc"
    std::vector<uint8_t> hash2 = SM3::hash("abc");
    std::cout << "abc哈希: \n";
    print_hash(hash2);  //预期：66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0

    //性能测试（100000次哈希计算）
    std::cout << "\n性能测试:" << std::endl;
    performance_test("abcdefghijklmnopqrstuvwxyz", 100000);

    return 0;
}