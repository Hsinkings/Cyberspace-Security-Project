#服务端模块
#实现密码检查的服务端逻辑：管理泄露密码库、处理客户端请求
import hashlib
from typing import Dict, List, Tuple
from GPC.GPC_Base import PasswordCrypto, ECPoint, P256Curve


class PasswordCheckServer:
    def __init__(self):
        self.crypto = PasswordCrypto()
        self.curve = P256Curve()
        self.server_sk = self._generate_server_key()  #服务端私钥s
        self.breach_database: Dict[Tuple[int, int], bool] = self._load_breach_database()  #泄露密码库：H(pwd)^s -> True

    def _generate_server_key(self) -> int:
        #生成服务端私钥（1 < s < 曲线阶）
        return self.crypto.generate_blind_factor()

    def _load_breach_database(self) -> Dict[Tuple[int, int], bool]:
        #加载泄露密码库并预处理为H(pwd)^s
        #模拟泄露的密码列表（实际检测场景中应为大规模数据）
        breach_passwords = [
            "123456", "password", "qwerty", "abc123", "admin",
            "letmein", "welcome", "monkey", "dragon", "sunshine"
        ]
        db = {}
        for pwd in breach_passwords:
            #预处理：计算H(pwd)^s
            pwd_hash = hashlib.sha256(pwd.encode("utf-8")).digest()
            h_point = self.curve.hash_to_curve(pwd_hash)
            s_point = self.curve.point_multiply(self.server_sk, h_point)  #H(pwd)^s
            db[(s_point.x, s_point.y)] = True
        return db

    def process_request(self, request: Dict) -> Dict:
        #处理客户端请求：计算H(pwd)^(r*s)并返回泄露集合的子集
        #反序列化客户端发送的盲化点
        blinded_point = ECPoint(request["blinded_x"], request["blinded_y"])
        
        #服务端处理：计算H(pwd)^(r*s)
        processed_point = self.crypto.server_process(blinded_point, self.server_sk)
        
        #选取泄露集合的部分点返回
        breach_points = list(self.breach_database.keys())[:5]  #返回前5个点

        return {
            "session_id": request["session_id"],
            "processed_x": processed_point.x,
            "processed_y": processed_point.y,
            "breach_points": breach_points
        }

    def get_statistics(self) -> Dict:
        #获取服务端统计信息（辅助功能）
        return {
            "总泄露密码数": len(self.breach_database),
            "服务端密钥长度": 256,
            "曲线类型": "P-256"
        }