@echo off
setlocal enabledelayedexpansion

REM ------------------------------------------------------------------------
REM 0) Ensure CMake is available (install and add to PATH if missing)
REM ------------------------------------------------------------------------
call :FindCMake
if not defined CMAKE_EXE (
  echo [STEP] CMake not found. Attempting install via winget...
  call :InstallCMakeWinget
  call :FindCMake
)
if not defined CMAKE_EXE (
  echo [WARN] winget path failed or not available. Trying Chocolatey...
  call :InstallCMakeChoco
  call :FindCMake
)
if not defined CMAKE_EXE (
  echo [WARN] Could not install via package manager. Trying Visual Studio bundled CMake...
  call :FindVSCMake
)
if not defined CMAKE_EXE (
  echo [ERROR] CMake still not found. Please install CMake and re-run.
  pause & exit /b 1
)

REM Put CMake on PATH for *this session* so the rest of the script can use it
set "PATH=%CMAKE_DIR%;%PATH%"
echo [INFO] Using CMake: "%CMAKE_EXE%"
"%CMAKE_EXE%" --version || (
  echo [ERROR] CMake invocation failed even after detection.
  pause & exit /b 1
)

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
  REM Fallback: if conan is already on PATH from a system/user install, use that
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

REM Now that conan exists, optionally pin CMake path inside Conan
conan config set tools.cmake.cmake_program="%CMAKE_EXE%" >nul 2>&1

REM ------------------------------------------------------------------------
REM 5) Detect default profile
REM ------------------------------------------------------------------------
echo [STEP] Detecting Conan default profile...
conan profile detect --force
if errorlevel 1 (
  echo [ERROR] Conan profile detect failed.
  pause & exit /b 1
)

REM ------------------------------------------------------------------------
REM 5.5) Ensure our custom msvc17 profile exists
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
) > profiles\msvc17

REM ------------------------------------------------------------------------
REM 6) Install Debug and Release property sheets
REM ------------------------------------------------------------------------
echo [STEP] Installing Conan packages for Release...
conan install . -of conanbuild\Release -pr:h profiles\msvc17 -pr:b profiles\msvc17 ^
  -s build_type=Release -g CMakeDeps -g CMakeToolchain --build=missing

echo [STEP] Installing Conan packages for Debug...
conan install . -of conanbuild\Debug -pr:h profiles\msvc17 -pr:b profiles\msvc17 ^
  -s build_type=Debug -g CMakeDeps -g CMakeToolchain --build=missing


echo.
echo ==== Done! Props are inside conanbuild/ ====
pause
exit /b 0

REM ===================== helper routines =====================

:FindCMake
REM Try PATH
for /f "usebackq delims=" %%p in (`where cmake 2^>nul`) do (
  set "CMAKE_EXE=%%p"
)
if defined CMAKE_EXE (
  for %%d in ("%CMAKE_EXE%") do set "CMAKE_DIR=%%~dpd"
  goto :eof
)

REM Common install path
if exist "C:\Program Files\CMake\bin\cmake.exe" (
  set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
  set "CMAKE_DIR=C:\Program Files\CMake\bin"
  goto :eof
)

REM User install path (winget sometimes installs here)
if exist "%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe" (
  set "CMAKE_EXE=%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe"
  set "CMAKE_DIR=%LOCALAPPDATA%\Programs\CMake\bin"
  goto :eof
)

set "CMAKE_EXE="
set "CMAKE_DIR="
goto :eof

:InstallCMakeWinget
where winget >nul 2>&1 || goto :eof
echo [STEP] Installing CMake with winget (silent)...
winget install -e --id Kitware.CMake --silent --accept-source-agreements --accept-package-agreements
goto :eof

:InstallCMakeChoco
where choco >nul 2>&1 || goto :eof
echo [STEP] Installing CMake with Chocolatey...
choco install cmake -y --no-progress --installargs "'ADD_CMAKE_TO_PATH=System'"
goto :eof

:FindVSCMake
REM Use vswhere to locate Visual Studio’s bundled CMake
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :eof
for /f "usebackq delims=" %%p in (`
  "%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild ^
   -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
`) do (
  if exist "%%p" (
    set "CMAKE_EXE=%%p"
    for %%d in ("%%p") do set "CMAKE_DIR=%%~dpd"
  )
)
goto :eof
