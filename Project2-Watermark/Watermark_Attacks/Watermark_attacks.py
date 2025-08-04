import cv2
import numpy as np
from PIL import Image, ImageEnhance
import os

class ImageAttacker:
    #图像攻击工具类
    #实现多种图像攻击方式，用于测试水印的鲁棒性

    @staticmethod
    def horizontal_flip(image):
        #水平翻转攻击
        #image: 输入图像数组
        #返回值: 水平翻转后的图像数组
        return cv2.flip(image, 1)

    @staticmethod
    def vertical_flip(image):
        #垂直翻转攻击
        #image: 输入图像数组
        #返回值: 垂直翻转后的图像数组
        return cv2.flip(image, 0)

    @staticmethod
    def rotate(image, angle=15):
        #旋转攻击
        #image: 输入图像数组
        #angle: 旋转角度（度），默认15度
        #返回值: 旋转后的图像数组
        h, w = image.shape[:2]
        center = (w // 2, h // 2)
        mat = cv2.getRotationMatrix2D(center, angle, 1.0)
        return cv2.warpAffine(image, mat, (w, h), borderMode=cv2.BORDER_REFLECT)

    @staticmethod
    def translate(image, dx=20, dy=10):
        #平移攻击
        #image: 输入图像数组
        #dx: X方向平移像素数，默认20
        #dy: Y方向平移像素数，默认10
        #返回值: 平移后的图像数组
        h, w = image.shape[:2]
        mat = np.float32([[1, 0, dx], [0, 1, dy]])
        return cv2.warpAffine(image, mat, (w, h), borderMode=cv2.BORDER_REFLECT)

    @staticmethod
    def crop(image, ratio=0.8):
        #居中裁剪攻击
        #image: 输入图像数组
        #ratio: 裁剪比例，默认0.8（保留80%）
        #返回值: 裁剪后重新缩放的图像数组
        h, w = image.shape[:2]
        new_h, new_w = int(h * ratio), int(w * ratio)
        start_y, start_x = (h - new_h) // 2, (w - new_w) // 2
        cropped = image[start_y:start_y+new_h, start_x:start_x+new_w]
        return cv2.resize(cropped, (w, h))

    @staticmethod
    def random_crop(image, ratio=0.7):
        #随机裁剪攻击
        #image: 输入图像数组
        #ratio: 裁剪比例，默认0.7（保留70%）
        #返回值: 随机裁剪后重新缩放的图像数组
        h, w = image.shape[:2]
        new_h, new_w = int(h * ratio), int(w * ratio)
        start_y = np.random.randint(0, h - new_h + 1)
        start_x = np.random.randint(0, w - new_w + 1)
        cropped = image[start_y:start_y+new_h, start_x:start_x+new_w]
        return cv2.resize(cropped, (w, h))

    @staticmethod
    def gaussian_blur(image, kernel=5, sigma=1.5):
        #高斯模糊攻击
        #image: 输入图像数组
        #kernel: 高斯核大小，默认5x5
        #sigma: 高斯标准差，默认1.5
        #返回值: 模糊后的图像数组
        return cv2.GaussianBlur(image, (kernel, kernel), sigma)

    @staticmethod
    def gaussian_noise(image, mean=0, std=15):
        #高斯噪声攻击
        #image: 输入图像数组
        #mean: 噪声均值，默认0
        #std: 噪声标准差，默认15
        #返回值: 添加噪声后的图像数组
        noise = np.random.normal(mean, std, image.shape).astype(np.int16)
        noisy = np.clip(image.astype(np.int16) + noise, 0, 255).astype(np.uint8)
        return noisy

    @staticmethod
    def adjust_contrast(image, factor=1.5):
        #对比度调整攻击
        #image: 输入图像数组
        #factor: 对比度调整因子，默认1.5
        #返回值: 调整对比度后的图像数组
        pil_img = Image.fromarray(image)
        enhancer = ImageEnhance.Contrast(pil_img)
        return np.array(enhancer.enhance(factor))

    @staticmethod
    def adjust_brightness(image, factor=1.3):
        #亮度调整攻击
        #image: 输入图像数组
        #factor: 亮度调整因子，默认1.3
        #返回值: 调整亮度后的图像数组
        pil_img = Image.fromarray(image)
        enhancer = ImageEnhance.Brightness(pil_img)
        return np.array(enhancer.enhance(factor))

    @staticmethod
    def jpeg_compress(image, quality=50):
        #JPEG压缩攻击
        #image: 输入图像数组
        #quality: JPEG质量参数（1-100），默认50
        #返回值: 压缩后的图像数组
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), quality]
        result, encimg = cv2.imencode('.jpg', image, encode_param)
        return cv2.imdecode(encimg, 0)  #0表示灰度图解码