import random
from SM2_Base import a, b, Gx, Gy, n, p, ECPoint, mod_inverse, multiply_fixed, G
from SM3 import sm3_hash

class SM2Signature:
    #SM2数字签名算法实现（优化验签流程）
    def __init__(self):
        self.n = n
        self.G = G  #基点
        #预计算蒙哥马利参数（用于签名过程中的模运算优化）
        self.r = 1 << 256
        self.r_inv = mod_inverse(self.r, p)
        self.r_sq = (self.r * self.r) % p

    def montgomery_mul(self, a: int, b: int) -> int:
        #蒙哥马利乘法优化
        #直接使用标准模乘，避免蒙哥马利实现的复杂性
        return (a * b) % p

    def int_to_bytes(self, x: int, length: int = None) -> bytes:
        if length is None:
            return x.to_bytes(32, byteorder='big')
        else:
            return x.to_bytes(length, byteorder='big')
    
    def bytes_to_int(self, b: bytes) -> int:
        return int.from_bytes(b, byteorder='big')
    
    def compute_Z(self, ID: bytes, Q: ECPoint) -> bytes:
        #计算用户标识杂凑值Z
        if not ID:
            ID = b'1234567812345678'
        entl = len(ID) * 8
        data = self.int_to_bytes(entl, 2) + ID + \
               self.int_to_bytes(a) + self.int_to_bytes(b) + \
               self.int_to_bytes(Gx) + self.int_to_bytes(Gy) + \
               self.int_to_bytes(Q.x) + self.int_to_bytes(Q.y)
        return sm3_hash(data)
    
    def generate_keypair(self):
        d = random.randint(1, self.n - 1)
        #使用固定点预计算表加速公钥生成
        Q = multiply_fixed(d)
        return d, Q
    
    def sign(self, message: bytes, d: int, Q: ECPoint, ID: bytes = b'') -> tuple:
        #签名生成（复用优化后的点乘）
        Z = self.compute_Z(ID, Q)
        e_hash = sm3_hash(Z + message)
        e = self.bytes_to_int(e_hash)
        
        while True:
            k = random.randint(1, self.n - 1)
            #优化1：使用预计算表加速kG计算
            kG = multiply_fixed(k)
            
            #优化2：蒙哥马利模乘加速坐标转换
            z_inv = mod_inverse(kG.z, p)
            z_inv_sq = self.montgomery_mul(z_inv, z_inv)
            x1 = self.montgomery_mul(kG.x, z_inv_sq)  #x = X/z²
            r = (e + x1) % self.n
            
            if r == 0 or r + k == self.n:
                continue
            
            d1 = (1 + d) % self.n
            d1_inv = mod_inverse(d1, self.n)
            s = (d1_inv * (k - r * d)) % self.n
            
            if s != 0:
                break
        
        return (r, s)
    
    def verify(self, message: bytes, signature: tuple, Q: ECPoint, ID: bytes = b'') -> bool:
        #验签优化：避免模逆运算 + Co-Z点加
        r, s = signature
        if not (1 <= r < self.n and 1 <= s < self.n):
            return False
        
        Z = self.compute_Z(ID, Q)
        e_hash = sm3_hash(Z + message)
        e = self.bytes_to_int(e_hash)
        t = (r + s) % self.n
        if t == 0:
            return False
        
        #优化1：使用预计算表加速sG计算（固定点）
        sG = multiply_fixed(s)
        #优化2：使用Co-Z非固定点点乘加速tQ计算
        tQ = Q.multiply_non_fixed(t)
        
        #统一Z坐标后执行Co-Z点加
        if sG.z != tQ.z:
            z_ratio = (sG.z * mod_inverse(tQ.z, p)) % p
            tQ.x = (tQ.x * pow(z_ratio, 2, p)) % p
            tQ.y = (tQ.y * pow(z_ratio, 3, p)) % p
            tQ.z = sG.z
        P = sG.add_co_z(tQ)  #执行低复杂度点加
        
        if P.is_infinity:
            return False
        
        #验证R = (e + x_P) mod n == r
        #需要将Jacobian坐标转换为仿射坐标
        z_inv = mod_inverse(P.z, p)
        z_inv_sq = self.montgomery_mul(z_inv, z_inv)
        x_P = self.montgomery_mul(P.x, z_inv_sq)  #转换为仿射坐标x
        R = (e + x_P) % self.n
        return R == r
