#!/bin/bash

# Assignment Tracker - Environment Check Script
# Run this in MSYS2 MinGW 64-bit terminal to verify your setup

echo "========================================="
echo "Assignment Tracker - Environment Check"
echo "========================================="
echo ""

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "${GREEN}✓${NC} $1 is installed"
        return 0
    else
        echo -e "${RED}✗${NC} $1 is NOT installed"
        return 1
    fi
}

check_pkg_config() {
    if pkg-config --exists $1; then
        echo -e "${GREEN}✓${NC} $1 is available"
        return 0
    else
        echo -e "${RED}✗${NC} $1 is NOT available"
        return 1
    fi
}

ERRORS=0

echo "Checking build tools..."
check_command gcc || ((ERRORS++))
check_command make || ((ERRORS++))
check_command pkg-config || ((ERRORS++))
echo ""

echo "Checking libraries..."
check_pkg_config gtk+-3.0 || ((ERRORS++))
check_pkg_config sqlite3 || ((ERRORS++))

# Check OpenSSL/crypto
if pkg-config --exists libcrypto; then
    echo -e "${GREEN}✓${NC} OpenSSL (libcrypto) is available"
elif pkg-config --exists openssl; then
    echo -e "${GREEN}✓${NC} OpenSSL is available"
else
    echo -e "${RED}✗${NC} OpenSSL is NOT available"
    ((ERRORS++))
fi
echo ""

echo "Version information:"
echo "-------------------"
gcc --version | head -n 1
make --version | head -n 1
pkg-config --version

echo ""
echo "GTK version: $(pkg-config --modversion gtk+-3.0 2>/dev/null || echo 'Not found')"
echo "SQLite version: $(pkg-config --modversion sqlite3 2>/dev/null || echo 'Not found')"
echo ""

echo "Checking project structure..."
if [ -d "src" ] && [ -d "include" ]; then
    echo -e "${GREEN}✓${NC} Project directories exist"
else
    echo -e "${RED}✗${NC} Project directories missing"
    ((ERRORS++))
fi

if [ -f "Makefile" ]; then
    echo -e "${GREEN}✓${NC} Makefile exists"
else
    echo -e "${RED}✗${NC} Makefile missing"
    ((ERRORS++))
fi
echo ""

echo "========================================="
if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}All checks passed!${NC}"
    echo "You can build the project with: make"
else
    echo -e "${RED}Found $ERRORS issue(s)${NC}"
    echo ""
    echo "To fix missing packages in MSYS2, run:"
    echo "  pacman -S mingw-w64-x86_64-gcc"
    echo "  pacman -S mingw-w64-x86_64-gtk3"
    echo "  pacman -S mingw-w64-x86_64-sqlite3"
    echo "  pacman -S mingw-w64-x86_64-openssl"
    echo "  pacman -S mingw-w64-x86_64-pkg-config"
    echo "  pacman -S make"
fi
echo "========================================="
