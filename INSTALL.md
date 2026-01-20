# Complete Installation and Build Guide

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Installation Steps](#installation-steps)
3. [Building the Project](#building-the-project)
4. [Running the Application](#running-the-application)
5. [Troubleshooting](#troubleshooting)
6. [Testing the Application](#testing-the-application)

---

## Prerequisites

### Hardware Requirements
- **Processor**: Any modern x64 processor
- **RAM**: 2 GB minimum (4 GB recommended)
- **Storage**: 500 MB for MSYS2 + dependencies
- **Display**: 800x600 minimum resolution

### Software Requirements
- **OS**: Windows 7 or later (64-bit)
- **MSYS2**: Development environment for Windows

---

## Installation Steps

### Step 1: Install MSYS2

1. **Download MSYS2**
   - Visit: https://www.msys2.org/
   - Download the installer (msys2-x86_64-*.exe)
   - File size: ~80 MB

2. **Run the Installer**
   - Double-click the downloaded file
   - Install to default location: `C:\msys64`
   - Check "Run MSYS2 now" at the end
   - Click Finish

3. **Update MSYS2**
   - In the opened MSYS2 terminal, run:
   ```bash
   pacman -Syu
   ```
   - When prompted, press Y and Enter
   - If the terminal closes, reopen **MSYS2 MSYS** and run:
   ```bash
   pacman -Su
   ```

### Step 2: Install Development Tools

1. **Close all MSYS2 windows**

2. **Open MSYS2 MinGW 64-bit terminal**
   - Search for "MSYS2 MinGW 64-bit" in Start menu
   - **Important**: Must use MinGW 64-bit, not MSYS or MinGW 32-bit!

3. **Install all required packages** (copy-paste each line):
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-gtk3
   pacman -S mingw-w64-x86_64-sqlite3
   pacman -S mingw-w64-x86_64-openssl
   pacman -S mingw-w64-x86_64-pkg-config
   pacman -S make
   ```
   
   Or install all at once:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk3 mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-openssl mingw-w64-x86_64-pkg-config make
   ```
   - Press Enter when prompted
   - Total download size: ~150-200 MB
   - Installation time: 5-10 minutes

4. **Verify Installation**
   ```bash
   gcc --version
   pkg-config --version
   ```
   - Both should show version information

### Step 3: Verify Environment

Run the verification script:
```bash
cd "/d/Professional/C Final Prokect"
bash check_setup.sh
```

Expected output:
```
✓ gcc is installed
✓ make is installed
✓ pkg-config is installed
✓ gtk+-3.0 is available
✓ sqlite3 is available
✓ OpenSSL is available
✓ Project directories exist
✓ Makefile exists

All checks passed!
```

---

## Building the Project

### First Build

1. **Navigate to project directory**
   ```bash
   cd "/d/Professional/C Final Prokect"
   ```
   
   Note: In MSYS2, Windows drives are accessed as:
   - C: becomes /c/
   - D: becomes /d/
   - etc.

2. **Check Makefile targets**
   ```bash
   make help
   ```

3. **Build the application**
   ```bash
   make
   ```
   
   You should see:
   ```
   Creating build directory...
   Creating data directory...
   Compiling src/common.c...
   Compiling src/database.c...
   ...
   Linking executable...
   =========================================
   Build complete: build/assignment_tracker.exe
   Run with: make run
   =========================================
   ```

### Rebuild After Changes

```bash
make clean    # Remove old build
make          # Build fresh
```

Or simply:
```bash
make rebuild
```

---

## Running the Application

### Method 1: Using Make (Recommended)
```bash
make run
```

### Method 2: Direct Execution
```bash
./build/assignment_tracker.exe
```

### Method 3: Windows Explorer (After PATH setup)
Double-click `assignment_tracker.exe` in the `build` folder

**Note**: For Method 3 to work, add to Windows PATH:
- `C:\msys64\mingw64\bin`

---

## Troubleshooting

### Build Errors

#### Error: "gtk/gtk.h: No such file or directory"

**Cause**: Using wrong terminal or GTK not installed

**Solution**:
1. Make sure you're in **MSYS2 MinGW 64-bit** terminal
2. Install GTK:
   ```bash
   pacman -S mingw-w64-x86_64-gtk3
   ```
3. Verify:
   ```bash
   pkg-config --cflags gtk+-3.0
   ```

#### Error: "sqlite3.h: No such file or directory"

**Solution**:
```bash
pacman -S mingw-w64-x86_64-sqlite3
```

#### Error: "openssl/sha.h: No such file or directory"

**Solution**:
```bash
pacman -S mingw-w64-x86_64-openssl
```

#### Error: "make: command not found"

**Solution**:
```bash
pacman -S make
```

#### Error: "undefined reference to `gtk_init`"

**Cause**: Linking issue, missing -lcrypto flag

**Solution**: The Makefile should have:
```makefile
LDFLAGS = `pkg-config --libs gtk+-3.0` -lsqlite3 -lcrypto -lm
```

### Runtime Errors

#### Application window doesn't appear

**Cause**: GTK libraries not in PATH

**Solution**: Run from MSYS2 MinGW 64-bit terminal, not Windows CMD

#### "Failed to initialize database"

**Cause**: Can't create data directory

**Solution**:
1. Check write permissions
2. Manually create: `mkdir data`

#### Application crashes immediately

**Solution**:
1. Run from MSYS2 terminal to see error messages
2. Check if all DLLs are available:
   ```bash
   ldd build/assignment_tracker.exe
   ```

### Login Issues

#### "Invalid email or password" on first use

**Cause**: You haven't registered yet

**Solution**: Click "Don't have an account? Register"

#### Can't register: "Account already exists"

**Cause**: Email already in database

**Solution**: Use a different email or delete database:
```bash
rm data/assignment_tracker.db
```

### Student Management Issues

#### "Email or roll number already exists"

**Cause**: Duplicate entries

**Solution**: Each student must have unique email and roll number

---

## Testing the Application

### Test Checklist

#### 1. User Registration
- [ ] Open application
- [ ] Click "Don't have an account? Register"
- [ ] Enter name: "Test User"
- [ ] Enter email: "test@example.com"
- [ ] Enter password: "password123"
- [ ] Click "Register"
- [ ] Success message should appear

#### 2. User Login
- [ ] Click "Already have an account? Login"
- [ ] Enter registered email
- [ ] Enter password
- [ ] Click "Login"
- [ ] Dashboard should appear

#### 3. Student Management
- [ ] Click "Student Management" card
- [ ] Student window should open
- [ ] Enter student details:
  - Name: "John Doe"
  - Email: "john@example.com"
  - Roll: "CS001"
  - Phone: "1234567890"
- [ ] Click "Add"
- [ ] Student appears in list
- [ ] Click on student in list
- [ ] Details fill the form
- [ ] Modify name to "John Smith"
- [ ] Click "Update"
- [ ] Name updates in list
- [ ] Select student and click "Delete"
- [ ] Confirm deletion
- [ ] Student removed from list

#### 4. Data Persistence
- [ ] Add several students
- [ ] Close application
- [ ] Reopen application
- [ ] Login again
- [ ] Open Student Management
- [ ] Verify students are still there

#### 5. Validation Testing
- [ ] Try adding student without email → Error
- [ ] Try adding student with invalid email → Error
- [ ] Try adding duplicate roll number → Error
- [ ] Try login with wrong password → Error

---

## Additional Information

### File Locations

- **Executable**: `build/assignment_tracker.exe`
- **Database**: `data/assignment_tracker.db`
- **Source Code**: `src/*.c`
- **Headers**: `include/*.h`

### Database Backup

To backup your data:
```bash
cp data/assignment_tracker.db data/backup_$(date +%Y%m%d).db
```

To restore:
```bash
cp data/backup_YYYYMMDD.db data/assignment_tracker.db
```

### Resetting Everything

To start fresh:
```bash
make clean
rm -f data/*.db
make
```

### Getting Help

1. Check error messages in terminal
2. Read [README.md](README.md) for detailed documentation
3. Review [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) for architecture
4. Check [QUICKSTART.md](QUICKSTART.md) for quick reference

---

## Success Indicators

You've successfully set up the project if:

✅ `make` completes without errors  
✅ `make run` launches the application  
✅ You can register and login  
✅ You can add, edit, delete students  
✅ Data persists after restart  
✅ All dialogs show properly  

---

## Next Steps After Installation

1. **Explore the Code**
   - Review modular architecture in `src/` and `include/`
   - Understand database operations in `database.c`
   - Study GUI creation in `ui_*.c` files

2. **Extend the Application**
   - Implement full Assignment Tracking module
   - Complete Attendance Management
   - Add Reports and Analytics

3. **Customize**
   - Add more validation rules
   - Enhance UI with icons and colors
   - Add export functionality

4. **Document Your Learning**
   - Take screenshots of running application
   - Document challenges and solutions
   - Prepare presentation for project defense

---

**Congratulations! You now have a fully functional C-based GUI application with database integration!**
