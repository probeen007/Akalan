<div align="center">
  <img src="akalan.ico" alt="Aakalan Logo" width="128" height="128">
  
  # Aakalan - Student Management System
  
  **A comprehensive GUI-based offline application built entirely in C for complete student lifecycle management**
  
  ![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-blue)
  ![Language](https://img.shields.io/badge/language-C-orange)
  ![GTK](https://img.shields.io/badge/GUI-GTK%203-green)
  ![Database](https://img.shields.io/badge/database-SQLite-lightblue)
</div>

---

## ✨ Features

### 🔐 Security & Authentication
- **Secure Login System** with email/password authentication
- **SHA-256 password hashing** with unique salt per user
- **SQL injection prevention** using prepared statements
- **Session management** with secure user context

### 👥 Student Management
- **Multi-class support** - manage multiple classes simultaneously
- **Add, edit, delete, and search** student records
- **Detailed student profiles** with roll number, name, email, and class
- **Real-time student list** with TreeView display
- **Bulk operations** support

### 📚 Assignment Tracking
- **Create and manage assignments** with title, subject, and due dates
- **Track submissions** per student with submission status
- **Quality assessment** - rate work quality (Excellent, Good, Average, Poor)
- **Subject categorization** - organize by subject areas
- **Due date tracking** - monitor deadlines and late submissions
- **Per-student submission details** with quality ratings

### 📅 Attendance Management
- **Daily attendance tracking** - mark Present/Absent for each student
- **Date-based views** - review attendance by specific dates
- **Class-wise attendance** - separate records per class
- **Attendance history** - view past attendance records
- **Bulk marking** - efficient attendance entry

### 📊 Reports & Analytics
- **Student performance reports**
- **Assignment submission statistics**
- **Attendance summaries** by student and class
- **Export capabilities** (planned)

### 💾 Data Management
- **Offline operation** - all data stored locally in SQLite
- **Automatic database initialization**
- **Data persistence** across sessions
- **Portable deployment** - run from USB drive or any location

### 🎨 User Interface
- **Clean modern GUI** built with GTK 3
- **Responsive layout** that adapts to window size
- **Intuitive navigation** with clear menu structure
- **Real-time updates** - UI refreshes automatically
- **Custom application icon** - branded identity

## Technology Stack

- **Language**: C (C11 standard)
- **GUI Framework**: GTK 3
- **Database**: SQLite 3
- **Security**: OpenSSL (SHA-256 hashing)

## Prerequisites

Before building this project, you need to install the following dependencies:

### Windows (MSYS2/MinGW)

1. Install MSYS2 from https://www.msys2.org/

2. Open MSYS2 MinGW 64-bit terminal and run:
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-sqlite3
pacman -S mingw-w64-x86_64-openssl
pacman -S mingw-w64-x86_64-pkg-config
pacman -S make
```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libgtk-3-dev
sudo apt-get install libsqlite3-dev
sudo apt-get install libssl-dev
sudo apt-get install pkg-config
```

### Linux (Fedora/RHEL)

```bash
sudo dnf install gcc make
sudo dnf install gtk3-devel
sudo dnf install sqlite-devel
sudo dnf install openssl-devel
sudo dnf install pkg-config
```

## Building the Project

1. Clone or extract the project to your desired location

2. Navigate to the project directory:
```bash
cd "C Final Prokect"
```

3. Build the project:
```bash
make
```

4. Run the application:
```bash
make run
```

Or run directly:
```bash
./build/assignment_tracker.exe  # Windows
./build/assignment_tracker      # Linux
```

## Project Structure

```
C Final Prokect/
├── include/           # Header files
│   ├── common.h       # Common definitions and utilities
│   ├── database.h     # Database operations
│   ├── security.h     # Password hashing and security
│   ├── validation.h   # Input validation functions
│   ├── ui_login.h     # Login screen
│   ├── ui_dashboard.h # Main dashboard
│   ├── ui_students.h  # Student management
│   ├── ui_assignments.h # Assignment tracking
│   ├── ui_attendance.h  # Attendance management
│   └── ui_reports.h   # Reports and analytics
├── src/               # Source files
│   ├── main.c         # Application entry point
│   ├── common.c       # Common utilities implementation
│   ├── database.c     # Database operations
│   ├── security.c     # Security functions
│   ├── validation.c   # Validation functions
│   ├── ui_login.c     # Login UI
│   ├── ui_dashboard.c # Dashboard UI
│   ├── ui_students.c  # Student management UI
│   ├── ui_assignments.c # Assignment tracking UI
│   ├── ui_attendance.c  # Attendance UI
│   └── ui_reports.c   # Reports UI
├── build/             # Compiled output
├── data/              # Database storage
│   └── assignment_tracker.db # SQLite database (created on first run)
├── Makefile           # Build configuration
└── README.md          # This file
```

## 🚀 Quick Start

### Portable Version (Windows)

1. Download or extract the `Aakalan` folder
2. Double-click `Start.bat` or `Aakalan.exe`
3. No installation required - runs immediately!

### From Source

1. Build the project: `make`
2. Run: `make run` or `./build/assignment_tracker`

---

## 📖 Usage Guide

### 🔑 First Time Setup

1. **Launch** the application
2. Click **"Don't have an account? Register"**
3. Fill in your details:
   - Name (minimum 3 characters)
   - Valid email address
   - Strong password (8+ characters, uppercase, lowercase, digit, special character)
4. Click **"Register"**
5. **Login** with your newly created credentials

### 👨‍🎓 Managing Students

1. From the dashboard, click **"Student Management"**
2. **Add new student**:
   - Enter Roll Number (unique identifier)
   - Enter student Name
   - Enter Email address
   - Select or enter Class name
   - Click **"Add Student"**
3. **Edit existing student**:
   - Click on a student in the list
   - Details populate in the form
   - Modify fields as needed
   - Click **"Update Student"**
4. **Delete student**: Select student → Click **"Delete Student"**
5. **Search**: Use the search box to filter students by name/roll

### 📝 Creating & Managing Assignments

1. From the dashboard, click **"Assignment Tracking"**
2. **Create new assignment**:
   - Enter Assignment Title
   - Enter Subject name
   - Select Due Date from calendar
   - Click **"Create Assignment"**
3. **Track submissions**:
   - Select an assignment from the list
   - View all students and their submission status
   - Mark submissions with quality ratings:
     - ✅ **Excellent** - Outstanding work
     - 👍 **Good** - Above average quality
     - 📝 **Average** - Meets requirements
     - ❌ **Poor** - Needs improvement
4. **Monitor deadlines**: Assignments show due dates for tracking

### ✅ Marking Attendance

1. From the dashboard, click **"Attendance Management"**
2. **Select date** from the calendar picker
3. **Mark attendance** for each student:
   - Click **"Present"** for attending students
   - Click **"Absent"** for non-attending students
4. **Review past attendance**:
   - Change the date to view historical records
   - Attendance automatically loads for selected date
5. **Class filtering**: Attendance is tracked per class

## 🔒 Security Features

- **🔐 Password Security**:
  - SHA-256 hashing algorithm
  - Unique random salt per user
  - Passwords never stored in plain text
  - Minimum complexity requirements enforced
  
- **🛡️ SQL Injection Prevention**:
  - All database queries use prepared statements
  - Parameterized inputs for all user data
  - No direct string concatenation in queries
  
- **✅ Input Validation**:
  - Email format validation with regex
  - Password strength requirements (8+ chars, mixed case, numbers, special chars)
  - Name length validation (3-50 characters)
  - Roll number uniqueness checks
  - SQL-safe character filtering
  
- **👤 Session Management**:
  - Secure user session context
  - User ID propagated throughout application
  - Automatic logout on window close

---

## 🗄️ Database Schema

### Tables

**users**
- `id` (INTEGER PRIMARY KEY) - Unique user identifier
- `name` (TEXT NOT NULL) - User's full name
- `email` (TEXT UNIQUE NOT NULL) - Login email
- `password_hash` (TEXT NOT NULL) - SHA-256 hashed password
- `salt` (TEXT NOT NULL) - Random salt for hashing
- `created_at` (DATETIME DEFAULT CURRENT_TIMESTAMP) - Account creation time

**students**
- `id` (INTEGER PRIMARY KEY) - Student ID
- `user_id` (INTEGER) - Foreign key to users table
- `roll_number` (TEXT NOT NULL) - Student roll number
- `name` (TEXT NOT NULL) - Student name
- `email` (TEXT) - Student email
- `class` (TEXT) - Class/section name
- `created_at` (DATETIME) - Record creation timestamp

**assignments**
- `id` (INTEGER PRIMARY KEY) - Assignment ID
- `user_id` (INTEGER) - Foreign key to users table
- `title` (TEXT NOT NULL) - Assignment title
- `subject` (TEXT NOT NULL) - Subject area
- `due_date` (DATE NOT NULL) - Submission deadline
- `created_at` (DATETIME) - Assignment creation time

**assignment_submissions**
- `id` (INTEGER PRIMARY KEY) - Submission ID
- `assignment_id` (INTEGER) - Foreign key to assignments
- `student_id` (INTEGER) - Foreign key to students
- `submission_status` (TEXT) - Submitted/Not Submitted
- `quality` (TEXT) - Excellent/Good/Average/Poor
- `submitted_at` (DATETIME) - Submission timestamp

**attendance**
- `id` (INTEGER PRIMARY KEY) - Attendance record ID
- `student_id` (INTEGER) - Foreign key to students
- `date` (DATE NOT NULL) - Attendance date
- `status` (TEXT NOT NULL) - Present/Absent
- `class` (TEXT) - Class name
- `marked_at` (DATETIME) - Record creation time

---

## Troubleshooting

### Build Errors

**Error: gtk/gtk.h not found**
- Make sure GTK 3 development packages are installed
- Verify pkg-config is working: `pkg-config --cflags gtk+-3.0`

**Error: sqlite3.h not found**
- Install SQLite development packages
- Check with: `pkg-config --libs sqlite3`

**Error: openssl/sha.h not found**
- Install OpenSSL development packages

### Runtime Errors

**Database initialization failed**
- Ensure the `data/` directory exists
- Check write permissions

**Window doesn't appear**
- Verify GTK runtime libraries are in your PATH
- On Windows, make sure MSYS2 MinGW bin directory is in PATH

## 🔧 Development Notes

### Architecture

- **Modular Design**: Clear separation of concerns across modules
- **MVC-like Pattern**: UI, business logic, and data access separated
- **Event-Driven**: GTK signal/callback architecture
- **Database Abstraction**: Centralized database operations

### Code Organization

- **Header Files** (`include/`): Interface definitions and declarations
- **Source Files** (`src/`): Implementation of all modules
- **Single Responsibility**: Each module handles one domain area
- **Reusable Components**: Common utilities shared across modules

### Best Practices Applied

- ✅ Const-correctness for function parameters
- ✅ Proper memory management with cleanup functions
- ✅ Error handling for all database/file operations
- ✅ Input validation before processing
- ✅ Prepared statements for SQL queries
- ✅ Meaningful variable and function names
- ✅ Comprehensive comments and documentation

---

## 🚧 Future Enhancements

- [ ] **Advanced Reports**: Graphical charts and visualizations
- [ ] **Export Functionality**: CSV, PDF, Excel export
- [ ] **Data Import**: Bulk student import from files
- [ ] **Backup/Restore**: Database backup and recovery
- [ ] **User Roles**: Admin, Instructor, Student access levels
- [ ] **Notifications**: Due date reminders and alerts
- [ ] **Dark Mode**: Theme switching support
- [ ] **Multi-language**: Internationalization support
- [ ] **Cloud Sync**: Optional cloud backup (future version)
- [ ] **Mobile App**: Companion mobile application

---

## 📦 Portable Package

The Windows portable version includes:
- **Aakalan.exe** - Main executable with embedded icon
- **97 DLL files** - All GTK, SQLite, OpenSSL dependencies
- **Start.bat** - Launcher script with PATH configuration
- **data/** - Database storage directory
- **README.txt** - Quick reference guide

**Package Size**: ~100 MB
**Requirements**: Windows 10/11 64-bit
**Installation**: None required - extract and run!

---

## 📄 License

This is an academic project developed for educational purposes.

---

## 👨‍💻 Author

**Developed as a comprehensive C programming project demonstrating:**

- ✨ Professional GUI development with GTK 3
- 🗄️ Database integration and design with SQLite
- 🔒 Security implementation (hashing, validation, SQL injection prevention)
- 🏗️ Modular software architecture and design patterns
- ✅ Input validation and comprehensive error handling
- 📦 Cross-platform development (Windows/Linux)
- 🎯 Real-world application development from concept to deployment

---

<div align="center">
  
  **Built with ❤️ using C, GTK 3, SQLite, and OpenSSL**
  
  *Aakalan - Empowering Education Through Technology*

</div>
