@echo off
setlocal enabledelayedexpansion

REM ------------------------------------------------------------------------
REM 1) Ensure Python is available
REM ------------------------------------------------------------------------
where python >nul 2>&1 || (
  echo [ERROR] Python 3.8+ not found in PATH. Please install and add to PATH.
  pause & exit /b 1
)

REM ------------------------------------------------------------------------
REM 2) Make sure pip tooling is current
REM ------------------------------------------------------------------------
echo [STEP] Installing/upgrading pip, setuptools, wheel...
python -m pip install --upgrade pip setuptools wheel

REM ------------------------------------------------------------------------
REM 3) Ensure Conan is installed (install only if missing)
REM ------------------------------------------------------------------------
where conan >nul 2>&1
if errorlevel 1 (
  echo [STEP] Conan not found. Installing with pip...
  python -m pip install --user conan
  if errorlevel 1 (
    echo [ERROR] pip install conan failed.
    pause & exit /b 1
  )
) else (
  echo [INFO] Conan already available in PATH.
)

REM (Optional) Clear any half-built bzip2 to avoid stale errors
conan remove "bzip2/*" -f

REM ------------------------------------------------------------------------
REM 4) Locate conan.exe and put it on PATH for this session
REM ------------------------------------------------------------------------
set "CONAN_PATH="

for /f "usebackq delims=" %%i in (`python -c "import sysconfig; print(sysconfig.get_path('scripts','nt_user'))"`) do (
    if exist "%%i\conan.exe" set "CONAN_PATH=%%i"
)

if not defined CONAN_PATH (
  for /d %%d in ("%LOCALAPPDATA%\Programs\Python\*") do (
    if exist "%%d\Scripts\conan.exe" (
      set "CONAN_PATH=%%d\Scripts"
      goto :found_conan
    )
  )
)

:found_conan
if not defined CONAN_PATH (
  for /f "usebackq delims=" %%p in (`where conan 2^>nul`) do (
    for %%d in ("%%p") do set "CONAN_PATH=%%~dpd"
  )
)

if not defined CONAN_PATH (
  echo [ERROR] conan.exe not found after installation.
  pause & exit /b 1
)

set "PATH=%CONAN_PATH%;%PATH%"
echo [INFO] Using Conan from: "%CONAN_PATH%\conan.exe"

REM ------------------------------------------------------------------------
REM 5) Detect default profile (not used directly, but helpful on clean boxes)
REM ------------------------------------------------------------------------
echo [STEP] Detecting Conan default profile...
conan profile detect --force
if errorlevel 1 (
  echo [ERROR] Conan profile detect failed.
  pause & exit /b 1
)

REM ------------------------------------------------------------------------
REM 5.5) Create build profile WITH cmake tool requirement
REM ------------------------------------------------------------------------
if not exist profiles mkdir profiles
(
  echo [settings]
  echo os=Windows
  echo arch=x86_64
  echo compiler=msvc
  echo compiler.version=193
  echo compiler.runtime=dynamic
  echo compiler.cppstd=17
  echo build_type=Release

  echo.
  echo [tool_requires]
  echo cmake/[^>=3.25]

  echo.
  echo [conf]
  echo tools.cmake.cmaketoolchain:generator="Visual Studio 17 2022"
) > profiles\msvc17

REM Create a host profile without tool_requires to avoid conflicts
(
  echo [settings]
  echo os=Windows
  echo arch=x86_64
  echo compiler=msvc
  echo compiler.version=193
  echo compiler.runtime=dynamic
  echo compiler.cppstd=17
  echo build_type=Release

  echo.
  echo [conf]
  echo tools.cmake.cmaketoolchain:generator="Visual Studio 17 2022"
) > profiles\msvc17_host

REM Make sure the ConanCenter remote exists so cmake can be fetched
conan remote list | findstr /I "conancenter" >nul || conan remote add conancenter https://center.conan.io

REM ------------------------------------------------------------------------
REM 6) Install build requirements first (including CMake)
REM ------------------------------------------------------------------------
echo [STEP] Installing CMake and other build tools...
conan install --tool-requires=cmake/[^>=3.25] -pr:b profiles\msvc17 --build=missing

REM ------------------------------------------------------------------------
REM 7) Install Debug and Release (MSBuildDeps only; project stays MSBuild)
REM ------------------------------------------------------------------------
echo [STEP] Installing Conan packages for Release (MSBuildDeps)...
conan install . -of conanbuild\Release -pr:h profiles\msvc17_host -pr:b profiles\msvc17 ^
  -s build_type=Release -g MSBuildDeps --build=missing

echo [STEP] Installing Conan packages for Debug (MSBuildDeps)...
conan install . -of conanbuild\Debug -pr:h profiles\msvc17_host -pr:b profiles\msvc17 ^
  -s build_type=Debug -g MSBuildDeps --build=missing

echo.
echo ==== Done! Props are inside conanbuild\^<Config^>\conandeps.props ====
pause
exit /b 0