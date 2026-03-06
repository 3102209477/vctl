@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ====================================
echo   版本控制系统 - 功能压力测试
echo ====================================
echo.

REM 设置测试目录
set TEST_DIR=%~dp0stress_test
if exist "%TEST_DIR%" rmdir /s /q "%TEST_DIR%"
mkdir "%TEST_DIR%"
cd /d "%TEST_DIR%"

set VCTL=..\versionctl\vctl.exe
set PASS=0
set FAIL=0

echo [测试 1/10] 初始化仓库测试...
%VCTL% init >nul 2>&1
if errorlevel 1 (
    echo [FAIL] 初始化失败
    set /a FAIL+=1
) else (
    echo [PASS] 初始化成功
    set /a PASS+=1
)
echo.

echo [测试 2/10] 批量创建文件（100 个）...
for /l %%i in (1,1,100) do (
    echo Test content line 1 > file_%%i.txt
    echo Test content line 2 >> file_%%i.txt
    echo Test content line 3 >> file_%%i.txt
)
echo 已创建 100 个测试文件
echo.

echo [测试 3/10] 批量添加文件（冷缓存）...
set STARTTIME=%TIME%
%VCTL% add . > add_output.txt 2>&1
set ENDTIME=%TIME%
findstr /C:"Added:" add_output.txt >nul
if errorlevel 1 (
    echo [FAIL] 添加文件失败
    type add_output.txt
    set /a FAIL+=1
) else (
    echo [PASS] 成功添加 100 个文件
    findstr /C:"Added:" add_output.txt
    set /a PASS+=1
)
echo 耗时：%STARTTIME% -> %ENDTIME%
echo.

echo [测试 4/10] 首次提交...
%VCTL% commit -m "Initial commit with 100 files" >nul 2>&1
if errorlevel 1 (
    echo [FAIL] 提交失败
    set /a FAIL+=1
) else (
    echo [PASS] 提交成功
    set /a PASS+=1
)
echo.

echo [测试 5/10] 状态检查（无变更）...
%VCTL% status > status1.txt 2>&1
findstr /C:"Nothing to commit" status1.txt >nul
if errorlevel 1 (
    echo [WARN] 状态检查有警告
    type status1.txt
) else (
    echo [PASS] 状态检查正常
    set /a PASS+=1
)
echo.

echo [测试 6/10] 批量修改文件（50 个）...
for /l %%i in (1,1,50) do (
    echo Modified line >> file_%%i.txt
)
echo 已修改 50 个文件
echo.

echo [测试 7/10] 再次添加（热缓存，部分变更）...
set STARTTIME=%TIME%
%VCTL% add . > add_output2.txt 2>&1
set ENDTIME=%TIME%
findstr /C:"Added:" add_output2.txt >nul
if errorlevel 1 (
    echo [FAIL] 添加失败
    type add_output2.txt
    set /a FAIL+=1
) else (
    echo [PASS] 成功添加变更
    findstr /C:"Added:" add_output2.txt
    findstr /C:"Unchanged:" add_output2.txt
    set /a PASS+=1
)
echo 耗时：%STARTTIME% -> %ENDTIME%
echo.

echo [测试 8/10] 第二次提交...
%VCTL% commit -m "Update 50 files" >nul 2>&1
if errorlevel 1 (
    echo [FAIL] 提交失败
    set /a FAIL+=1
) else (
    echo [PASS] 提交成功
    set /a PASS+=1
)
echo.

echo [测试 9/10] 查看提交历史...
%VCTL% log -n 5 > log_output.txt 2>&1
findstr /C:"commit" log_output.txt >nul
if errorlevel 1 (
    echo [FAIL] 查看历史失败
    type log_output.txt
    set /a FAIL+=1
) else (
    echo [PASS] 历史记录正常
    find /C "commit" log_output.txt
    set /a PASS+=1
)
echo.

echo [测试 10/10] 创建和切换分支...
%VCTL% branch -b feature-test >nul 2>&1
if errorlevel 1 (
    echo [FAIL] 创建分支失败
    set /a FAIL+=1
) else (
    echo [PASS] 创建分支成功
    set /a PASS+=1
)

%VCTL% checkout feature-test >nul 2>&1
if errorlevel 1 (
    echo [FAIL] 切换分支失败
    set /a FAIL+=1
) else (
    echo [PASS] 切换分支成功
    set /a PASS+=1
)

REM 在分支上工作
echo Branch work > branch_file.txt
%VCTL% add . >nul 2>&1
%VCTL% commit -m "Branch commit" >nul 2>&1

%VCTL% checkout master >nul 2>&1
if errorlevel 1 (
    echo [WARN] 切回 master 失败
) else (
    echo [PASS] 切回 master 成功
    set /a PASS+=1
)
echo.

echo ====================================
echo   测试结果汇总
echo ====================================
echo 通过：%PASS%
echo 失败：%FAIL%
echo 总计：%PASS%+%FAIL%
echo.

if %FAIL%==0 (
    echo [SUCCESS] 所有测试通过！
) else (
    echo [WARNING] 部分测试失败
)

echo.
echo 清理测试目录...
cd /d "%~dp0"
rmdir /s /q "%TEST_DIR%"
echo 清理完成

pause
