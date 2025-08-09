from SM2_Sign import SM2Signature
from SM2 import generate_key, encrypt, decrypt
import time

def test_all():
    #========== SM2加解密测试 ==========
    k, Q = generate_key()
    print("密钥对生成成功")
    plaintext = "国密SM2测试".encode('utf-8')
    cipher = encrypt(Q, plaintext)
    decrypted = decrypt(k, cipher)
    assert decrypted == plaintext, "加解密失败"
    print("加解密测试通过")

    #========== SM2签名/验签测试 ==========
    signer = SM2Signature()
    d, Q2 = signer.generate_keypair()
    print("签名密钥对生成成功")
    
    message = "SM2签名测试".encode('utf-8')
    ID = b'1234567812345678'  #确保签名和验签使用相同ID
    
    signature = signer.sign(message, d, Q2, ID)
    print(f"签名结果: r={hex(signature[0])}, s={hex(signature[1])}")
    
    result = signer.verify(message, signature, Q2, ID)
    assert result, "验签失败"
    print("签名验证测试通过")
    
    #篡改消息测试
    tampered = "SM2签名测试（已篡改）".encode('utf-8')
    assert not signer.verify(tampered, signature, Q2, ID), "篡改消息验签应失败"
    print("篡改消息验签测试通过")

def test_performance():
    #性能测试（对比优化前后）
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
    test_all()
    test_performance()