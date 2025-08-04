import cv2
import numpy as np
import os

class WatermarkCore:
    #图像水印核心算法类
    #实现基于DCT变换的频域图像水印嵌入和提取算法
    def __init__(self, block_size=8, quantization_factor=30):
        #初始化水印核心参数
        #block_size: DCT分块大小，默认为8x8像素
        #quantization_factor: 量化因子，控制水印强度，默认30
        self.block_size = block_size
        self.q_factor = quantization_factor

    def preprocess_watermark(self, watermark_path):
        #预处理水印图像#
        #加载水印图像
        watermark = cv2.imread(watermark_path, cv2.IMREAD_GRAYSCALE)
        if watermark is None:
            raise FileNotFoundError(f"无法读取水印图像: {watermark_path}")
        
        #调整水印图像大小（建议为64x64或更小）
        target_size = 64
        watermark_resized = cv2.resize(watermark, (target_size, target_size))
        
        return watermark_resized

    def embed_watermark(self, image_path, watermark_path):
        #嵌入图像水印到载体图像中
        #image_path: 载体图像路径
        #watermark_path: 水印图像路径
        #返回值: 含水印的图像数组
        
        #加载载体图像
        carrier_image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
        if carrier_image is None:
            raise FileNotFoundError(f"无法读取载体图像: {image_path}")
        
        #预处理水印图像
        watermark = self.preprocess_watermark(watermark_path)
        watermark_height, watermark_width = watermark.shape
        
        #调整载体图像尺寸为block_size的倍数
        carrier_height, carrier_width = carrier_image.shape
        new_height = ((carrier_height + self.block_size - 1) // self.block_size) * self.block_size
        new_width = ((carrier_width + self.block_size - 1) // self.block_size) * self.block_size
        carrier_resized = cv2.resize(carrier_image, (new_width, new_height))
        
        #检查载体图像是否足够大
        if new_height < watermark_height * self.block_size or new_width < watermark_width * self.block_size:
            raise ValueError("载体图像太小，无法嵌入水印")
        
        #转换为float32以进行DCT变换
        carrier_float = np.float32(carrier_resized)
        
        #嵌入水印
        for i in range(watermark_height):
            for j in range(watermark_width):
                #计算载体图像中的块位置
                block_i = i * self.block_size
                block_j = j * self.block_size
                
                #提取8x8块并进行DCT变换
                block = carrier_float[block_i:block_i+self.block_size, block_j:block_j+self.block_size]
                dct_block = cv2.dct(block)
                
                #获取水印像素值（0-255）
                watermark_pixel = watermark[i, j]
                
                #将水印像素值嵌入到DCT系数中
                #使用多个中频系数位置提高鲁棒性
                coef_positions = [(2, 3), (3, 2), (3, 3), (2, 2)]
                
                for pos_idx, (row, col) in enumerate(coef_positions):
                    coef = dct_block[row, col]
                    
                    #量化系数
                    quantized = round(coef / self.q_factor)
                    
                    #根据水印像素值调整系数
                    #将水印像素值分解为多个比特
                    bit_offset = pos_idx * 2
                    watermark_bits = (watermark_pixel >> bit_offset) & 0x03  #取2位
                    
                    #确保量化值的低2位与水印比特匹配
                    quantized = (quantized & 0xFC) | watermark_bits  #保留高6位，替换低2位
                    
                    #反量化回DCT系数
                    dct_block[row, col] = quantized * self.q_factor
                
                #逆DCT变换
                idct_block = cv2.idct(dct_block)
                
                #将修改后的块放回载体图像
                carrier_float[block_i:block_i+self.block_size, block_j:block_j+self.block_size] = idct_block
        
        #转换回uint8格式
        watermarked_image = np.clip(carrier_float, 0, 255).astype(np.uint8)
        
        return watermarked_image

    def extract_watermark(self, watermarked_image_path, watermark_size=(64, 64)):
        #从含水印图像中提取水印图像
        #watermarked_image_path: 含水印图像路径
        #watermark_size: 水印图像大小，默认(64, 64)
        #返回值: 提取的水印图像数组
        
        #加载含水印图像
        watermarked_image = cv2.imread(watermarked_image_path, cv2.IMREAD_GRAYSCALE)
        if watermarked_image is None:
            raise FileNotFoundError(f"无法读取含水印图像: {watermarked_image_path}")
        
        #调整图像尺寸为block_size的倍数
        height, width = watermarked_image.shape
        new_height = ((height + self.block_size - 1) // self.block_size) * self.block_size
        new_width = ((width + self.block_size - 1) // self.block_size) * self.block_size
        watermarked_resized = cv2.resize(watermarked_image, (new_width, new_height))
        
        #转换为float32
        watermarked_float = np.float32(watermarked_resized)
        
        #提取水印
        watermark_height, watermark_width = watermark_size
        extracted_watermark = np.zeros(watermark_size, dtype=np.uint8)
        
        for i in range(watermark_height):
            for j in range(watermark_width):
                #计算载体图像中的块位置
                block_i = i * self.block_size
                block_j = j * self.block_size
                
                #检查块是否在图像范围内
                if (block_i + self.block_size > new_height or 
                    block_j + self.block_size > new_width):
                    continue
                
                #提取8x8块并进行DCT变换
                block = watermarked_float[block_i:block_i+self.block_size, block_j:block_j+self.block_size]
                dct_block = cv2.dct(block)
                
                #从DCT系数中提取水印像素值
                coef_positions = [(2, 3), (3, 2), (3, 3), (2, 2)]
                watermark_pixel = 0
                
                for pos_idx, (row, col) in enumerate(coef_positions):
                    coef = dct_block[row, col]
                    
                    #量化系数
                    quantized = round(coef / self.q_factor)
                    
                    #提取低2位作为水印比特
                    bit_offset = pos_idx * 2
                    watermark_bits = quantized & 0x03
                    watermark_pixel |= (watermark_bits << bit_offset)
                
                #限制像素值范围
                watermark_pixel = min(255, max(0, watermark_pixel))
                extracted_watermark[i, j] = watermark_pixel
        
        return extracted_watermark

    def calculate_watermark_similarity(self, original_watermark, extracted_watermark):
        #计算原始水印与提取水印的相似度#
        #确保两个水印大小一致
        if original_watermark.shape != extracted_watermark.shape:
            #调整提取的水印大小以匹配原始水印
            extracted_watermark = cv2.resize(extracted_watermark, 
                                           (original_watermark.shape[1], original_watermark.shape[0]))
        
        #使用归一化互相关(NCC)计算相似度
        original_norm = original_watermark.astype(np.float32) / 255.0
        extracted_norm = extracted_watermark.astype(np.float32) / 255.0
        
        #计算均值
        original_mean = np.mean(original_norm)
        extracted_mean = np.mean(extracted_norm)
        
        #计算归一化互相关
        numerator = np.sum((original_norm - original_mean) * (extracted_norm - extracted_mean))
        denominator = np.sqrt(np.sum((original_norm - original_mean)**2) * np.sum((extracted_norm - extracted_mean)**2))
        
        if denominator == 0:
            return 0.0
        
        ncc = numerator / denominator
        
        #对于水印检测，我们只关心正相关，负相关表示完全不匹配
        return max(0.0, ncc)  #将负值截断为0

    def detect_watermark(self, watermarked_image_path, watermark_paths):
        #检测图像中包含哪种水印#
        #watermarked_image_path: 含水印图像路径
        #watermark_paths: 候选水印图像路径列表
        #返回值: (最佳匹配的水印路径, 相似度分数)
        
        best_match = None
        best_similarity = 0.0  #改为0.0（只考虑正相关）
        
        for watermark_path in watermark_paths:
            try:
                #预处理原始水印
                original_watermark = self.preprocess_watermark(watermark_path)
                
                #提取水印
                extracted_watermark = self.extract_watermark(watermarked_image_path, original_watermark.shape)
                
                #计算相似度
                similarity = self.calculate_watermark_similarity(original_watermark, extracted_watermark)
                
                print(f"水印 {os.path.basename(watermark_path)} 相似度: {similarity:.4f}")
                
                if similarity > best_similarity:
                    best_similarity = similarity
                    best_match = watermark_path
                    
            except Exception as e:
                print(f"处理水印 {watermark_path} 时出错: {e}")
                continue
        
        return best_match, best_similarity