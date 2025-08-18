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

REM Optional: Tell Conan exactly which cmake.exe to use (avoids PATH issues)
conan config set tools.cmake.cmake_program="%CMAKE_EXE%" >nul 2>&1

REM ------------------------------------------------------------------------
REM 1) Ensure Python is available
REM ------------------------------------------------------------------------
where python >nul 2>&1 || (
  echo [ERROR] Python 3.8+ not found in PATH. Please install and add to PATH.
  pause & exit /b 1
)

REM ------------------------------------------------------------------------
REM 2) Upgrade pip and install Conan
REM ------------------------------------------------------------------------
echo [STEP] Installing/upgrading pip, setuptools, wheel...
python -m pip install --upgrade pip setuptools wheel

echo [STEP] Installing/upgrading Conan...
python -m pip install --user --upgrade conan
if errorlevel 1 (
  echo [ERROR] pip install conan failed.
  pause & exit /b 1
)

REM ------------------------------------------------------------------------
REM 3) Try to find conan.exe in common paths
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
  echo [ERROR] conan.exe not found in user scripts folders.
  pause & exit /b 1
)

set "PATH=%CONAN_PATH%;%PATH%"
echo [INFO] Using Conan from: %CONAN_PATH%\conan.exe

REM ------------------------------------------------------------------------
REM 4) Detect default profile
REM ------------------------------------------------------------------------
echo [STEP] Detecting Conan default profile...
conan profile detect --force
if errorlevel 1 (
  echo [ERROR] Conan profile detect failed.
  pause & exit /b 1
)

REM ------------------------------------------------------------------------
REM 5 & 6) Install Debug and Release property sheets
REM ------------------------------------------------------------------------
echo [STEP] Installing Conan packages for Release...
conan install . --output-folder=conanbuild --build=missing -g MSBuildDeps ^
             -s build_type=Release -s arch=x86_64

echo [STEP] Installing Conan packages for Debug...
conan install . --output-folder=conanbuild --build=missing -g MSBuildDeps ^
             -s build_type=Debug   -s arch=x86_64

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
REM No guarantee PATH is updated in this session, so we’ll re-detect afterward.
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
