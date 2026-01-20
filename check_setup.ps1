# PowerShell Setup Verification Script
# Run this to check if MSYS2 is installed

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Assignment Tracker - Windows Setup Check" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

$errors = 0

# Check if MSYS2 is installed
Write-Host "Checking for MSYS2..." -NoNewline
if (Test-Path "C:\msys64\mingw64\bin\gcc.exe") {
    Write-Host " OK" -ForegroundColor Green
    
    # Check for required packages
    $packages = @(
        @{Name="GCC"; Path="C:\msys64\mingw64\bin\gcc.exe"},
        @{Name="GTK3"; Path="C:\msys64\mingw64\lib\pkgconfig\gtk+-3.0.pc"},
        @{Name="SQLite3"; Path="C:\msys64\mingw64\lib\pkgconfig\sqlite3.pc"},
        @{Name="OpenSSL"; Path="C:\msys64\mingw64\lib\pkgconfig\libcrypto.pc"},
        @{Name="pkg-config"; Path="C:\msys64\mingw64\bin\pkg-config.exe"},
        @{Name="make"; Path="C:\msys64\usr\bin\make.exe"}
    )
    
    Write-Host ""
    Write-Host "Checking required packages:" -ForegroundColor Yellow
    foreach ($pkg in $packages) {
        Write-Host "  $($pkg.Name)..." -NoNewline
        if (Test-Path $pkg.Path) {
            Write-Host " OK" -ForegroundColor Green
        } else {
            Write-Host " MISSING" -ForegroundColor Red
            $errors++
        }
    }
} else {
    Write-Host " NOT INSTALLED" -ForegroundColor Red
    $errors++
    Write-Host ""
    Write-Host "MSYS2 is not installed!" -ForegroundColor Red
}

Write-Host ""
Write-Host "Checking project structure..." -NoNewline
if ((Test-Path "src") -and (Test-Path "include") -and (Test-Path "Makefile")) {
    Write-Host " OK" -ForegroundColor Green
} else {
    Write-Host " MISSING FILES" -ForegroundColor Red
    $errors++
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
if ($errors -eq 0) {
    Write-Host "All checks passed!" -ForegroundColor Green
    Write-Host ""
    Write-Host "To build the project:" -ForegroundColor Yellow
    Write-Host "1. Open 'MSYS2 MinGW 64-bit' from Start Menu" -ForegroundColor White
    Write-Host "2. Run: cd '/d/Professional/C Final Prokect'" -ForegroundColor White
    Write-Host "3. Run: make" -ForegroundColor White
    Write-Host "4. Run: make run" -ForegroundColor White
} else {
    Write-Host "Found $errors issue(s)" -ForegroundColor Red
    Write-Host ""
    Write-Host "To fix:" -ForegroundColor Yellow
    Write-Host "1. Download MSYS2 from: https://www.msys2.org/" -ForegroundColor White
    Write-Host "2. Install to C:\msys64" -ForegroundColor White
    Write-Host "3. Open 'MSYS2 MinGW 64-bit' terminal" -ForegroundColor White
    Write-Host "4. Run: pacman -Syu" -ForegroundColor White
    Write-Host "5. Run: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-openssl mingw-w64-x86_64-pkg-config make" -ForegroundColor White
}
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
