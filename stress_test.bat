@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ====================================
echo   版本控制系统 - Python 功能压力测试
echo ====================================
echo.

set REPO_ROOT=%~dp0
set PYTHON=%REPO_ROOT%.venv\Scripts\python.exe
set SCRIPT=%REPO_ROOT%stress_test.py

if not exist "%PYTHON%" (
    echo Error: Python interpreter not found at "%PYTHON%".
    echo Please create the virtual environment or adjust the path.
    exit /b 1
)

if not exist "%SCRIPT%" (
    echo Error: Python script not found at "%SCRIPT%".
    exit /b 1
)

echo Running Python stress test...
"%PYTHON%" "%SCRIPT%"
exit /b %errorlevel%
