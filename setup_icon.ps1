# Icon Creation Helper for Aakalan

Write-Host "========================================"
Write-Host "  Aakalan Icon Setup Helper"
Write-Host "========================================"
Write-Host ""

$iconPath = "icon.ico"

if (Test-Path $iconPath) {
    Write-Host "[OK] Icon file found: $iconPath" -ForegroundColor Green
    Write-Host ""
    Write-Host "Current icon will be used in the build." -ForegroundColor Yellow
    Write-Host "To replace it, delete icon.ico and add your new one." -ForegroundColor Yellow
} else {
    Write-Host "[!] Icon file not found: $iconPath" -ForegroundColor Red
    Write-Host ""
    Write-Host "HOW TO ADD A CUSTOM ICON:" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "METHOD 1: Convert your image online" -ForegroundColor Yellow
    Write-Host "  1. Go to https://convertio.co/png-ico/"
    Write-Host "  2. Upload your logo (PNG, JPG, etc.)"
    Write-Host "  3. Convert and download as icon.ico"
    Write-Host "  4. Save it in this folder: $PWD"
    Write-Host ""
    Write-Host "METHOD 2: Download a free icon" -ForegroundColor Yellow
    Write-Host "  1. Visit https://www.flaticon.com/"
    Write-Host "  2. Search for 'student' or 'education'"
    Write-Host "  3. Download in ICO format (256x256)"
    Write-Host "  4. Rename to icon.ico"
    Write-Host "  5. Save it in this folder"
    Write-Host ""
    Write-Host "METHOD 3: Create a simple default icon now" -ForegroundColor Yellow
    $create = Read-Host "Create a simple blue 'A' icon now? (y/n)"
    
    if ($create -eq 'y' -or $create -eq 'Y') {
        Write-Host "Creating default icon..." -ForegroundColor Cyan
        
        # Try to download a simple icon
        try {
            $url = "https://icons.iconarchive.com/icons/paomedia/small-n-flat/256/sign-check-icon.png"
            Invoke-WebRequest -Uri $url -OutFile "temp_icon.png" -ErrorAction Stop
            Write-Host "[OK] Downloaded temporary icon" -ForegroundColor Green
            Write-Host ""
            Write-Host "NOTE: You can now:" -ForegroundColor Yellow
            Write-Host "  - Use an online converter to turn temp_icon.png into icon.ico"
            Write-Host "  - Or replace temp_icon.png with your own image"
        } catch {
            Write-Host "[!] Could not download sample icon" -ForegroundColor Red
            Write-Host "Please add your icon.ico file manually" -ForegroundColor Yellow
        }
    }
}

Write-Host ""
Write-Host "NEXT STEPS:" -ForegroundColor Cyan
Write-Host "1. Add icon.ico to this folder"
Write-Host "2. Rebuild: make clean && make"
Write-Host "3. Update portable: .\create_portable.ps1"
Write-Host ""
