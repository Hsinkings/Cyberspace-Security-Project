import os
import numpy as np
import cv2
from Watermark.Watermark_API import WatermarkSystem
from Robustness_Test.Robustness_eva import RobustnessTester
from Watermark.Watermark_imageload import ImageUtils

def show_watermark_info():
    #显示图像水印系统信息
    print("=== 图像水印系统演示 ===")
    print("本系统支持以下功能：")
    print("1. 图像水印嵌入 - 将水印图像嵌入到载体图像中")
    print("2. 图像水印提取 - 从含水印图像中提取水印图像")
    print("3. 图像水印检测 - 检测图像中包含哪种水印")
    print("4. 鲁棒性测试 - 测试水印对各种攻击的抵抗能力")
    print()

def check_watermarks():
    #检查可用的水印图像
    ws = WatermarkSystem()
    watermark_paths = ws.get_available_watermarks()
    
    if not watermark_paths:
        print("错误：未找到水印图像文件")
        print("请确保Samples目录中包含以下文件：")
        print("  - Watermark1.png")
        print("  - Watermark2.png") 
        print("  - Watermark3.jpg")
        return None
    
    print(f"找到 {len(watermark_paths)} 个水印图像：")
    for i, path in enumerate(watermark_paths, 1):
        #读取水印图像信息
        img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        if img is not None:
            height, width = img.shape
            print(f"  {i}. {os.path.basename(path)} - 尺寸: {width}x{height}")
        else:
            print(f"  {i}. {os.path.basename(path)} - 读取失败")
    
    return watermark_paths

def create_demo_samples():
    #创建演示用测试图像样本
    #生成五种不同类型的测试图像：自然纹理、复杂纹理、渐变、噪声、规则图案
    sample_dir = "samples"
    os.makedirs(sample_dir, exist_ok=True)
    types = ["natural", "texture", "gradient", "noise", "pattern"]
    for t in types:
        img = ImageUtils.create_test_image(512, 512, t)
        ImageUtils.save_image(img, f"{sample_dir}/{t}.png")
    return [f"{sample_dir}/{t}.png" for t in types]

def demo_image_watermark_embedding():
    #演示图像水印嵌入功能#
    print("=== 图像水印嵌入演示 ===")
    
    #初始化水印系统
    ws = WatermarkSystem(quantization_factor=30)
    
    #获取可用的水印图像
    watermark_paths = ws.get_available_watermarks()
    if not watermark_paths:
        print("错误：未找到水印图像文件")
        print("请确保Samples目录中包含Watermark1.png、Watermark2.png、Watermark3.jpg等文件")
        return None
    
    print(f"找到 {len(watermark_paths)} 个水印图像:")
    for i, path in enumerate(watermark_paths, 1):
        print(f"  {i}. {os.path.basename(path)}")
    
    #创建测试图像
    sample_paths = create_demo_samples()
    
    #为每个测试图像嵌入不同的水印
    watermarked_paths = []
    for i, img_path in enumerate(sample_paths):
        #选择水印（循环使用）
        watermark_path = watermark_paths[i % len(watermark_paths)]
        img_name = os.path.basename(img_path).split('.')[0]
        watermark_name = os.path.basename(watermark_path).split('.')[0]
        
        output_path = f"output/{img_name}_{watermark_name}_watermarked.png"
        
        print(f"\n嵌入水印 {watermark_name} 到 {img_name}...")
        
        try:
            #嵌入水印
            ws.embed_watermark(img_path, watermark_path, output_path)
            
            #计算PSNR评估图像质量
            psnr = ws.calculate_psnr(img_path, output_path)
            print(f"嵌入成功，图像质量 (峰值信噪比PSNR): {psnr:.2f} dB")
            
            watermarked_paths.append(output_path)
            
        except Exception as e:
            print(f"嵌入失败: {e}")
    
    return watermarked_paths

def demo_image_watermark_extraction():
    #演示图像水印提取功能#
    print("\n=== 图像水印提取演示 ===")
    
    #初始化水印系统
    ws = WatermarkSystem()
    
    #检查output目录中的含水印图像
    output_dir = "output"
    if not os.path.exists(output_dir):
        print("未找到含水印图像，请先运行嵌入演示")
        return
    
    watermarked_files = [f for f in os.listdir(output_dir) if f.endswith('_watermarked.png')]
    
    if not watermarked_files:
        print("未找到含水印图像，请先运行嵌入演示")
        return
    
    for watermarked_file in watermarked_files:
        watermarked_path = os.path.join(output_dir, watermarked_file)
        print(f"\n提取水印从: {watermarked_file}")
        
        try:
            #提取水印
            extracted_watermark = ws.extract_watermark(watermarked_path)
            
            #保存提取的水印
            extracted_path = watermarked_path.replace('_watermarked.png', '_extracted.png')
            ImageUtils.save_image(extracted_watermark, extracted_path)
            print(f"提取成功 - 水印保存到: {os.path.basename(extracted_path)}")
            
        except Exception as e:
            print(f"提取失败: {e}")

def demo_image_watermark_detection():
    #演示图像水印检测功能#
    print("\n=== 图像水印检测演示 ===")
    
    #初始化水印系统
    ws = WatermarkSystem()
    
    #获取可用的水印图像
    watermark_paths = ws.get_available_watermarks()
    
    #检查output目录中的含水印图像
    output_dir = "output"
    if not os.path.exists(output_dir):
        print("未找到含水印图像，请先运行嵌入演示")
        return
    
    watermarked_files = [f for f in os.listdir(output_dir) if f.endswith('_watermarked.png')]
    
    if not watermarked_files:
        print("未找到含水印图像，请先运行嵌入演示")
        return
    
    for watermarked_file in watermarked_files:
        watermarked_path = os.path.join(output_dir, watermarked_file)
        print(f"\n检测水印从: {watermarked_file}")
        
        try:
            #检测水印
            best_match, similarity = ws.detect_watermark(watermarked_path, watermark_paths)
            
            if best_match:
                watermark_name = os.path.basename(best_match)
                print(f"检测到水印: {watermark_name}")
                print(f"相似度: {similarity:.4f}")
                
                #判断检测结果
                if similarity > 0.7:
                    print("检测结果: 优秀")
                elif similarity > 0.5:
                    print("检测结果: 良好")
                else:
                    print("检测结果: 较差")
            else:
                print("未检测到任何水印")
                
        except Exception as e:
            print(f"检测失败: {e}")

def demo_robustness_test(watermarked_paths):
    #演示鲁棒性测试功能#
    if not watermarked_paths:
        print("没有含水印图像可用于鲁棒性测试")
        return
    
    print("\n=== 鲁棒性测试演示 ===")
    print("注意：鲁棒性测试可能需要较长时间...")
    
    ws = WatermarkSystem()
    tester = RobustnessTester(ws.core)
    
    #获取可用的水印图像
    watermark_paths = ws.get_available_watermarks()
    
    #对每个含水印图像进行鲁棒性测试
    for watermarked_path in watermarked_paths:
        print(f"\n测试图像: {os.path.basename(watermarked_path)}")
        
        try:
            #执行多种攻击测试
            results = tester.run_multiple_attacks(watermarked_path, watermark_paths)
            
            #生成详细的鲁棒性测试报告
            report_path = watermarked_path.replace('_watermarked.png', '_robustness_report.txt')
            tester.generate_report(results, report_path)
            print(f"鲁棒性测试完成 - 报告保存到: {os.path.basename(report_path)}")
            
            #显示简要结果
            successful_attacks = [r for r in results if r.get('success', False)]
            if successful_attacks:
                avg_similarity = np.mean([r.get('similarity', 0) for r in successful_attacks])
                print(f"平均相似度: {avg_similarity:.4f}")
            
        except Exception as e:
            print(f"鲁棒性测试失败: {e}")

def main():
    #主函数#
    show_watermark_info()
    
    #检查水印图像
    watermark_paths = check_watermarks()
    if not watermark_paths:
        return
    
    #确保输出目录存在
    os.makedirs("output", exist_ok=True)
    
    #1. 图像水印嵌入演示
    watermarked_paths = demo_image_watermark_embedding()
    
    #2. 图像水印提取演示
    demo_image_watermark_extraction()
    
    #3. 图像水印检测演示
    demo_image_watermark_detection()
    
    #4. 鲁棒性测试演示
    demo_robustness_test(watermarked_paths)
    
    print("\n图像水印系统演示完成！")
    print("结果保存在 output 目录中")
    print("\n生成的文件包括：")
    print("  - *_watermarked.png: 含水印的图像")
    print("  - *_extracted.png: 提取的水印图像")
    print("  - *_robustness_report.txt: 鲁棒性测试报告")

if __name__ == "__main__":
    main()