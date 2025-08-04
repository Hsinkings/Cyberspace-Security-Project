import os
import numpy as np
from Watermark_Attacks.Watermark_attacks import ImageAttacker
from Robustness_Test.Image_eva import QualityMetrics

class RobustnessTester:
    #鲁棒性测试器类
    #对含水印图像执行多种攻击，评估图像水印的鲁棒性
    def __init__(self, watermark_core):
        #初始化鲁棒性测试器
        #watermark_core: 水印核心算法对象
        self.attacker = ImageAttacker()
        self.watermark_core = watermark_core
        self.metrics = QualityMetrics()

    def run_single_attack(self, watermarked_img, watermark_paths, attack_name, attack_params, output_dir):
        #执行单个攻击并评估结果
        #watermarked_img: 含水印图像数组
        #watermark_paths: 候选水印图像路径列表
        #attack_name: 攻击方法名称
        #attack_params: 攻击参数字典
        #output_dir: 输出目录
        #返回值: 攻击结果字典
        
        #获取攻击函数并执行攻击
        attack_func = getattr(self.attacker, attack_name, None)
        if not attack_func:
            raise ValueError(f"攻击方式不存在: {attack_name}")
        attacked_img = attack_func(watermarked_img, **attack_params)

        #保存攻击后的图像
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"{attack_name}_attacked.png")
        from Watermark.Watermark_imageload import ImageUtils  #避免循环导入
        ImageUtils.save_image(attacked_img, output_path)

        #检测水印并评估相似度
        best_match, similarity = self.watermark_core.detect_watermark(output_path, watermark_paths)
        psnr = self.metrics.calculate_psnr(watermarked_img, attacked_img)

        return {
            "success": True,
            "attack_name": attack_name,
            "parameters": attack_params,
            "output_path": output_path,
            "detected_watermark": os.path.basename(best_match) if best_match else None,
            "similarity": similarity,
            "psnr": psnr
        }

    def run_multiple_attacks(self, watermarked_path, watermark_paths, output_dir="output/attacks"):
        #执行多种攻击并生成汇总结果
        #watermarked_path: 含水印图像路径
        #watermark_paths: 候选水印图像路径列表
        #output_dir: 输出目录
        #返回值: 所有攻击结果列表
        
        from Watermark.Watermark_imageload import ImageUtils  #避免循环导入
        watermarked_img = ImageUtils.load_image(watermarked_path)

        #定义攻击配置（覆盖翻转、平移、裁剪等要求）
        #包含几何攻击、滤波攻击、噪声攻击、压缩攻击等多种类型
        attack_configs = [
            ("horizontal_flip", {}),
            ("vertical_flip", {}),
            ("rotate", {"angle": 5}),  #减小旋转角度
            ("rotate", {"angle": 10}),  #减小旋转角度
            ("translate", {"dx": 5, "dy": 5}),  #减小平移距离
            ("crop", {"ratio": 0.9}),  #增加裁剪比例
            ("random_crop", {"ratio": 0.85}),  #增加裁剪比例
            ("gaussian_blur", {"kernel": 3}),  #减小模糊核大小
            ("gaussian_noise", {"std": 5}),  #减小噪声强度
            ("adjust_contrast", {"factor": 1.2}),  #减小对比度调整
            ("adjust_contrast", {"factor": 0.8}),  #减小对比度调整
            ("adjust_brightness", {"factor": 1.1}),  #减小亮度调整
            ("adjust_brightness", {"factor": 0.9}),  #减小亮度调整
            ("jpeg_compress", {"quality": 80}),  #提高JPEG质量
        ]

        results = []
        for attack_name, params in attack_configs:
            try:
                #执行单个攻击测试
                res = self.run_single_attack(
                    watermarked_img, watermark_paths, attack_name, params, output_dir
                )
                results.append(res)
                print(f"完成攻击: {attack_name} (相似度: {res['similarity']:.4f})")
            except Exception as e:
                #记录攻击失败的情况
                results.append({
                    "success": False,
                    "attack_name": attack_name,
                    "error": str(e)
                })
                print(f"攻击失败: {attack_name} (错误: {e})")

        return results

    def generate_report(self, results, output_file="robustness_report.txt"):
        #生成鲁棒性测试报告
        #results: 攻击结果列表
        #output_file: 输出报告文件路径
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("图像水印鲁棒性测试报告\n")
            f.write("=" * 50 + "\n\n")
            
            #统计信息
            successful_attacks = [r for r in results if r.get('success', False)]
            failed_attacks = [r for r in results if not r.get('success', False)]
            
            f.write(f"总攻击次数: {len(results)}\n")
            f.write(f"成功攻击次数: {len(successful_attacks)}\n")
            f.write(f"失败攻击次数: {len(failed_attacks)}\n\n")
            
            #成功攻击的详细结果
            if successful_attacks:
                f.write("成功攻击结果:\n")
                f.write("-" * 30 + "\n")
                
                #按相似度排序
                successful_attacks.sort(key=lambda x: x.get('similarity', 0), reverse=True)
                
                for res in successful_attacks:
                    f.write(f"攻击方式: {res['attack_name']}\n")
                    f.write(f"检测到的水印: {res.get('detected_watermark', 'None')}\n")
                    f.write(f"相似度: {res.get('similarity', 0):.4f}\n")
                    f.write(f"PSNR: {res.get('psnr', 0):.2f} dB\n")
                    f.write(f"参数: {res.get('parameters', {})}\n")
                    f.write("-" * 20 + "\n")
            
            #失败攻击的记录
            if failed_attacks:
                f.write("\n失败攻击记录:\n")
                f.write("-" * 30 + "\n")
                for res in failed_attacks:
                    f.write(f"攻击方式: {res['attack_name']}\n")
                    f.write(f"错误信息: {res.get('error', 'Unknown error')}\n")
                    f.write("-" * 20 + "\n")
            
            #总结
            f.write("\n总结:\n")
            f.write("-" * 30 + "\n")
            
            if successful_attacks:
                avg_similarity = np.mean([r.get('similarity', 0) for r in successful_attacks])
                avg_psnr = np.mean([r.get('psnr', 0) for r in successful_attacks])
                f.write(f"平均相似度: {avg_similarity:.4f}\n")
                f.write(f"平均PSNR: {avg_psnr:.2f} dB\n")
                
                #鲁棒性评估
                high_similarity_count = len([r for r in successful_attacks if r.get('similarity', 0) > 0.7])
                medium_similarity_count = len([r for r in successful_attacks if 0.5 < r.get('similarity', 0) <= 0.7])
                low_similarity_count = len([r for r in successful_attacks if r.get('similarity', 0) <= 0.5])
                
                f.write(f"高相似度攻击数 (>0.7): {high_similarity_count}\n")
                f.write(f"中等相似度攻击数 (0.5-0.7): {medium_similarity_count}\n")
                f.write(f"低相似度攻击数 (<0.5): {low_similarity_count}\n")
                
                #鲁棒性评级
                robustness_score = (high_similarity_count * 3 + medium_similarity_count * 2 + low_similarity_count * 1) / len(successful_attacks)
                if robustness_score >= 2.5:
                    robustness_level = "优秀"
                elif robustness_score >= 2.0:
                    robustness_level = "良好"
                elif robustness_score >= 1.5:
                    robustness_level = "一般"
                else:
                    robustness_level = "较差"
                
                f.write(f"鲁棒性评分: {robustness_score:.2f}\n")
                f.write(f"鲁棒性等级: {robustness_level}\n")
            else:
                f.write("没有成功的攻击测试\n")