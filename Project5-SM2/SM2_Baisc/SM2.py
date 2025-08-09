import random
from SM2_Base import a, b, Gx, Gy, G, n, ECPoint, mod_inverse
from SM3 import sm3_hash

def int_to_bytes(x: int, length: int = None) -> bytes:
    #整数转字节串，默认32字节（256位），用于密钥、坐标等
    if length is None:
        return x.to_bytes(32, byteorder='big')
    else:
        return x.to_bytes(length, byteorder='big')

def bytes_to_int(b: bytes) -> int:
    #字节串转整数（大端序）
    return int.from_bytes(b, 'big')

def precompute_Z(ID: bytes, Q: ECPoint) -> bytes:
    #计算用户标识杂凑值Z，作为签名消息杂凑的前缀
    #Z = SM3(ENTL || ID || a || b || xG || yG || xA || yA)
    #详见GB/T 35276-2017 8.1节
    ID = ID or b'1234567812345678'
    entl = len(ID) * 8
    data = int_to_bytes(entl, 2) + ID + int_to_bytes(a) + int_to_bytes(b) + \
           int_to_bytes(Gx) + int_to_bytes(Gy) + int_to_bytes(Q.x) + int_to_bytes(Q.y)
    return sm3_hash(data)

def generate_key():
    #生成SM2密钥对
    #返回：私钥d，公钥Q=dG
    k = random.randint(1, n-1)
    Q = G.multiply(k)
    return k, Q

def encrypt(Q: ECPoint, plaintext: bytes):
    #SM2加密算法。输入公钥Q和明文，输出密文三元组
    k = random.randint(1, n-1)
    kG = G.multiply(k)
    kQ = Q.multiply(k)
    x2, y2 = kQ.x, kQ.y
    t = sm3_hash(int_to_bytes(x2) + int_to_bytes(y2))
    c2 = bytes([p ^ t[i % len(t)] for i, p in enumerate(plaintext)])
    c3 = sm3_hash(int_to_bytes(x2) + plaintext + int_to_bytes(y2))
    return (kG.x, kG.y), c2, c3

def decrypt(k: int, cipher):
    #SM2解密算法。输入私钥和密文三元组，输出明文
    (x1, y1), c2, c3 = cipher
    c1 = ECPoint(x1, y1)
    kc1 = c1.multiply(k)
    x2, y2 = kc1.x, kc1.y
    t = sm3_hash(int_to_bytes(x2) + int_to_bytes(y2))
    plaintext = bytes([c ^ t[i % len(t)] for i, c in enumerate(c2)])
    if sm3_hash(int_to_bytes(x2) + plaintext + int_to_bytes(y2)) != c3:
        raise ValueError("解密失败")
    return plaintext

def sign(d: int, Q: ECPoint, message: bytes, ID: bytes = b'') -> tuple:
    #SM2数字签名算法（GB/T 35276-2017 7.2节）
    #输入：私钥d，公钥Q，消息m，用户ID
    #输出：签名(r, s)
    Z = precompute_Z(ID, Q)
    e = bytes_to_int(sm3_hash(Z + message))
    while True:
        k = random.randint(1, n-1)
        kG = G.multiply(k)
        r = (e + kG.x) % n
        if r == 0 or r + k == n:
            continue
        d1 = (1 + d) % n
        d1_inv = mod_inverse(d1, n)
        s = (d1_inv * (k - r * d)) % n
        if s != 0:
            break
    return (r, s)

def verify(Q: ECPoint, message: bytes, signature: tuple, ID: bytes = b'') -> bool:
    #SM2签名验证算法（GB/T 35276-2017 7.3节）
    #输入：公钥Q，消息m，签名(r, s)，用户ID
    #输出：验签结果True/False
    r, s = signature
    if not (1 <= r < n and 1 <= s < n):
        return False
    Z = precompute_Z(ID, Q)
    e = bytes_to_int(sm3_hash(Z + message))
    t = (r + s) % n
    if t == 0:
        return False
    sG = G.multiply(s)
    tQ = Q.multiply(t)
    P = sG + tQ
    R = (e + P.x) % n
    return R == r
