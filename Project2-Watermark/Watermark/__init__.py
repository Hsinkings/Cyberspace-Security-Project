#Watermark包初始化文件
#导出水印系统的核心类和工具
from .Watermark_API import WatermarkSystem
from .Watermark_main import WatermarkCore
from .Watermark_imageload import ImageUtils

__all__ = ['WatermarkSystem', 'WatermarkCore', 'ImageUtils'] 