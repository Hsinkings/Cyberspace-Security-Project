import numpy as np
from difflib import SequenceMatcher

class WatermarkEvaluator:
    #水印提取效果评估工具类
    #提供水印提取准确性和文本相似度评估方法

    @staticmethod
    def calculate_extraction_accuracy(original_text, extracted_text):
        #计算提取准确率（基于字符匹配）
        #original_text: 原始水印文本
        #extracted_text: 提取的水印文本
        #返回值: 准确率（0-1之间的float）
        if not original_text:
            return 0.0
        #使用序列匹配算法计算最长公共子序列占原文本的比例
        matcher = SequenceMatcher(None, original_text, extracted_text)
        match_ratio = matcher.ratio()
        return match_ratio

    @staticmethod
    def text_similarity(text1, text2):
        #计算文本相似度（基于编辑距离）
        #text1: 第一个文本字符串
        #text2: 第二个文本字符串
        #返回值: 相似度（0-1之间的float）
        return SequenceMatcher(None, text1, text2).ratio()