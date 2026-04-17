@echo off
echo ============================================
echo Code Counter - Compilation Script
echo ============================================
echo.

:: Add MSYS2 UCRT64 to PATH
if exist "C:\msys64\ucrt64\bin\gcc.exe" goto compile

:: Check other compilers
where gcc >nul 2>&1
if %errorlevel% equ 0 goto compile

where clang >nul 2>&1
if %errorlevel% equ 0 goto compile

if exist "C:\msys64\mingw64\bin\gcc.exe" goto compile
if exist "C:\MinGW\bin\gcc.exe" goto compile
where cl >nul 2>&1
if %errorlevel% equ 0 goto compile_msvc

echo [ERROR] No C compiler found
echo Please install MSYS2, MinGW, or MSVC
exit /b 1

:compile
set PATH=C:\msys64\ucrt64\bin;%PATH%
set SOURCE=main.c
set OUTPUT=ccount.exe

echo [INFO] Using MSYS2 UCRT64 GCC
echo [INFO] Source: %SOURCE%
echo.

gcc -O2 -Wall -Wextra -o "%OUTPUT%" "%SOURCE%"
if %errorlevel% equ 0 (
    echo [SUCCESS] Created: %OUTPUT%
) else (
    echo [ERROR] Compilation failed
    exit /b 1
)
exit /b 0

:compile_msvc
echo [INFO] Using MSVC
set SOURCE=main.c
set OUTPUT=ccount.exe

if exist "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
)
cl /O2 /W4 /Fe:"%OUTPUT%" "%SOURCE%"
if %errorlevel% equ 0 (
    echo [SUCCESS] Created: %OUTPUT%
) else (
    echo [ERROR] Compilation failed
    exit /b 1
)
exit /b 0