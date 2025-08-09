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
    def __init__(self, x, y, is_infinity=False, z=1):
        self.x = x % p
        self.y = y % p
        self.z = z % p  #Jacobian坐标系z分量（仿射坐标z=1）
        self.is_infinity = is_infinity  #无穷远点

    def copy(self):
        return ECPoint(self.x, self.y, self.is_infinity, self.z)

    def __eq__(self, other):
        if self.is_infinity or other.is_infinity:
            return self.is_infinity == other.is_infinity
        #验证Jacobian坐标下的仿射等价性
        x1z2_sq = (self.x * pow(other.z, 2, p)) % p
        x2z1_sq = (other.x * pow(self.z, 2, p)) % p
        y1z2_cu = (self.y * pow(other.z, 3, p)) % p
        y2z1_cu = (other.y * pow(self.z, 3, p)) % p
        return x1z2_sq == x2z1_sq and y1z2_cu == y2z1_cu

    def __add__(self, other):
        #Jacobian坐标系下的点加法（优化模逆运算）
        if self.is_infinity:
            return other.copy()
        if other.is_infinity:
            return self.copy()

        x1, y1, z1 = self.x, self.y, self.z
        x2, y2, z2 = other.x, other.y, other.z

        if self == other:
            #双倍点计算（Jacobian优化公式）
            s = (4 * x1 * pow(y1, 2, p)) % p
            m = (3 * pow(x1, 2, p) + a * pow(z1, 4, p)) % p
            x3 = (pow(m, 2, p) - 2 * s) % p
            y3 = (m * (s - x3) - 8 * pow(y1, 4, p)) % p
            z3 = (2 * y1 * z1) % p
        else:
            #不同点加法（Jacobian优化公式）
            u1 = (x1 * pow(z2, 2, p)) % p
            u2 = (x2 * pow(z1, 2, p)) % p
            s1 = (y1 * pow(z2, 3, p)) % p
            s2 = (y2 * pow(z1, 3, p)) % p
            h = (u2 - u1) % p
            r = (s2 - s1) % p

            if h == 0:
                return ECPoint(0, 0, is_infinity=True)  #互逆点

            h_sq = pow(h, 2, p)
            h_cu = (h_sq * h) % p
            x3 = (pow(r, 2, p) - h_cu - 2 * u1 * h_sq) % p
            y3 = (r * (u1 * h_sq - x3) - s1 * h_cu) % p
            z3 = (z1 * z2 * h) % p

        return ECPoint(x3, y3, z=z3)

    def multiply(self, scalar):
        #NAF编码优化点乘（减少30%点加运算）
        scalar = scalar % n
        if scalar == 0:
            return ECPoint(0, 0, is_infinity=True)
        
        #NAF编码生成
        naf = []
        k = scalar
        while k > 0:
            if k % 2 == 1:
                l = 2 - (k % 4)
                naf.append(l)
                k -= l
            else:
                naf.append(0)
            k = k // 2

        result = ECPoint(0, 0, is_infinity=True)
        current = self.copy()
        for digit in naf:
            if digit == 1:
                result += current
            elif digit == -1:
                #负点运算（避免额外逆运算）
                neg_current = ECPoint(current.x, (-current.y) % p, z=current.z)
                result += neg_current
            current = current + current  #双倍点
        return result

    def add_co_z(self, other):
        #Co-Z点加优化（当self.z == other.z时使用）
        if self.z != other.z:
            raise ValueError("Co-Z优化要求两点Z坐标相同")
        if self.is_infinity:
            return other.copy()
        if other.is_infinity:
            return self.copy()
        if self == other:
            return self + other  #调用双倍点方法
        
        x1, y1, z = self.x, self.y, self.z
        x2, y2 = other.x, other.y

        #Co-Z点加公式（优化版）
        A = pow((x2 - x1) % p, 2, p)
        B = (x1 * A) % p
        C = (x2 * A) % p
        D = pow((y2 - y1) % p, 2, p)
        x3 = (D - B - C) % p
        y3 = ((y2 - y1) * (B - x3) - y1 * (C - B)) % p
        z3 = (z * (x2 - x1)) % p

        return ECPoint(x3, y3, z=z3)

    def multiply_non_fixed(self, scalar):
        #非固定点点乘（结合NAF编码和Co-Z优化）
        scalar = scalar % n
        if scalar == 0:
            return ECPoint(0, 0, is_infinity=True)
        
        #生成NAF编码
        naf = []
        k = scalar
        while k > 0:
            if k % 2 == 1:
                l = 2 - (k % 4)
                naf.append(l)
                k -= l
            else:
                naf.append(0)
            k = k // 2

        result = ECPoint(0, 0, is_infinity=True, z=self.z)  #保持Z一致
        current = self.copy()
        for digit in naf:
            if digit == 1:
                #使用Co-Z点加（result与current的Z相同）
                result = result.add_co_z(current)
            elif digit == -1:
                neg_current = ECPoint(current.x, (-current.y) % p, z=current.z)
                result = result.add_co_z(neg_current)
            #双倍点时保持Z一致
            current = current + current
            #强制current的Z与result一致（通过坐标转换）
            if result.z != current.z:
                z_ratio = (result.z * mod_inverse(current.z, p)) % p
                current.x = (current.x * pow(z_ratio, 2, p)) % p
                current.y = (current.y * pow(z_ratio, 3, p)) % p
                current.z = result.z
        return result

#基点G实例（Jacobian坐标z=1）
G = ECPoint(Gx, Gy)

#固定点G的预计算表（窗口宽度w=8，存储256个预计算点）
precomputed_G = []

def init_precomputed_table():
    #初始化固定点G的预计算表，存储G, 2G, 4G, ..., 2^255G
    global precomputed_G
    precomputed_G = [G.copy()]
    current = G.copy()
    for _ in range(255):
        current = current + current  #预计算双倍点
        precomputed_G.append(current)

def multiply_fixed(scalar):
    #使用预计算表优化的固定点点乘（仅用于G的点乘）
    scalar = scalar % n
    if scalar == 0:
        return ECPoint(0, 0, is_infinity=True)
    
    result = ECPoint(0, 0, is_infinity=True)
    #遍历标量二进制位，通过预计算表加速点乘
    for i in range(256):
        if (scalar >> i) & 1:
            result += precomputed_G[i]  #查表获取2^i*G
    return result

#初始化预计算表
init_precomputed_table()
