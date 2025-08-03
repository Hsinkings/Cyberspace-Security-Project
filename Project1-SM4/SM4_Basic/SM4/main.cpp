#include "SM4.h"
#include <iostream>
#include <string>

//确保函数声明存在
bool encrypt_file(const std::string& in_path, const std::string& key_path, const std::string& out_path);
bool decrypt_file(const std::string& in_path, const std::string& key_path, const std::string& out_path);

int main(int argc, char* argv[]) {
    //检查命令行参数数量
    if (argc != 5) {
        std::cerr << "用法: " << argv[0] << " <操作模式> <输入文件> <密钥文件> <输出文件>" << std::endl;
        std::cerr << "操作模式: -e(加密) 或 -d(解密)" << std::endl;
        return 1;
    }

    std::string op = argv[1];
    std::string in_path = argv[2];
    std::string key_path = argv[3];
    std::string out_path = argv[4];

    //处理加密操作
    if (op == "-e") {
        if (encrypt_file(in_path, key_path, out_path)) {
            std::cout << "加密成功，输出文件: " << out_path << std::endl;
            return 0;
        }
    }
    //处理解密操作
    else if (op == "-d") {
        if (decrypt_file(in_path, key_path, out_path)) {
            std::cout << "解密成功，输出文件: " << out_path << std::endl;
            return 0;
        }
    }
    //处理无效操作模式
    else {
        std::cerr << "无效操作模式，请使用 -e 或 -d" << std::endl;
        return 1;
    }

    std::cerr << "操作失败" << std::endl;
    return 1;
}
