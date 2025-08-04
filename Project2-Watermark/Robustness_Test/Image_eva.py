import numpy as np
import cv2

class QualityMetrics:
    #图像质量评估指标类
    #提供MSE、PSNR、SSIM等图像质量评估方法

    @staticmethod
    def calculate_mse(original, watermarked):
        #计算均方误差(MSE)
        #original: 原始图像数组
        #watermarked: 含水印图像数组
        #返回值: MSE值（float）
        if original.shape != watermarked.shape:
            raise ValueError("图像尺寸必须一致")
        return np.mean((original - watermarked) ** 2)

    @staticmethod
    def calculate_psnr(original, watermarked):
        #计算峰值信噪比(PSNR)
        #original: 原始图像数组
        #watermarked: 含水印图像数组
        #返回值: PSNR值（dB，float）
        mse = QualityMetrics.calculate_mse(original, watermarked)
        if mse == 0:
            return float('inf')  #相同图像PSNR无穷大
        max_pixel = 255.0
        return 10 * np.log10((max_pixel ** 2) / mse)

    @staticmethod
    def calculate_ssim(original, watermarked):
        #计算结构相似性(SSIM)
        #original: 原始图像数组
        #watermarked: 含水印图像数组
        #返回值: SSIM值（0-1之间，float）
        if original.shape != watermarked.shape:
            raise ValueError("图像尺寸必须一致")
        
        #SSIM计算参数
        C1 = (0.01 * 255) ** 2
        C2 = (0.03 * 255) ** 2

        #使用高斯滤波计算局部统计量
        #高斯核大小11x11，标准差1.5
        mu1 = cv2.GaussianBlur(original, (11, 11), 1.5)
        mu2 = cv2.GaussianBlur(watermarked, (11, 11), 1.5)

        #计算方差和协方差
        sigma1 = cv2.GaussianBlur(original**2, (11, 11), 1.5) - mu1**2
        sigma2 = cv2.GaussianBlur(watermarked**2, (11, 11), 1.5) - mu2**2
        sigma12 = cv2.GaussianBlur(original*watermarked, (11, 11), 1.5) - mu1*mu2

        #计算SSIM映射
        ssim_map = ((2*mu1*mu2 + C1) * (2*sigma12 + C2)) / ((mu1**2 + mu2**2 + C1) * (sigma1 + sigma2 + C2))
        return np.mean(ssim_map)