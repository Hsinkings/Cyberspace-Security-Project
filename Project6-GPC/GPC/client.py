#客户端模块
#实现密码检查的客户端逻辑：生成盲化请求、验证服务端响应
import hashlib
import uuid
from typing import Dict, Tuple, Optional
from GPC.GPC_Base import PasswordCrypto, ECPoint


class PasswordCheckClient:
    def __init__(self):
        self.crypto = PasswordCrypto()
        self.session_cache: Dict[str, Tuple[int, bytes]] = {}  #会话缓存：session_id -> (盲化因子, 原始密码哈希)

    def _hash_password(self, password: str) -> bytes:
        #密码预处理：先哈希为字节串（模拟实际场景中的密码哈希存储）
        return hashlib.sha256(password.encode("utf-8")).digest()

    def prepare_check_request(self, password: str) -> Dict:
        #准备密码检查请求：生成盲化元素和会话ID
        password_hash = self._hash_password(password)
        r = self.crypto.generate_blind_factor()  #盲化因子r
        blinded_point = self.crypto.blind(password_hash, r)  #计算H(pwd)^r

        #生成会话ID并缓存盲化因子（用于后续验证）
        session_id = str(uuid.uuid4())
        self.session_cache[session_id] = (r, password_hash)

        #序列化盲化点（实际场景中可使用ASN.1或自定义格式）
        return {
            "session_id": session_id,
            "blinded_x": blinded_point.x,
            "blinded_y": blinded_point.y
        }

    def process_server_response(self, response: Dict) -> bool:
        #处理服务端响应：去盲化并检查是否在泄露集合中
        session_id = response["session_id"]
        if session_id not in self.session_cache:
            raise ValueError("无效的会话ID")

        #从缓存中获取盲化因子和原始密码哈希
        r, password_hash = self.session_cache.pop(session_id)
        r_inv = self.crypto.inverse(r)  #计算r的逆元

        #反序列化服务端返回的点
        processed_point = ECPoint(response["processed_x"], response["processed_y"])
        breach_points = [ECPoint(x, y) for x, y in response["breach_points"]]

        #去盲化：计算H(pwd)^s = (H(pwd)^(r*s))^r⁻¹
        unblinded_point = self.crypto.unblind(processed_point, r_inv)

        #检查是否在泄露集合中
        return unblinded_point in breach_points

    def check_password_strength(self, password: str) -> Dict:
        #密码强度辅助分析（非协议核心，用于增强演示）
        score = 0
        if len(password) >= 8:
            score += 20
        if any(c.isupper() for c in password) and any(c.islower() for c in password):
            score += 20
        if any(c.isdigit() for c in password):
            score += 20
        if any(c in "!@#$%^&*()" for c in password):
            score += 20
        #检查是否为常见弱密码（简化版）
        weak_passwords = {"123456", "password", "qwerty", "admin"}
        if password not in weak_passwords:
            score += 20

        return {
            "strength_score": score,
            "strength_level": "弱" if score < 40 else "中" if score < 80 else "强"
        }

    def batch_check(self, passwords: list[str]) -> Dict[str, bool]:
        #批量检查密码（批量处理优化）
        results = {}
        for pwd in passwords:
            req = self.prepare_check_request(pwd)
            #模拟服务端调用（实际场景中为网络请求）
            from GPC.server import PasswordCheckServer
            server = PasswordCheckServer()
            resp = server.process_request(req)
            results[pwd] = self.process_server_response(resp)
        return results