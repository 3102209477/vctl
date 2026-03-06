@echo off
chcp 65001 >nul
echo ====================================
echo   版本控制系统性能测试
echo ====================================
echo.

REM 设置测试目录
set TEST_DIR=%~dp0test_perf
if exist "%TEST_DIR%" rmdir /s /q "%TEST_DIR%"
mkdir "%TEST_DIR%"
cd /d "%TEST_DIR%"

echo [1/5] 初始化仓库...
..\vctl init
echo.

echo [2/5] 创建测试文件（100 个小文件）...
for /l %%i in (1,1,100) do (
    echo Test content %%i > file_%%i.txt
)
echo.

echo [3/5] 首次添加所有文件（冷缓存）...
set STARTTIME=%TIME%
..\vctl add .
set ENDTIME=%TIME%
echo 首次添加完成
echo.

echo [4/5] 提交文件...
..\vctl commit -m "Initial commit"
echo.

echo [5/5] 再次添加（热缓存，无变更）...
set STARTTIME2=%TIME%
..\vctl add .
set ENDTIME2=%TIME%
echo 第二次添加完成（应显示 Unchanged）
echo.

echo ====================================
echo   测试完成！
echo ====================================
echo.
echo 测试结果：
echo - 首次添加：需要计算所有文件的哈希
echo - 二次添加：使用 mtime 快速检测，应几乎瞬间完成
echo.

REM 清理
cd /d "%~dp0"
rmdir /s /q "%TEST_DIR%"

pause
