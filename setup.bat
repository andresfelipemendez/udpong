@echo off
setlocal enabledelayedexpansion

echo === UDP Pong Setup ===
echo.

:: Check for git
where git >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: Git not found. Please install Git.
    exit /b 1
)

:: Check for cmake
where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: CMake not found. Please install CMake.
    exit /b 1
)

:: Initialize and update submodules
echo Initializing Git submodules...
git submodule update --init --recursive
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to initialize submodules.
    exit /b 1
)

:: Configuration
set DEPS_DIR=deps

echo.
echo Building SDL3 dependencies...
echo This may take several minutes on first run.
echo.

:: Build SDL3
call :build_dep "%DEPS_DIR%\SDL3" "SDL3"
call :build_dep "%DEPS_DIR%\SDL3_image" "SDL3_image"
call :build_dep "%DEPS_DIR%\SDL3_ttf" "SDL3_ttf"
call :build_dep "%DEPS_DIR%\SDL3_mixer" "SDL3_mixer"
call :build_dep "%DEPS_DIR%\SDL3_net" "SDL3_net"

echo.
echo === Setup Complete ===
echo.
echo To build the game:
echo   build.bat         (from VS Developer Command Prompt)
echo.
echo To start the Nakama server:
echo   cd nakama ^&^& docker-compose up -d
echo.
echo To run the game:
echo   client.exe

endlocal
exit /b 0

:: Function to build a dependency
:build_dep
set dep_dir=%~1
set dep_name=%~2

if not exist "%dep_dir%" (
    echo Warning: %dep_name% not found at %dep_dir%
    exit /b 1
)

if exist "%dep_dir%\build" (
    echo %dep_name% already built, skipping...
    exit /b 0
)

echo.
echo Building %dep_name%...
cmake -S "%dep_dir%" -B "%dep_dir%\build" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to configure %dep_name%
    exit /b 1
)

cmake --build "%dep_dir%\build" --config Release --parallel
if %ERRORLEVEL% neq 0 (
    echo Error: Failed to build %dep_name%
    exit /b 1
)

echo %dep_name% built successfully!
exit /b 0
