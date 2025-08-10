#密码学工具模块
#实现椭圆曲线运算、哈希映射到曲线、盲化/去盲化等核心操作
#基于P-256椭圆曲线（NIST P-256）
import hashlib
import os
from typing import Tuple

#椭圆曲线参数（P-256）
P = 0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF
A = 0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC
B = 0x5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B
Gx = 0x6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296
Gy = 0x4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5
ORDER = 0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551


class ECPoint:
    #椭圆曲线上的点
    def __init__(self, x: int, y: int, is_infinite: bool = False):
        self.x = x
        self.y = y
        self.is_infinite = is_infinite

    def __eq__(self, other) -> bool:
        if self.is_infinite:
            return other.is_infinite
        return self.x == other.x and self.y == other.y

    def __repr__(self) -> str:
        if self.is_infinite:
            return "ECPoint(infinite)"
        return f"ECPoint(0x{self.x:x}, 0x{self.y:x})"


class P256Curve:
    #P-256椭圆曲线运算实现
    def __init__(self):
        self.G = ECPoint(Gx, Gy)  #生成元
        self.order = ORDER

    def point_add(self, p: ECPoint, q: ECPoint) -> ECPoint:
        #椭圆曲线点加法
        if p.is_infinite:
            return q
        if q.is_infinite:
            return p
        if p.x == q.x and p.y != q.y:
            return ECPoint(0, 0, True)  #无穷远点

        if p != q:
            lam = (q.y - p.y) * pow(q.x - p.x, P-2, P) % P
        else:
            lam = (3 * p.x * p.x + A) * pow(2 * p.y, P-2, P) % P

        x = (lam * lam - p.x - q.x) % P
        y = (lam * (p.x - x) - p.y) % P
        return ECPoint(x, y)

    def point_multiply(self, scalar: int, p: ECPoint) -> ECPoint:
        #椭圆曲线标量乘法（二进制扩展法）
        result = ECPoint(0, 0, True)  #初始为无穷远点
        current = p
        while scalar > 0:
            if scalar % 2 == 1:
                result = self.point_add(result, current)
            current = self.point_add(current, current)
            scalar = scalar // 2
        return result

    def hash_to_curve(self, data: bytes) -> ECPoint:
        #哈希映射到椭圆曲线（概率性方法）
        counter = 0
        while True:
            #哈希数据+计数器，确保找到曲线上的点
            h = hashlib.sha256(data + counter.to_bytes(4, "big")).digest()
            x = int.from_bytes(h, "big") % P
            #求解y² = x³ + A x + B (mod P)
            y_sq = (x * x * x + A * x + B) % P
            y = pow(y_sq, (P + 1) // 4, P)  #二次剩余求解（P ≡ 3 mod 4）
            if (y * y) % P == y_sq:
                return ECPoint(x, y)
            counter += 1


class PasswordCrypto:
    #密码检查协议的密码学工具
    def __init__(self):
        self.curve = P256Curve()

    def generate_blind_factor(self) -> int:
        #生成盲化因子（1 < r < 曲线阶）
        return int.from_bytes(os.urandom(32), "big") % (self.curve.order - 2) + 1

    def blind(self, data: bytes, r: int) -> ECPoint:
        #盲化操作：H(password)^r
        h_point = self.curve.hash_to_curve(data)
        return self.curve.point_multiply(r, h_point)

    def server_process(self, blinded_point: ECPoint, server_sk: int) -> ECPoint:
        #服务端处理：(H(password)^r)^s = H(password)^(r*s)
        return self.curve.point_multiply(server_sk, blinded_point)

    def unblind(self, processed_point: ECPoint, r_inv: int) -> ECPoint:
        #去盲化操作：H(password)^(r*s)^(r⁻¹) = H(password)^s
        return self.curve.point_multiply(r_inv, processed_point)

    def inverse(self, scalar: int) -> int:
        #计算标量的逆元（模曲线阶）
        return pow(scalar, self.curve.order - 2, self.curve.order)