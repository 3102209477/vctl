# Version Control Project - Build & Package Script
# 使用方法：.\build_package.ps1

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Version Control Tool - Build Package" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 设置路径
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$packageDir = Join-Path $projectRoot "..\vctl_project_$timestamp"
$outputZip = Join-Path $projectRoot "..\vctl_project_$timestamp.zip"

Write-Host "Project Root: $projectRoot" -ForegroundColor Yellow
Write-Host "Package Dir: $packageDir" -ForegroundColor Yellow
Write-Host "Output ZIP: $outputZip" -ForegroundColor Yellow
Write-Host ""

# 创建打包目录
Write-Host "[1/5] Creating package directory..." -ForegroundColor Green
New-Item -ItemType Directory -Path $packageDir -Force | Out-Null

# 复制源代码目录
Write-Host "[2/5] Copying source directories..." -ForegroundColor Green
$dirs = @("commands", "config", "core", "include", "storage", "utils")
foreach ($dir in $dirs) {
    $srcPath = Join-Path $projectRoot $dir
    $destPath = Join-Path $packageDir $dir
    if (Test-Path $srcPath) {
        Copy-Item -Path $srcPath -Destination $destPath -Recurse -Force
        Write-Host "  ✓ Copied: $dir" -ForegroundColor Gray
    }
}

# 复制根目录文件
Write-Host "[3/5] Copying root files..." -ForegroundColor Green
$rootFiles = @("main.cpp", "*.md", "*.bat", "*.cpp")
foreach ($pattern in $rootFiles) {
    $files = Get-ChildItem -Path $projectRoot -Filter $pattern -File
    foreach ($file in $files) {
        if ($file.Name -notlike "build_*.ps1") {
            Copy-Item -Path $file.FullName -Destination $packageDir -Force
        }
    }
}

# 复制可执行文件
Write-Host "[4/5] Copying executable files..." -ForegroundColor Green
$binDir = Join-Path $packageDir "bin"
New-Item -ItemType Directory -Path $binDir -Force | Out-Null

$exeFiles = @("vctl.exe", "test_format.exe")
foreach ($exe in $exeFiles) {
    $exePath = Join-Path $projectRoot $exe
    if (Test-Path $exePath) {
        Copy-Item -Path $exePath -Destination $binDir -Force
        Write-Host "  ✓ Copied: $exe" -ForegroundColor Gray
    }
}

# 创建 README 和清单
Write-Host "[5/5] Creating documentation..." -ForegroundColor Green

$readmeContent = @"
# Version Control Project Package

## 打包信息
- **时间**: $timestamp
- **位置**: $packageDir
- **大小**: $(Get-ChildItem $packageDir -Recurse | Measure-Object -Property Length -Sum | ForEach-Object { "{0:N2} MB" -f ($_.Sum / 1MB) })

## 目录结构
``````
$(tree /F $packageDir | Select-Object -First 40)
``````

## 快速开始
1. 使用 bin\vctl.exe
2. 或重新编译：g++ -std=c++17 ... main.cpp ... -o vctl.exe

详细文档见 *.md 文件
"@

$readmeContent | Out-File -FilePath (Join-Path $packageDir "PACKAGE_INFO.md") -Encoding UTF8
Write-Host "  ✓ Created: PACKAGE_INFO.md" -ForegroundColor Gray

# 显示统计信息
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Package Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

$fileCount = (Get-ChildItem $packageDir -Recurse -File).Count
$totalSize = Get-ChildItem $packageDir -Recurse | Measure-Object -Property Length -Sum | ForEach-Object { "{0:N2} MB" -f ($_.Sum / 1MB) }

Write-Host "Package Directory: $packageDir" -ForegroundColor Yellow
Write-Host "Total Files: $fileCount" -ForegroundColor Yellow
Write-Host "Total Size: $totalSize" -ForegroundColor Yellow
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Review files in: $packageDir" -ForegroundColor White
Write-Host "  2. Compress to ZIP manually or use: Compress-Archive -Path $packageDir -DestinationPath $outputZip" -ForegroundColor White
Write-Host "  3. Test the package before distribution" -ForegroundColor White
Write-Host ""
