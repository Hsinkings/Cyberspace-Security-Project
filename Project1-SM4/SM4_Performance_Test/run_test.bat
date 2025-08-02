@echo off
echo SM4加密性能测试
echo ================

REM 检查Python是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo 错误: 找不到Python，请确保已安装Python 3.6或更高版本
    pause
    exit /b 1
)

echo Python版本:
python --version

echo.
echo 选择测试类型:
echo 1. 快速测试 (单次加密)
echo 2. 简单测试 (多次测试取平均值)
echo 3. 完整测试 (所有功能)
echo.
set /p choice="请输入选择 (1-3): "

if "%choice%"=="1" (
    echo 运行快速测试...
    python quick_test.py
) else if "%choice%"=="2" (
    echo 运行简单测试...
    python simple_test.py
) else if "%choice%"=="3" (
    echo 运行完整测试...
    python sm4_performance_test.py
) else (
    echo 无效选择，运行快速测试...
    python quick_test.py
)

echo.
echo 测试完成！
pause 