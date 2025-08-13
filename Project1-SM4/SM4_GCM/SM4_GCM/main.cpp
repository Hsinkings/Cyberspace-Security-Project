#include "SM4.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 8) {
        std::cerr << "用法: " << argv[0] << " <模式> <输入文件> <密钥文件> <IV文件> <AAD文件> <输出文件> <标签文件>" << std::endl;
        std::cerr << "模式: -e(加密) 或 -d(解密)" << std::endl;
        return 1;
    }

    std::string op = argv[1];
    std::string in_path = argv[2];
    std::string key_path = argv[3];
    std::string iv_path = argv[4];
    std::string aad_path = argv[5];
    std::string out_path = argv[6];
    std::string tag_path = argv[7];

    if (op == "-e") {
        if (encrypt_file_gcm(in_path, key_path, iv_path, aad_path, out_path, tag_path)) {
            std::cout << "GCM加密成功" << std::endl;
            return 0;
        }
    }
    else if (op == "-d") {
        if (decrypt_file_gcm(in_path, key_path, iv_path, aad_path, tag_path, out_path)) {
            std::cout << "GCM解密成功（标签验证通过）" << std::endl;
            return 0;
        }
        else {
            std::cerr << "解密失败（标签验证不通过）" << std::endl;
        }
    }
    else {
        std::cerr << "无效模式" << std::endl;
    }

    return 1;
}
