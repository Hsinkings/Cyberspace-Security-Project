import cv2
import numpy as np
import os

class ImageUtils:
    #图像处理工具类
    #提供图像加载、保存和测试图像生成功能

    @staticmethod
    def load_image(path, grayscale=True):
        #加载图像文件
        #path: 图像文件路径
        #grayscale: 是否转换为灰度图，默认True
        #返回值: 图像数组（numpy.ndarray）
        flag = cv2.IMREAD_GRAYSCALE if grayscale else cv2.IMREAD_COLOR
        img = cv2.imread(path, flag)
        if img is None:
            raise FileNotFoundError(f"图像加载失败: {path}")
        return img

    @staticmethod
    def save_image(image, path):
        #保存图像到文件
        #image: 图像数组
        #path: 保存路径
        dir_name = os.path.dirname(path)
        if dir_name and not os.path.exists(dir_name):
            os.makedirs(dir_name, exist_ok=True)
        cv2.imwrite(path, image)

    @staticmethod
    def create_test_image(width=512, height=512, type="natural"):
        #创建测试图像
        #width: 图像宽度，默认512
        #height: 图像高度，默认512
        #type: 图像类型，可选"natural"、"texture"、"gradient"、"noise"、"pattern"
        #返回值: 生成的测试图像数组
        
        if type == "natural":
            #生成自然纹理图像（模拟自然场景）
            #包含不同亮度的区域和自然过渡
            img = np.zeros((height, width), dtype=np.uint8)
            
            #创建多个亮度区域
            for i in range(4):
                for j in range(4):
                    #每个区域的基础亮度
                    base_brightness = 50 + (i + j) * 20
                    #区域坐标
                    x1, x2 = i * width // 4, (i + 1) * width // 4
                    y1, y2 = j * height // 4, (j + 1) * height // 4
                    
                    #添加渐变和噪声
                    for y in range(y1, y2):
                        for x in range(x1, x2):
                            #渐变效果
                            grad_x = (x - x1) / (x2 - x1) if x2 > x1 else 0
                            grad_y = (y - y1) / (y2 - y1) if y2 > y1 else 0
                            gradient = (grad_x + grad_y) * 30
                            
                            #添加噪声
                            noise = np.random.randint(-10, 11)
                            
                            #最终像素值
                            pixel_value = int(base_brightness + gradient + noise)
                            pixel_value = max(0, min(255, pixel_value))
                            img[y, x] = pixel_value
            
            return img
            
        elif type == "texture":
            #生成复杂纹理图像
            #模拟真实图像的纹理特征
            img = np.zeros((height, width), dtype=np.uint8)
            
            #基础纹理
            for y in range(height):
                for x in range(width):
                    #正弦波纹理
                    sine1 = 50 * np.sin(x * 0.05) * np.cos(y * 0.03)
                    sine2 = 30 * np.sin(x * 0.02 + y * 0.04)
                    
                    #噪声纹理
                    noise = np.random.randint(-15, 16)
                    
                    #边缘纹理
                    edge_effect = 20 * np.exp(-((x - width//2)**2 + (y - height//2)**2) / (width * height / 8))
                    
                    #组合所有效果
                    pixel_value = int(128 + sine1 + sine2 + noise + edge_effect)
                    pixel_value = max(0, min(255, pixel_value))
                    img[y, x] = pixel_value
            
            return img
            
        elif type == "gradient":
            #生成渐变图像
            #测试水印在不同亮度变化下的表现
            img = np.zeros((height, width), dtype=np.uint8)
            
            #径向渐变
            center_x, center_y = width // 2, height // 2
            max_dist = np.sqrt(center_x**2 + center_y**2)
            
            for y in range(height):
                for x in range(width):
                    #计算到中心的距离
                    dist = np.sqrt((x - center_x)**2 + (y - center_y)**2)
                    
                    #径向渐变
                    radial = int(255 * (1 - dist / max_dist))
                    
                    #添加线性渐变
                    linear_x = int(255 * x / width)
                    linear_y = int(255 * y / height)
                    
                    #组合渐变
                    pixel_value = int((radial + linear_x + linear_y) / 3)
                    pixel_value = max(0, min(255, pixel_value))
                    img[y, x] = pixel_value
            
            return img
            
        elif type == "noise":
            #生成噪声图像
            #测试水印在高噪声环境下的鲁棒性
            img = np.zeros((height, width), dtype=np.uint8)
            
            #基础噪声
            base_noise = np.random.randint(0, 256, (height, width), dtype=np.uint8)
            
            #添加不同频率的噪声
            for y in range(height):
                for x in range(width):
                    #高频噪声
                    high_freq = 20 * np.sin(x * 0.3) * np.cos(y * 0.2)
                    
                    #中频噪声
                    mid_freq = 15 * np.sin(x * 0.1) * np.cos(y * 0.08)
                    
                    #低频噪声
                    low_freq = 10 * np.sin(x * 0.02) * np.cos(y * 0.015)
                    
                    #组合所有噪声
                    pixel_value = int(base_noise[y, x] + high_freq + mid_freq + low_freq)
                    pixel_value = max(0, min(255, pixel_value))
                    img[y, x] = pixel_value
            
            return img
            
        elif type == "pattern":
            #生成规则图案图像
            #测试水印在规则结构下的表现
            img = np.zeros((height, width), dtype=np.uint8)
            
            #棋盘格图案
            block_size = 32
            for y in range(height):
                for x in range(width):
                    block_x = x // block_size
                    block_y = y // block_size
                    
                    if (block_x + block_y) % 2 == 0:
                        img[y, x] = 200
                    else:
                        img[y, x] = 50
            
            #添加圆形图案
            for i in range(3):
                for j in range(3):
                    cx = width // 6 + i * width // 3
                    cy = height // 6 + j * height // 3
                    radius = min(width, height) // 12
                    
                    #绘制圆形
                    for y in range(max(0, cy-radius), min(height, cy+radius)):
                        for x in range(max(0, cx-radius), min(width, cx+radius)):
                            if (x - cx)**2 + (y - cy)**2 <= radius**2:
                                img[y, x] = 100
            
            return img
            
        else:
            #默认：生成混合特征图像
            #结合多种图像特征
            img = np.zeros((height, width), dtype=np.uint8)
            
            #基础渐变
            for y in range(height):
                for x in range(width):
                    #线性渐变
                    linear = int(255 * (x + y) / (width + height))
                    
                    #添加纹理
                    texture = 30 * np.sin(x * 0.1) * np.cos(y * 0.08)
                    
                    #添加噪声
                    noise = np.random.randint(-10, 11)
                    
                    #组合
                    pixel_value = int(linear + texture + noise)
                    pixel_value = max(0, min(255, pixel_value))
                    img[y, x] = pixel_value
            
            return img