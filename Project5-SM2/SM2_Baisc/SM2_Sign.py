import random
from SM2_Base import a, b, Gx, Gy, n, ECPoint, mod_inverse
from SM3 import sm3_hash

class SM2Signature:
    #SM2数字签名算法实现，严格遵循国标GB/T 35276-2017
    #包含密钥生成、签名、验签功能
    
    def __init__(self):
        self.n = n
        self.G = ECPoint(Gx, Gy)
    
    def int_to_bytes(self, x: int, length: int = None) -> bytes:
        #整数转字节串
        if length is None:
            return x.to_bytes(32, byteorder='big')
        else:
            return x.to_bytes(length, byteorder='big')
    
    def bytes_to_int(self, b: bytes) -> int:
        #字节串转整数
        return int.from_bytes(b, byteorder='big')
    
    def compute_Z(self, ID: bytes, Q: ECPoint) -> bytes:
        #计算用户标识杂凑值Z
        #Z = SM3(ENTL || ID || a || b || xG || yG || xA || yA)
        if not ID:
            ID = b'1234567812345678'
        entl = len(ID) * 8
        data = self.int_to_bytes(entl, 2) + ID + \
               self.int_to_bytes(a) + self.int_to_bytes(b) + \
               self.int_to_bytes(Gx) + self.int_to_bytes(Gy) + \
               self.int_to_bytes(Q.x) + self.int_to_bytes(Q.y)
        return sm3_hash(data)
    
    def generate_keypair(self):
        #生成SM2密钥对
        d = random.randint(1, self.n - 1)
        Q = self.G.multiply(d)
        return d, Q
    
    def sign(self, message: bytes, d: int, Q: ECPoint, ID: bytes = b'') -> tuple:
        #SM2数字签名生成
        #输入：消息m，私钥d，公钥Q，用户ID
        #输出：签名(r, s)
        #1. 计算Z值
        Z = self.compute_Z(ID, Q)
        
        #2. 计算e = SM3(Z || M)
        e_hash = sm3_hash(Z + message)
        e = self.bytes_to_int(e_hash)
        
        #3. 生成随机数k并计算签名
        while True:
            k = random.randint(1, self.n - 1)
            kG = self.G.multiply(k)
            r = (e + kG.x) % self.n
            
            if r == 0 or r + k == self.n:
                continue
            
            #计算s = ((1 + d)^-1 * (k - r * d)) mod n
            d1 = (1 + d) % self.n
            d1_inv = mod_inverse(d1, self.n)
            s = (d1_inv * (k - r * d)) % self.n
            
            if s != 0:
                break
        
        return (r, s)
    
    def verify(self, message: bytes, signature: tuple, Q: ECPoint, ID: bytes = b'') -> bool:
        #SM2签名验证
        #输入：消息m，签名(r, s)，公钥Q，用户ID
        #输出：验证结果True/False
        r, s = signature
        
        #1. 验证r, s范围
        if not (1 <= r < self.n and 1 <= s < self.n):
            return False
        
        #2. 计算Z值和e值
        Z = self.compute_Z(ID, Q)
        e_hash = sm3_hash(Z + message)
        e = self.bytes_to_int(e_hash)
        
        #3. 计算t = (r + s) mod n
        t = (r + s) % self.n
        if t == 0:
            return False
        
        #4. 计算点P = sG + tQ
        sG = self.G.multiply(s)
        tQ = Q.multiply(t)
        P = sG + tQ
        
        if P.is_infinity:
            return False
        
        #5. 验证R = (e + x_P) mod n == r
        R = (e + P.x) % self.n
        return R == r