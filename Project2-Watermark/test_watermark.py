#水印算法修复测试脚本
#验证修复后的水印嵌入、提取和检测功能

import os
import cv2
import numpy as np
from Watermark.Watermark_API import WatermarkSystem
from Watermark.Watermark_imageload import ImageUtils

def test_watermark_embedding_and_extraction():
    #测试水印嵌入和提取的一致性#
    print("=== 水印嵌入和提取一致性测试 ===")
    
    #初始化水印系统
    ws = WatermarkSystem(quantization_factor=30)
    
    #获取水印图像
    watermark_paths = ws.get_available_watermarks()
    if not watermark_paths:
        print("未找到水印图像")
        return
    
    #创建测试载体图像
    carrier_img = ImageUtils.create_test_image(512, 512, "natural")
    carrier_path = "test_carrier.png"
    ImageUtils.save_image(carrier_img, carrier_path)
    
    print(f"使用水印: {os.path.basename(watermark_paths[0])}")
    
    #嵌入水印
    watermarked_path = "test_watermarked.png"
    try:
        ws.embed_watermark(carrier_path, watermark_paths[0], watermarked_path)
        print("水印嵌入成功")
        
        #计算PSNR
        psnr = ws.calculate_psnr(carrier_path, watermarked_path)
        print(f"PSNR: {psnr:.2f} dB")
        
    except Exception as e:
        print(f"水印嵌入失败: {e}")
        return
    
    #提取水印
    try:
        extracted_watermark = ws.extract_watermark(watermarked_path)
        extracted_path = "test_extracted.png"
        ImageUtils.save_image(extracted_watermark, extracted_path)
        print("水印提取成功")
        
    except Exception as e:
        print(f"水印提取失败: {e}")
        return
    
    #计算相似度
    try:
        similarity = ws.calculate_watermark_similarity(watermark_paths[0], extracted_watermark)
        print(f"相似度: {similarity:.4f}")
        
        if similarity > 0.7:
            print("相似度优秀 - 算法工作正常")
        elif similarity > 0.5:
            print("相似度良好 - 算法基本正常")
        else:
            print("相似度较差 - 算法可能存在问题")
            
    except Exception as e:
        print(f"相似度计算失败: {e}")

def test_watermark_detection():
    #测试水印检测功能#
    print("\n=== 水印检测功能测试 ===")
    
    #初始化水印系统
    ws = WatermarkSystem(quantization_factor=30)
    
    #获取水印图像
    watermark_paths = ws.get_available_watermarks()
    if not watermark_paths:
        print("未找到水印图像")
        return
    
    #检查是否有含水印的图像
    watermarked_files = [f for f in os.listdir("output") if f.endswith('_watermarked.png')]
    
    if not watermarked_files:
        print("未找到含水印图像，请先运行主程序")
        return
    
    #测试检测功能
    for watermarked_file in watermarked_files[:3]:  #只测试前3个
        watermarked_path = os.path.join("output", watermarked_file)
        print(f"\n检测图像: {watermarked_file}")
        
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
                print("未检测到水印")
                
        except Exception as e:
            print(f"检测失败: {e}")

def test_similarity_calculation():
    #测试相似度计算功能#
    print("\n=== 相似度计算测试 ===")
    
    #初始化水印系统
    ws = WatermarkSystem()
    
    #获取水印图像
    watermark_paths = ws.get_available_watermarks()
    if not watermark_paths:
        print("未找到水印图像")
        return
    
    #测试相同图像的相似度
    original_watermark = ws.core.preprocess_watermark(watermark_paths[0])
    similarity = ws.core.calculate_watermark_similarity(original_watermark, original_watermark)
    print(f"相同图像相似度: {similarity:.4f} (应该接近1.0)")
    
    #测试不同图像的相似度
    if len(watermark_paths) > 1:
        different_watermark = ws.core.preprocess_watermark(watermark_paths[1])
        similarity = ws.core.calculate_watermark_similarity(original_watermark, different_watermark)
        print(f"不同图像相似度: {similarity:.4f} (应该较低)")
    
    #测试随机噪声的相似度
    random_noise = np.random.randint(0, 256, original_watermark.shape, dtype=np.uint8)
    similarity = ws.core.calculate_watermark_similarity(original_watermark, random_noise)
    print(f"随机噪声相似度: {similarity:.4f} (应该很低)")

def main():
    #主函数#
    print("水印算法修复测试")
    print("=" * 50)
    
    #1. 测试嵌入和提取一致性
    test_watermark_embedding_and_extraction()
    
    #2. 测试水印检测功能
    test_watermark_detection()
    
    #3. 测试相似度计算
    test_similarity_calculation()
    
    print("\n测试完成！")
    
    #清理测试文件
    test_files = ["test_carrier.png", "test_watermarked.png", "test_extracted.png"]
    for file in test_files:
        if os.path.exists(file):
            os.remove(file)

if __name__ == "__main__":
    main() 