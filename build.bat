@echo off
setlocal enabledelayedexpansion

echo Building UDP Pong for Windows...

:: Check for Visual Studio environment
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: Visual Studio compiler not found.
    echo Please run this from a Visual Studio Developer Command Prompt
    echo or run vcvarsall.bat first.
    exit /b 1
)

:: Configuration - use deps/ submodules
set DEPS_DIR=deps
set SDL3_DIR=%DEPS_DIR%\SDL3
set SDL3_IMAGE_DIR=%DEPS_DIR%\SDL3_image
set SDL3_TTF_DIR=%DEPS_DIR%\SDL3_ttf
set SDL3_MIXER_DIR=%DEPS_DIR%\SDL3_mixer
set SDL3_NET_DIR=%DEPS_DIR%\SDL3_net

:: Check if submodules exist
if not exist "%SDL3_DIR%" (
    echo SDL3 submodule not found.
    echo Run: git submodule update --init --recursive
    exit /b 1
)

:: Build dependencies if needed
call :build_dep "%SDL3_DIR%" "SDL3"
call :build_dep "%SDL3_IMAGE_DIR%" "SDL3_image"
call :build_dep "%SDL3_TTF_DIR%" "SDL3_ttf"
call :build_dep "%SDL3_MIXER_DIR%" "SDL3_mixer"
call :build_dep "%SDL3_NET_DIR%" "SDL3_net"

:: Include paths
set INCLUDE_DIRS=/I"%SDL3_DIR%\include"
set INCLUDE_DIRS=%INCLUDE_DIRS% /I"%SDL3_IMAGE_DIR%\include"
set INCLUDE_DIRS=%INCLUDE_DIRS% /I"%SDL3_TTF_DIR%\include"
set INCLUDE_DIRS=%INCLUDE_DIRS% /I"%SDL3_MIXER_DIR%\include"
set INCLUDE_DIRS=%INCLUDE_DIRS% /I"%SDL3_NET_DIR%\include"

:: Library paths
set LIB_DIRS=/LIBPATH:"%SDL3_DIR%\build\Release"
set LIB_DIRS=%LIB_DIRS% /LIBPATH:"%SDL3_IMAGE_DIR%\build\Release"
set LIB_DIRS=%LIB_DIRS% /LIBPATH:"%SDL3_TTF_DIR%\build\Release"
set LIB_DIRS=%LIB_DIRS% /LIBPATH:"%SDL3_MIXER_DIR%\build\Release"
set LIB_DIRS=%LIB_DIRS% /LIBPATH:"%SDL3_NET_DIR%\build\Release"

set LIBS=SDL3.lib SDL3_image.lib SDL3_ttf.lib SDL3_mixer.lib SDL3_net.lib

echo.
echo Compiling client...
cl /nologo /W3 /O2 /MD ^
    %INCLUDE_DIRS% ^
    client.c ^
    /Fe:client.exe ^
    /link %LIB_DIRS% %LIBS% ^
    Shell32.lib User32.lib

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

:: Clean up object files
del /q *.obj 2>nul

:: Copy DLLs
echo.
echo Copying DLLs...
copy /Y "%SDL3_DIR%\build\Release\SDL3.dll" . >nul 2>&1
copy /Y "%SDL3_IMAGE_DIR%\build\Release\SDL3_image.dll" . >nul 2>&1
copy /Y "%SDL3_TTF_DIR%\build\Release\SDL3_ttf.dll" . >nul 2>&1
copy /Y "%SDL3_MIXER_DIR%\build\Release\SDL3_mixer.dll" . >nul 2>&1
copy /Y "%SDL3_NET_DIR%\build\Release\SDL3_net.dll" . >nul 2>&1

echo.
echo Build complete!
echo Run with: client.exe

endlocal
exit /b 0

:: Function to build a dependency
:build_dep
set dep_dir=%~1
set dep_name=%~2
if exist "%dep_dir%" (
    if not exist "%dep_dir%\build" (
        echo.
        echo Building %dep_name%...
        cmake -S "%dep_dir%" -B "%dep_dir%\build" -DCMAKE_BUILD_TYPE=Release
        cmake --build "%dep_dir%\build" --config Release --parallel
    )
)
exit /b 0
