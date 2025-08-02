@echo off
echo 编译SM4性能测试程序...

REM 设置Visual Studio环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
if errorlevel 1 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" 2>nul
    if errorlevel 1 (
        echo 错误: 找不到Visual Studio环境，请确保已安装Visual Studio
        pause
        exit /b 1
    )
)

REM 编译项目
echo 正在编译项目...
msbuild SM4_Performance_Test.sln /p:Configuration=Release /p:Platform=x64

if errorlevel 1 (
    echo 编译失败！
    pause
    exit /b 1
)

echo 编译成功！

REM 运行测试程序
echo.
echo 运行性能测试...
echo.

x64\Release\SM4_Performance_Test.exe

echo.
echo 测试完成！
pause 