#SM2椭圆曲线参数（GB/T 35276-2017）
p = 0x8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3
a = 0x787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498
b = 0x63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A
n = 0x8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7  #阶
Gx = 0x421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D
Gy = 0x0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2

def mod_inverse(x, mod):
    #计算x关于mod的乘法逆元
    return pow(x, -1, mod)

class ECPoint:
    def __init__(self, x, y, is_infinity=False):
        self.x = x % p
        self.y = y % p
        self.is_infinity = is_infinity  #无穷远点

    def __add__(self, other):
        #椭圆曲线点加法，支持一般点加和双倍点
        #严格遵循椭圆曲线加法规则：
        #- O + P = P
        #- P + O = P
        #- P + (-P) = O
        #- P + P = 2P（双倍点）
        #- P + Q = R（一般点加）
        if self.is_infinity:
            return other
        if other.is_infinity:
            return self
        if self.x == other.x:
            if (self.y + other.y) % p == 0:
                #P + (-P) = O
                return ECPoint(0, 0, is_infinity=True)
            #双倍点：P == Q
            k = ((3 * self.x ** 2 + a) * mod_inverse(2 * self.y, p)) % p
        else:
            #一般点加
            k = ((other.y - self.y) * mod_inverse((other.x - self.x) % p, p)) % p
        x3 = (k ** 2 - self.x - other.x) % p
        y3 = (k * (self.x - x3) - self.y) % p
        return ECPoint(x3, y3)

    def multiply(self, scalar):
        #椭圆曲线点乘，二进制展开法
        result = ECPoint(0, 0, is_infinity=True)
        current = self
        scalar = scalar % n  #模n约简
        while scalar > 0:
            if scalar & 1:
                result += current
            current += current  #双倍点
            scalar >>= 1
        return result

#基点G实例
G = ECPoint(Gx, Gy)