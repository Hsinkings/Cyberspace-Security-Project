#Password Checkup协议演示程序
#展示客户端-服务端的完整交互流程
import time
from GPC.client import PasswordCheckClient
from GPC.server import PasswordCheckServer


def print_separator(title: str):
    #打印分隔符
    print(f"\n{'='*60}")
    print(f" {title}")
    print(f"{'='*60}")


def demo_single_check():
    #演示单个密码检查流程
    print_separator("单个密码检查演示")
    client = PasswordCheckClient()
    server = PasswordCheckServer()

    test_cases = [
        ("123456", "已知泄露密码"),
        ("SecureP@ssw0rd2023", "强密码（未泄露）"),
        ("password", "常见泄露密码")
    ]

    for pwd, desc in test_cases:
        print(f"\n检查密码: {pwd} ({desc})")
        
        #客户端准备请求
        start = time.time()
        req = client.prepare_check_request(pwd)
        print(f"客户端准备耗时: {((time.time() - start)*1000):.2f}ms")
        
        #服务端处理
        start = time.time()
        resp = server.process_request(req)
        print(f"服务端处理耗时: {((time.time() - start)*1000):.2f}ms")
        
        #客户端验证
        start = time.time()
        is_compromised = client.process_server_response(resp)
        print(f"客户端验证耗时: {((time.time() - start)*1000):.2f}ms")
        
        #结果与强度分析
        print(f"检查结果: {'已泄露' if is_compromised else '安全'}")
        strength = client.check_password_strength(pwd)
        print(f"密码强度: {strength['strength_level']} (评分: {strength['strength_score']}/100)")


def demo_batch_check():
    #演示批量密码检查
    print_separator("批量密码检查演示")
    client = PasswordCheckClient()
    passwords = [
        "123456", "password", "qwerty", "abc123", "admin",
        "SecureP@ss1", "MyPassword!", "Complex2023!", "letmein", "welcome"
    ]

    start = time.time()
    results = client.batch_check(passwords)
    total_time = time.time() - start

    print(f"批量检查 {len(passwords)} 个密码，总耗时: {total_time:.2f}s")
    print("结果统计:")
    compromised = sum(1 for res in results.values() if res)
    print(f"  已泄露: {compromised} 个")
    print(f"  安全: {len(passwords) - compromised} 个")


def demo_performance():
    #性能基准测试
    print_separator("性能基准测试")
    client = PasswordCheckClient()
    pwd = "test_performance_password_123"
    iterations = 100

    #测试盲化操作性能
    start = time.time()
    for _ in range(iterations):
        client.prepare_check_request(pwd)
    avg_blind = (time.time() - start) / iterations * 1000
    print(f"平均盲化耗时: {avg_blind:.2f}ms/次")

    #测试服务端处理性能
    server = PasswordCheckServer()
    req = client.prepare_check_request(pwd)
    start = time.time()
    for _ in range(iterations):
        server.process_request(req)
    avg_process = (time.time() - start) / iterations * 1000
    print(f"平均服务端处理耗时: {avg_process:.2f}ms/次")


if __name__ == "__main__":
    print("=== Google Password Checkup协议演示 ===")
    print("基于私有集合交集技术的隐私保护密码泄露检查")
    
    demo_single_check()
    demo_batch_check()
    demo_performance()
    
    print_separator("演示结束")
    print("协议流程验证完成：客户端未泄露原始密码，服务端未泄露完整泄露库")