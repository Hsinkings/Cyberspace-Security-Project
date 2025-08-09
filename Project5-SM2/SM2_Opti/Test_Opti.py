from SM2_Sign import SM2Signature
from SM2 import generate_key, encrypt, decrypt, montgomery_mul, r_inv, r_sq, p
from SM2_Base import G, ECPoint, precomputed_G, mod_inverse

def test_montgomery_mul():
    #测试蒙哥马利模乘正确性
    a = 0x123456789abcdef0123456789abcdef0
    b = 0xfedcba9876543210fedcba9876543210
    expected = (a * b) % p
    actual = montgomery_mul(a, b)
    assert actual == expected, f"蒙哥马利模乘错误：预期{hex(expected)}，实际{hex(actual)}"
    print("蒙哥马利模乘测试通过")

def test_precomputed_table():
    #测试固定点预计算表正确性
    #验证2G = G + G
    G_double = G + G
    assert precomputed_G[1] == G_double, "预计算表索引1错误"
    #验证4G = 2G + 2G
    G_4 = precomputed_G[1] + precomputed_G[1]
    assert precomputed_G[2] == G_4, "预计算表索引2错误"
    #验证8G = 4G + 4G
    G_8 = precomputed_G[2] + precomputed_G[2]
    assert precomputed_G[3] == G_8, "预计算表索引3错误"
    print("固定点预计算表测试通过")

def test_co_z_addition():
    #测试Co-Z点加优化正确性
    P = G.multiply(3)
    Q = G.multiply(5)
    
    #统一Z坐标
    z_ratio = (P.z * mod_inverse(Q.z, p)) % p
    Q.x = (Q.x * pow(z_ratio, 2, p)) % p
    Q.y = (Q.y * pow(z_ratio, 3, p)) % p
    Q.z = P.z
    
    #比较两种点加结果
    co_z_result = P.add_co_z(Q)
    normal_result = P + Q
    assert co_z_result == normal_result, "Co-Z点加结果错误"
    print("Co-Z点加测试通过")

def test_encrypt_decrypt():
    #测试加解密流程完整性
    k, Q = generate_key()
    test_cases = [
        b"",  #空消息
        b"0123456789abcdefghijklmnopqrstuvwxyz",  #字母数字
        b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09"  #二进制数据
    ]
    
    for plaintext in test_cases:
        cipher = encrypt(Q, plaintext)
        decrypted = decrypt(k, cipher)
        assert decrypted == plaintext, f"加解密失败，明文: {plaintext}"
    print("加解密流程测试通过")

def test_sign_verify():
    #测试签名验签功能
    signer = SM2Signature()
    d, Q = signer.generate_keypair()
    message = "SM2签名验证测试".encode('utf-8')
    ID = b'user123456'
    
    #正常签名验签
    signature = signer.sign(message, d, Q, ID)
    assert signer.verify(message, signature, Q, ID), "正常验签失败"
    
    #篡改消息验签
    tampered = "SM2签名验证测试（篡改）".encode('utf-8')
    assert not signer.verify(tampered, signature, Q, ID), "篡改消息验签应失败"
    
    #错误ID验签
    wrong_id = b'user654321'
    assert not signer.verify(message, signature, Q, wrong_id), "错误ID验签应失败"
    
    print("签名验签测试通过")

def test_performance():
    #简单性能测试（对比优化前后）
    import time
    signer = SM2Signature()
    d, Q = signer.generate_keypair()
    message = b"Test Message" * 10
    
    #测试签名性能
    start = time.time()
    for _ in range(100):
        signer.sign(message, d, Q)
    sign_time = time.time() - start
    
    #测试验签性能
    signature = signer.sign(message, d, Q)
    start = time.time()
    for _ in range(100):
        signer.verify(message, signature, Q)
    verify_time = time.time() - start
    
    print(f"性能测试: 100次签名耗时{sign_time:.4f}s, 100次验签耗时{verify_time:.4f}s")

if __name__ == "__main__":
    test_montgomery_mul()
    test_precomputed_table()
    test_co_z_addition()
    test_encrypt_decrypt()
    test_sign_verify()
    test_performance()
    print("所有测试通过")
    