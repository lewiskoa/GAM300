@echo off
setlocal enabledelayedexpansion

REM ------------------------------------------------------------------------
REM 0) Ensure CMake is available (install only if missing)
REM ------------------------------------------------------------------------
call :FindCMake
if defined CMAKE_EXE (
    echo [INFO] CMake found: "%CMAKE_EXE%"
    goto :cmake_ready
)

echo [INFO] CMake not found. Installing...
call :InstallCMake
if not defined CMAKE_EXE (
    echo [ERROR] Failed to install CMake. Please install manually and re-run.
    pause & exit /b 1
)

:cmake_ready
REM Put CMake on PATH for *this session*
set "PATH=%CMAKE_DIR%;%PATH%"
echo [INFO] CMake ready: "%CMAKE_EXE%"

REM Quick version check to ensure it works
"%CMAKE_EXE%" --version >nul 2>&1 || (
    echo [ERROR] CMake installed but not working properly.
    pause & exit /b 1
)

REM ------------------------------------------------------------------------
REM 1) Ensure Python is available
REM ------------------------------------------------------------------------
python --version >nul 2>&1 || (
    echo [ERROR] Python 3.8+ not found. Please install Python and add to PATH.
    pause & exit /b 1
)
echo [INFO] Python found: & python --version

REM ------------------------------------------------------------------------
REM 2) Install/upgrade pip tools (only if needed)
REM ------------------------------------------------------------------------
echo [STEP] Checking pip, setuptools, wheel...
python -c "import pip, setuptools, wheel" >nul 2>&1 || (
    echo [STEP] Upgrading pip tooling...
    python -m pip install --upgrade pip setuptools wheel --quiet
)

REM ------------------------------------------------------------------------
REM 3) Ensure Conan is installed (install only if missing)
REM ------------------------------------------------------------------------
where conan >nul 2>&1
if errorlevel 1 (
    echo [STEP] Installing Conan...
    python -m pip install --user conan --quiet
    if errorlevel 1 (
        echo [ERROR] Failed to install Conan.
        pause & exit /b 1
    )
) else (
    echo [INFO] Conan already available.
)

REM ------------------------------------------------------------------------
REM 4) Locate conan.exe and add to PATH
REM ------------------------------------------------------------------------
call :FindConan
if not defined CONAN_EXE (
    echo [ERROR] Conan installed but not found.
    pause & exit /b 1
)

set "PATH=%CONAN_DIR%;%PATH%"
echo [INFO] Conan ready: "%CONAN_EXE%"

REM Configure CMake path in Conan (suppress output)
conan config set tools.cmake.cmake_program="%CMAKE_EXE%" >nul 2>&1

REM ------------------------------------------------------------------------
REM 5) Setup Conan profile
REM ------------------------------------------------------------------------
echo [STEP] Setting up Conan profile...
conan profile detect --force >nul 2>&1
if errorlevel 1 (
    echo [WARN] Conan profile detect failed, continuing...
)

REM Ensure custom profile directory exists
if not exist profiles mkdir profiles

REM Create optimized MSVC profile
(
    echo [settings]
    echo os=Windows
    echo arch=x86_64
    echo compiler=msvc
    echo compiler.version=193
    echo compiler.runtime=dynamic
    echo compiler.cppstd=17
    echo build_type=Release
) > profiles\msvc17

REM ---- FMOD Studio API in repo root ----
for %%i in ("%~dp0..") do set "REPO_ROOT=%%~fi"
set "FMOD_DIR=%REPO_ROOT%\FMOD Studio API Windows\api"

if not exist "%FMOD_DIR%\core\lib\x64\fmod_vc.lib" (
  echo [ERROR] FMOD API not found at: "%FMOD_DIR%"
  echo         Expect: core\inc, core\lib\x64\fmod_vc.lib, studio\inc, studio\lib\x64\fmodstudio_vc.lib
  pause & exit /b 1
)
echo [INFO] FMOD_DIR = "%FMOD_DIR%"

REM ------------------------------------------------------------------------
REM 6) Install dependencies for both configurations
REM ------------------------------------------------------------------------
echo [STEP] Installing dependencies for Release...
conan install . -of conanbuild\Release -pr:h profiles\msvc17 -pr:b profiles\msvc17 ^
  -s build_type=Release -g MSBuildDeps --build=missing ^
  -o glfw/*:shared=True -o glew/*:shared=True

echo [STEP] Installing dependencies for Debug...
conan install . -of conanbuild\Debug -pr:h profiles\msvc17 -pr:b profiles\msvc17 ^
  -s build_type=Debug -g MSBuildDeps --build=missing ^
  -o glfw/*:shared=True -o glew/*:shared=True

echo.
echo ==== Setup Complete! Dependencies are in conanbuild/ ====
pause
exit /b 0

REM ===================== Helper Functions =====================

:FindCMake
REM Check if already in PATH
for /f "usebackq delims=" %%p in (`where cmake 2^>nul`) do (
    set "CMAKE_EXE=%%p"
    for %%d in ("%%p") do set "CMAKE_DIR=%%~dpd"
    goto :eof
)

REM Check common installation paths
set "PATHS[0]=C:\Program Files\CMake\bin\cmake.exe"
set "PATHS[1]=%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe"

for /l %%i in (0,1,1) do (
    if defined PATHS[%%i] if exist "!PATHS[%%i]!" (
        set "CMAKE_EXE=!PATHS[%%i]!"
        for %%d in ("!PATHS[%%i]!") do set "CMAKE_DIR=%%~dpd"
        goto :eof
    )
)

REM Try Visual Studio bundled CMake as last resort
call :FindVSCMake
goto :eof

:InstallCMake
REM Try winget first (most reliable and lightweight)
where winget >nul 2>&1 && (
    echo [STEP] Installing CMake with winget...
    winget install -e --id Kitware.CMake --silent --accept-source-agreements --accept-package-agreements >nul 2>&1
    call :FindCMake
    if defined CMAKE_EXE goto :eof
)

REM Fallback to Chocolatey
where choco >nul 2>&1 && (
    echo [STEP] Installing CMake with Chocolatey...
    choco install cmake -y --no-progress --limit-output >nul 2>&1
    call :FindCMake
    if defined CMAKE_EXE goto :eof
)

REM Last resort: try Visual Studio bundled version
call :FindVSCMake
goto :eof

:FindVSCMake
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :eof

for /f "usebackq delims=" %%p in (`
    "%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild ^
    -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe 2^>nul
`) do (
    if exist "%%p" (
        set "CMAKE_EXE=%%p"
        for %%d in ("%%p") do set "CMAKE_DIR=%%~dpd"
        goto :eof
    )
)
goto :eof

:FindConan
REM Check if already in PATH
for /f "usebackq delims=" %%p in (`where conan 2^>nul`) do (
    set "CONAN_EXE=%%p"
    for %%d in ("%%p") do set "CONAN_DIR=%%~dpd"
    goto :eof
)

REM Check Python user scripts directory
for /f "usebackq delims=" %%i in (`python -c "import sysconfig; print(sysconfig.get_path('scripts','nt_user'))" 2^>nul`) do (
    if exist "%%i\conan.exe" (
        set "CONAN_EXE=%%i\conan.exe"
        set "CONAN_DIR=%%i"
        goto :eof
    )
)

REM Check common Python installation paths
for /d %%d in ("%LOCALAPPDATA%\Programs\Python\*") do (
    if exist "%%d\Scripts\conan.exe" (
        set "CONAN_EXE=%%d\Scripts\conan.exe"
        set "CONAN_DIR=%%d\Scripts"
        goto :eof
    )
)

set "CONAN_EXE="
set "CONAN_DIR="
goto :eof