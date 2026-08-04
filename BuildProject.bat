@echo off
setlocal

if not exist vcpkg\vcpkg.exe (
    echo Bootstrapping vcpkg...
    call vcpkg\bootstrap-vcpkg.bat
)

echo Configuring...
cmake --preset default

if errorlevel 1 exit /b 1

echo Building...
cmake --build build

echo Done!
pause