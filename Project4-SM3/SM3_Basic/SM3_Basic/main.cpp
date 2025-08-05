#include "SM3.h"
#include <iostream>
#include <iomanip>
#include <chrono>

//打印哈希值
void print_hash(const std::vector<uint8_t>& hash) {
    for (uint8_t b : hash) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b;
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
    std::cout << "SM3哈希算法基础版本性能测试\n";
    //测试用例1："abc"
    std::vector<uint8_t> hash2 = SM3::hash("abc");
    std::cout << "abc哈希: \n";
    print_hash(hash2);  //应该输出66C7F0F462EEEDD9D1F2D46BDC10E4E24167C4875CF2F7A2297DA02B8F4BA8E0

    //测试用例2："SDUQDWLAQ2022QC"
    std::vector<uint8_t> hash1 = SM3::hash("");
    std::cout << "\nSDUQDWLAQ2022QC（随机选定示例字符串）哈希: \n";
    print_hash(hash1);

    //性能测试（10000次哈希计算）
    std::cout << "\n性能测试:" << std::endl;
    performance_test("abcdefghijklmnopqrstuvwxyz", 100000);

    return 0;
}