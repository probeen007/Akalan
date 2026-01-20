# Quick Start Guide

## For Windows Users

### Prerequisites Check
Open MSYS2 MinGW 64-bit terminal and verify:
```bash
gcc --version
pkg-config --version
pkg-config --libs gtk+-3.0
pkg-config --libs sqlite3
```

### Build and Run (3 Simple Steps)

1. **Navigate to project**
```bash
cd "/d/Professional/C Final Prokect"
```

2. **Build**
```bash
make
```

3. **Run**
```bash
make run
```

### First Time Use

1. **Register Account**
   - Click "Don't have an account? Register"
   - Enter your name, email, and password
   - Click "Register"

2. **Login**
   - Enter your email and password
   - Click "Login"

3. **Add Students**
   - Click "Student Management" from dashboard
   - Fill in student details
   - Click "Add"

4. **Explore Other Modules**
   - Assignment Tracking (coming soon)
   - Attendance Management (coming soon)
   - Reports (coming soon)

## Keyboard Shortcuts

- **Tab**: Move between fields
- **Enter**: Submit forms (in most dialogs)
- **Esc**: Close dialogs

## Tips

- All data is saved automatically to `data/assignment_tracker.db`
- You can backup this file to preserve your data
- Email and roll numbers must be unique per student
- Passwords must be at least 6 characters

## Need Help?

- Read [README.md](README.md) for detailed documentation
- Check [BUILD_WINDOWS.md](BUILD_WINDOWS.md) for setup help
- Review error messages in dialogs for specific issues

## Common Issues

**"Database initialization failed"**
- The `data/` folder will be created automatically
- Check you have write permissions

**"Invalid email or password"**
- Email format must be valid (user@domain.com)
- Password is case-sensitive

**"Email or roll number already exists"**
- Each student must have unique email and roll number
- Check existing students before adding

## Development

To rebuild after code changes:
```bash
make clean
make
```

To see all make targets:
```bash
make help
```
