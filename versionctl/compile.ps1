# Version Control System 编译脚本
# 使用 g++ 编译模块化版本控制系统

Write-Host "Building Version Control System..." -ForegroundColor Cyan

# 设置输出文件名
$OUTPUT = "vctl.exe"

# 获取所有源文件
$SOURCES = @(
    "main.cpp",
    "utils\path_utils.cpp",
    "utils\platform.cpp",
    "storage\hash.cpp",
    "storage\binary_io.cpp",
    "storage\cache.cpp",
    "core\objects.cpp",
    "core\refs.cpp",
    "config\config_manager.cpp",
    "config\ignore_rules.cpp",
    "commands\cmd_init.cpp",
    "commands\cmd_add.cpp",
    "commands\cmd_commit.cpp",
    "commands\cmd_status.cpp",
    "commands\cmd_log.cpp",
    "commands\cmd_branch.cpp"
)

# 包含目录
$INCLUDES = @("-Iinclude")

# 编译选项
$CXXFLAGS = @(
    "-std=c++17",
    "-Wall",
    "-Wextra",
    "-O2",
    "-D_WIN32_WINNT=0x0A00"  # Windows 10 API
)

# 链接选项 (Windows CryptoAPI)
$LDFLAGS = @(
    "-lcrypt32"
)

# 构建完整命令
$COMMAND = "g++"
$ARGS = $CXXFLAGS + $INCLUDES + $SOURCES + $LDFLAGS + @("-o", $OUTPUT)

Write-Host "Compiling with MinGW g++..." -ForegroundColor Yellow
Write-Host "Output: $OUTPUT" -ForegroundColor Yellow

# 执行编译
try {
    $processInfo = Start-Process -FilePath $COMMAND `
        -ArgumentList $ARGS `
        -NoNewWindow `
        -PassThru `
        -Wait
    
    if ($processInfo.ExitCode -eq 0) {
        Write-Host "`nBuild successful!" -ForegroundColor Green
        Write-Host "Executable: $(Get-Location)\$OUTPUT" -ForegroundColor Green
        
        # 显示文件信息
        $fileSize = (Get-Item $OUTPUT).Length / 1KB
        Write-Host "File size: $([math]::Round($fileSize, 2)) KB" -ForegroundColor Gray
    } else {
        Write-Host "`nBuild failed with exit code $($processInfo.ExitCode)" -ForegroundColor Red
    }
} catch {
    Write-Host "`nError during compilation: $_" -ForegroundColor Red
    Write-Host "`nMake sure MinGW g++ is installed and in PATH" -ForegroundColor Yellow
}
