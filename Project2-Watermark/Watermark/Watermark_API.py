from Watermark.Watermark_main import WatermarkCore
from Watermark.Watermark_imageload import ImageUtils
from Robustness_Test.Image_eva import QualityMetrics
import os

class WatermarkSystem:
    #图像水印系统入口类
    #提供图像水印嵌入、提取、检测和质量评估的统一接口
    def __init__(self, block_size=8, quantization_factor=30):
        #初始化水印系统
        #block_size: DCT分块大小，默认为8x8
        #quantization_factor: 量化因子，控制水印强度，默认30
        self.core = WatermarkCore(block_size, quantization_factor)
        self.utils = ImageUtils
        self.metrics = QualityMetrics()

    def embed_watermark(self, image_path, watermark_path, output_path=None, strength=None):
        #嵌入图像水印到载体图像中
        #image_path: 载体图像路径
        #watermark_path: 水印图像路径
        #output_path: 输出图像路径，可选
        #strength: 水印强度，可选，用于动态调整量化因子
        #返回值: 含水印的图像数组
        
        #动态调整量化因子（如果指定强度参数）
        if strength is not None:
            self.core.q_factor = strength
        
        #执行水印嵌入
        watermarked_img = self.core.embed_watermark(image_path, watermark_path)
        
        #保存含水印图像（如果指定输出路径）
        if output_path:
            self.utils.save_image(watermarked_img, output_path)
        return watermarked_img

    def extract_watermark(self, watermarked_path, watermark_size=(64, 64)):
        #从含水印图像中提取水印图像
        #watermarked_path: 含水印图像路径
        #watermark_size: 水印图像大小，默认(64, 64)
        #返回值: 提取的水印图像数组
        return self.core.extract_watermark(watermarked_path, watermark_size)

    def detect_watermark(self, watermarked_path, watermark_paths):
        #检测图像中包含哪种水印
        #watermarked_path: 含水印图像路径
        #watermark_paths: 候选水印图像路径列表
        #返回值: (最佳匹配的水印路径, 相似度分数)
        return self.core.detect_watermark(watermarked_path, watermark_paths)

    def calculate_psnr(self, original_path, watermarked_path):
        #计算原始图像与含水印图像的PSNR（峰值信噪比）
        #original_path: 原始图像路径
        #watermarked_path: 含水印图像路径
        #返回值: PSNR值（dB）
        original = self.utils.load_image(original_path)
        watermarked = self.utils.load_image(watermarked_path)
        return self.metrics.calculate_psnr(original, watermarked)

    def calculate_watermark_similarity(self, original_watermark_path, extracted_watermark):
        #计算原始水印与提取水印的相似度
        #original_watermark_path: 原始水印图像路径
        #extracted_watermark: 提取的水印图像数组
        #返回值: 相似度分数（0-1）
        original_watermark = self.core.preprocess_watermark(original_watermark_path)
        return self.core.calculate_watermark_similarity(original_watermark, extracted_watermark)

    def get_available_watermarks(self, samples_dir="Samples"):
        #获取可用的水印图像列表
        #samples_dir: 水印图像目录
        #返回值: 水印图像路径列表
        watermark_paths = []
        if os.path.exists(samples_dir):
            for file in os.listdir(samples_dir):
                if file.lower().startswith('watermark') and file.lower().endswith(('.png', '.jpg', '.jpeg')):
                    watermark_paths.append(os.path.join(samples_dir, file))
        return sorted(watermark_paths)