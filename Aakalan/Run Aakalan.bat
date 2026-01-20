@echo off
title Aakalan - Student Management System

REM Set current directory as base
set APP_DIR=%~dp0

REM Run the application
"%APP_DIR%Aakalan.exe"

REM Exit code handling
if errorlevel 1 (
    echo.
    echo Application exited with error code %errorlevel%
    echo Press any key to close...
    pause >nul
)
