# Build Instructions for Windows

This guide will help you set up the development environment and build the Assignment Tracker application on Windows.

## Step 1: Install MSYS2

1. Download MSYS2 from https://www.msys2.org/
2. Run the installer and follow the installation steps
3. Install to the default location: `C:\msys64`

## Step 2: Update MSYS2

1. After installation, open **MSYS2 MSYS** terminal (from Start menu)
2. Run the update command:
```bash
pacman -Syu
```
3. If the terminal closes, reopen it and run again:
```bash
pacman -Su
```

## Step 3: Install Development Tools

1. Close the MSYS2 MSYS terminal
2. Open **MSYS2 MinGW 64-bit** terminal (this is important!)
3. Install the required packages:

```bash
# Install GCC compiler
pacman -S mingw-w64-x86_64-gcc

# Install GTK 3
pacman -S mingw-w64-x86_64-gtk3

# Install SQLite 3
pacman -S mingw-w64-x86_64-sqlite3

# Install OpenSSL
pacman -S mingw-w64-x86_64-openssl

# Install pkg-config
pacman -S mingw-w64-x86_64-pkg-config

# Install make
pacman -S make
```

When prompted, press Enter to install all packages.

## Step 4: Add MSYS2 to PATH (Optional but Recommended)

To run the application outside MSYS2 terminal:

1. Open "Edit the system environment variables" from Start menu
2. Click "Environment Variables"
3. Under "System variables", find "Path" and click "Edit"
4. Click "New" and add: `C:\msys64\mingw64\bin`
5. Click OK on all dialogs

## Step 5: Navigate to Project

In the MSYS2 MinGW 64-bit terminal:

```bash
# Change to your project directory
# Windows paths use /c/ instead of C:\
cd "/d/Professional/C Final Prokect"
```

## Step 6: Build the Project

```bash
# Clean any previous builds
make clean

# Build the project
make
```

You should see compilation messages. If successful, you'll see:
```
Build complete: build/assignment_tracker.exe
```

## Step 7: Run the Application

```bash
# Run from the build directory
make run

# Or run directly
./build/assignment_tracker.exe
```

## Troubleshooting

### Problem: "gtk/gtk.h: No such file or directory"

**Solution**: You forgot to open MSYS2 MinGW 64-bit terminal. Close the current terminal and open the correct one.

### Problem: "command not found: make"

**Solution**: Install make:
```bash
pacman -S make
```

### Problem: "pkg-config not found"

**Solution**: Install pkg-config:
```bash
pacman -S mingw-w64-x86_64-pkg-config
```

### Problem: Build succeeds but application crashes immediately

**Solution**: Make sure you're running from MSYS2 MinGW 64-bit terminal, not Windows Command Prompt or PowerShell. The application needs GTK DLLs from MSYS2.

### Problem: "cannot open shared object file: libgtk-3-0.dll"

**Solution**: Either:
1. Run from MSYS2 MinGW 64-bit terminal, OR
2. Add `C:\msys64\mingw64\bin` to your Windows PATH

## Verifying Installation

Test if all tools are correctly installed:

```bash
# Test GCC
gcc --version

# Test pkg-config
pkg-config --version

# Test GTK
pkg-config --cflags gtk+-3.0

# Test SQLite
pkg-config --libs sqlite3
```

All commands should return version information without errors.

## Alternative: Using Visual Studio Code

1. Install the C/C++ extension in VS Code
2. Open MSYS2 MinGW 64-bit terminal
3. Navigate to project directory
4. Run: `code .`
5. Use the integrated terminal in VS Code (make sure it's using MSYS2 MinGW bash)
6. Build with `make`

## Next Steps

After successful build:

1. The database will be automatically created in `data/assignment_tracker.db`
2. Start by registering a new user account
3. Explore the Student Management module
4. Refer to README.md for usage instructions

## Common Build Warnings

These warnings are normal and can be ignored:
- Unused parameter warnings
- Deprecated GTK function warnings

## Getting Help

If you encounter issues:
1. Check that you're using MSYS2 MinGW 64-bit terminal
2. Verify all packages are installed: `pacman -Q | grep mingw`
3. Make sure pkg-config works for all dependencies
4. Check the main README.md for more information
