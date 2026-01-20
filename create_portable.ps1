Write-Host "Creating Complete Aakalan Portable Package..." -ForegroundColor Cyan
Write-Host ""

# Create directory structure
$outDir = "Aakalan"
New-Item -ItemType Directory -Path $outDir -Force | Out-Null
New-Item -ItemType Directory -Path "$outDir\data" -Force | Out-Null
Write-Host "[1/4] Created directory structure" -ForegroundColor Green

# Copy executable
Copy-Item "build\assignment_tracker.exe" "$outDir\Aakalan.exe"
Write-Host "[2/4] Copied executable" -ForegroundColor Green

# COMPLETE list of all GTK dependencies
$allDlls = @(
    'libgtk-3-0.dll',
    'libgdk-3-0.dll',
    'libglib-2.0-0.dll',
    'libgobject-2.0-0.dll',
    'libgio-2.0-0.dll',
    'libgmodule-2.0-0.dll',
    'libgthread-2.0-0.dll',
    'libcairo-2.dll',
    'libcairo-gobject-2.dll',
    'libpango-1.0-0.dll',
    'libpangocairo-1.0-0.dll',
    'libpangoft2-1.0-0.dll',
    'libpangowin32-1.0-0.dll',
    'libgdk_pixbuf-2.0-0.dll',
    'libatk-1.0-0.dll',
    'libsqlite3-0.dll',
    'libssl-3-x64.dll',
    'libcrypto-3-x64.dll',
    'libintl-8.dll',
    'libwinpthread-1.dll',
    'libgcc_s_seh-1.dll',
    'libstdc++-6.dll',
    'zlib1.dll',
    'libpng16-16.dll',
    'libjpeg-8.dll',
    'libtiff-6.dll',
    'libharfbuzz-0.dll',
    'libfreetype-6.dll',
    'libfontconfig-1.dll',
    'libfribidi-0.dll',
    'libepoxy-0.dll',
    'libpixman-1-0.dll',
    'libpcre2-8-0.dll',
    'libiconv-2.dll',
    'libbz2-1.dll',
    'libexpat-1.dll',
    'libbrotlidec.dll',
    'libbrotlicommon.dll',
    'libgraphite2.dll',
    'libxml2-2.dll',
    'liblzma-5.dll',
    'libzstd.dll',
    'libffi-8.dll',
    'libthai-0.dll',
    'libdatrie-1.dll',
    'librsvg-2-2.dll'
)

$mingwBin = "C:\msys64\mingw64\bin"
$copiedCount = 0
$missingCount = 0

foreach ($dll in $allDlls) {
    $srcPath = Join-Path $mingwBin $dll
    if (Test-Path $srcPath) {
        Copy-Item $srcPath $outDir -Force
        $copiedCount++
    } else {
        $missingCount++
    }
}

Write-Host "[3/4] Copied $copiedCount DLL files" -ForegroundColor Green
if ($missingCount -gt 0) {
    Write-Host "      ($missingCount optional DLLs not found, but app should still work)" -ForegroundColor DarkGray
}

# Create launcher
$launcher = @"
@echo off
cd /d "%~dp0"
set PATH=%CD%;%PATH%
start "" "%~dp0Aakalan.exe"
"@

Set-Content "$outDir\Start.bat" -Value $launcher -Encoding ASCII
Write-Host "[4/4] Created launcher" -ForegroundColor Green

# Create README
$readme = @"
AAKALAN - STUDENT MANAGEMENT SYSTEM
Portable Edition v1.0

QUICK START:
Double-click Start.bat to run the application

FEATURES:
- Multi-class management
- Student attendance tracking  
- Assignment submission tracking
- Comprehensive reports
- Secure password encryption
- Fully portable

FIRST TIME USE:
1. Double-click Start.bat
2. Click Register to create account
   Password must be 8+ characters with:
   - Uppercase letter
   - Lowercase letter
   - Number
   - Special character
3. Login and create your first class
4. Start managing students!

DATA:
Your database is in the data folder.
Keep this folder to preserve your data.

REQUIREMENTS:
Windows 10/11 (64-bit)
No installation needed!
"@

Set-Content "$outDir\README.txt" -Value $readme -Encoding UTF8

$totalSize = (Get-ChildItem $outDir -Recurse | Measure-Object -Property Length -Sum).Sum
$sizeMB = [math]::Round($totalSize / 1MB, 2)

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  SUCCESS! Portable package created" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Location: $outDir" -ForegroundColor White
Write-Host "Size: $sizeMB MB" -ForegroundColor White
Write-Host ""
Write-Host "TO RUN:" -ForegroundColor Yellow
Write-Host "  cd $outDir" -ForegroundColor White
Write-Host "  Double-click Start.bat" -ForegroundColor White
Write-Host ""
