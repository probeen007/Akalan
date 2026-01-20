# Assignment Tracker and Student Attendance Management System

A comprehensive GUI-based offline application built entirely in C for tracking student assignments and attendance.

## Features

- **Secure Login System**: Email/password authentication with SHA-256 hashed passwords
- **Student Management**: Add, edit, delete, and view student records
- **Assignment Tracking**: Create assignments and track submissions with quality assessment
- **Attendance Management**: Mark and review student attendance by date
- **Reports**: Generate analytics and reports (coming soon)
- **Offline Operation**: All data stored locally in SQLite database
- **Clean GUI**: Built with GTK 3 for a modern, user-friendly interface

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

## Usage

### First Time Setup

1. Run the application
2. Click "Don't have an account? Register"
3. Enter your name, email, and password (minimum 6 characters)
4. Click "Register"
5. Login with your credentials

### Managing Students

1. From the dashboard, click "Student Management"
2. Add students by filling the form on the right
3. Click on a student to edit their details
4. Use Update/Delete buttons as needed

### Creating Assignments

1. From the dashboard, click "Assignment Tracking"
2. Create new assignments with title, subject, and due date
3. Track submissions and quality for each student

### Marking Attendance

1. From the dashboard, click "Attendance Management"
2. Select a date
3. Mark students as Present or Absent

## Security Features

- **Password Hashing**: All passwords are hashed using SHA-256 before storage
- **SQL Injection Prevention**: All database queries use prepared statements
- **Input Validation**: Comprehensive validation on all user inputs
- **Session Management**: Secure user session handling

## Database Schema

The application uses the following tables:

- `users`: User accounts with hashed passwords
- `students`: Student records
- `assignments`: Assignment metadata
- `assignment_submissions`: Per-student submission tracking
- `attendance`: Daily attendance records

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

## Development Notes

- The project follows a modular architecture with clear separation of concerns
- All UI operations use GTK 3 widgets
- Database operations are abstracted in the database module
- Input validation is centralized in the validation module
- Security operations are isolated in the security module

## Future Enhancements

- Full implementation of Assignment Tracking with dynamic UI
- Complete Attendance Management module
- Reports and Analytics dashboard
- Export functionality (CSV, PDF)
- Backup and restore features
- Multiple user roles (Admin, Instructor, Student)

## License

This is an academic project developed for educational purposes.

## Author

Developed as a final year C programming project demonstrating:
- GUI programming with GTK
- Database integration with SQLite
- Security best practices
- Modular software architecture
- Input validation and error handling
