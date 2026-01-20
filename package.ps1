param(
    [string]$OutputName = "Aakalan"
)

Write-Host "`n=== Aakalan Portable Packager ===" -ForegroundColor Cyan
Write-Host "Creating standalone executable bundle...`n" -ForegroundColor White

# Check if executable exists
if (-not (Test-Path "build\assignment_tracker.exe")) {
    Write-Host "Error: build\assignment_tracker.exe not found!" -ForegroundColor Red
    Write-Host "Run 'make' first to build the application." -ForegroundColor Yellow
    exit 1
}

# Create output directory
$outDir = $OutputName
if (Test-Path $outDir) {
    Remove-Item $outDir -Recurse -Force
}
New-Item -ItemType Directory -Path $outDir | Out-Null
New-Item -ItemType Directory -Path "$outDir\data" | Out-Null

Write-Host "[1/5] Created output directory: $outDir" -ForegroundColor Green

# Copy main executable
Copy-Item "build\assignment_tracker.exe" "$outDir\$OutputName.exe"
Write-Host "[2/5] Copied main executable" -ForegroundColor Green

# Find MSYS2/MinGW installation
$mingwPaths = @(
    "C:\msys64\mingw64\bin",
    "C:\msys64\ucrt64\bin",
    "D:\msys64\mingw64\bin",
    "D:\msys64\ucrt64\bin",
    "$env:MSYSTEM_PREFIX\bin"
)

$mingwBin = $null
foreach ($path in $mingwPaths) {
    if (Test-Path $path) {
        $mingwBin = $path
        break
    }
}

if (-not $mingwBin) {
    Write-Host "Error: MSYS2/MinGW not found!" -ForegroundColor Red
    Write-Host "Install from: https://www.msys2.org/" -ForegroundColor Yellow
    exit 1
}

Write-Host "[3/5] Found MinGW at: $mingwBin" -ForegroundColor Green

# Core DLLs needed
$coreDlls = @(
    # GTK core
    'libgtk-3-0.dll',
    'libgdk-3-0.dll',
    'libglib-2.0-0.dll',
    'libgobject-2.0-0.dll',
    'libgio-2.0-0.dll',
    
    # Graphics
    'libcairo-2.dll',
    'libcairo-gobject-2.dll',
    'libpango-1.0-0.dll',
    'libpangocairo-1.0-0.dll',
    'libpangoft2-1.0-0.dll',
    'libpangowin32-1.0-0.dll',
    'libgdk_pixbuf-2.0-0.dll',
    'libatk-1.0-0.dll',
    
    # Database & Security
    'libsqlite3-0.dll',
    'libssl-3-x64.dll',
    'libcrypto-3-x64.dll',
    
    # Runtime
    'libintl-8.dll',
    'libwinpthread-1.dll',
    'libgcc_s_seh-1.dll',
    'libstdc++-6.dll',
    
    # Compression & Image
    'zlib1.dll',
    'libpng16-16.dll',
    'libjpeg-8.dll',
    'libtiff-6.dll',
    
    # Text rendering
    'libharfbuzz-0.dll',
    'libfreetype-6.dll',
    'libfontconfig-1.dll',
    'libfribidi-0.dll',
    
    # Other dependencies
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
    'libffi-8.dll'
)

# Copy DLLs
$copiedCount = 0
$missingDlls = @()

foreach ($dll in $coreDlls) {
    $srcPath = Join-Path $mingwBin $dll
    if (Test-Path $srcPath) {
        Copy-Item $srcPath $outDir -ErrorAction SilentlyContinue
        $copiedCount++
    } else {
        $missingDlls += $dll
    }
}

Write-Host "[4/5] Copied $copiedCount DLL files" -ForegroundColor Green
if ($missingDlls.Count -gt 0) {
    Write-Host "      (Skipped $($missingDlls.Count) optional DLLs)" -ForegroundColor DarkGray
}

# Create launcher batch file
$launcher = @"
@echo off
title Aakalan - Student Management System

REM Set current directory as base
set APP_DIR=%~dp0

REM Run the application
"%APP_DIR%$OutputName.exe"

REM Exit code handling
if errorlevel 1 (
    echo.
    echo Application exited with error code %errorlevel%
    echo Press any key to close...
    pause >nul
)
"@

Set-Content "$outDir\Run $OutputName.bat" -Value $launcher -Encoding ASCII
Write-Host "[5/5] Created launcher script" -ForegroundColor Green

# Create README
$readme = @"
╔══════════════════════════════════════════════════════════════╗
║          Aakalan - Student Management System                 ║
║                  Portable Edition v1.0                       ║
╚══════════════════════════════════════════════════════════════╝

📌 QUICK START
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Double-click: "Run $OutputName.bat"

✨ FEATURES
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ Multi-class management
✓ Student records with email & phone
✓ Attendance tracking with date selection
✓ Assignment management & submission tracking
✓ Comprehensive reports & analytics
✓ Secure password encryption with salt
✓ Data validation & error handling
✓ Fully portable - runs from any folder/USB drive

🖥️ SYSTEM REQUIREMENTS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
• Windows 10 or later (64-bit)
• 50 MB free disk space
• No installation required!

🚀 FIRST TIME SETUP
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1. Run "$OutputName.bat"
2. Click "Register" to create your account
   (Password must be 8+ chars with uppercase, lowercase, digit, special char)
3. Login with your credentials
4. Create your first class
5. Start managing students and assignments!

💾 DATA LOCATION
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Your data is stored in: data\assignment_tracker.db

⚠️ IMPORTANT: Keep the "data" folder to preserve your information
             when moving or copying the application.

📦 DISTRIBUTION
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
To share with others:
1. Copy the entire "$OutputName" folder
2. Share via USB drive, cloud storage, or zip file
3. Recipient just runs "Run $OutputName.bat"

🔒 SECURITY FEATURES
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
• SHA-256 password hashing with random salt
• SQL injection protection (parameterized queries)
• Email & roll number duplicate detection
• Input validation on all forms
• Password complexity requirements

🛠️ TECHNICAL INFO
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Built with: C (C11 standard), GTK 3, SQLite 3, OpenSSL
License: Educational/Academic Use
Version: 1.0
Build Date: $(Get-Date -Format 'yyyy-MM-dd')

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Made with ❤️ for educational purposes
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
"@

Set-Content "$outDir\README.txt" -Value $readme -Encoding UTF8

# Calculate total size
$totalSize = (Get-ChildItem $outDir -Recurse | Measure-Object -Property Length -Sum).Sum
$sizeMB = [math]::Round($totalSize / 1MB, 2)
$fileCount = (Get-ChildItem $outDir -Recurse -File | Measure-Object).Count

# Summary
Write-Host ""
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  Portable Bundle Created Successfully!" -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Location:  $outDir\" -ForegroundColor White
Write-Host "Size:      $sizeMB MB" -ForegroundColor White
Write-Host "Files:     $fileCount" -ForegroundColor White
Write-Host "Launcher:  Run $OutputName.bat" -ForegroundColor White
Write-Host ""
Write-Host "TO RUN NOW:" -ForegroundColor Yellow
Write-Host "  cd $outDir" -ForegroundColor White
Write-Host "  .\Run`` $OutputName.bat" -ForegroundColor White
Write-Host ""
Write-Host "TO DISTRIBUTE:" -ForegroundColor Yellow
Write-Host "  1. Zip the '$outDir' folder" -ForegroundColor White
Write-Host "  2. Share with anyone" -ForegroundColor White
Write-Host "  3. Works on any Windows 10+ PC!" -ForegroundColor White
Write-Host ""
