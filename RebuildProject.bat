@echo off
echo ============================================
echo VibeCoding Project Rebuild Script
echo ============================================
echo.

cd /d "%~dp0"

echo Step 1: Finding .uproject file...
for %%f in (*.uproject) do set UPROJECT=%%f

if "%UPROJECT%"=="" (
    echo ERROR: No .uproject file found!
    pause
    exit /b 1
)

echo Found: %UPROJECT%
echo.

echo Step 2: Locating Unreal Engine...
set UVS="C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"

if not exist %UVS% (
    echo ERROR: UnrealVersionSelector not found at default location!
    echo.
    echo Please do this manually:
    echo 1. Right-click on %UPROJECT%
    echo 2. Select "Generate Visual Studio project files"
    echo 3. Open the generated .sln file
    echo 4. Press Ctrl+Shift+B to build
    pause
    exit /b 1
)

echo Step 3: Generating Visual Studio project files...
%UVS% /projectfiles "%CD%\%UPROJECT%"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to generate project files!
    pause
    exit /b 1
)

echo.
echo ============================================
echo SUCCESS! Project files generated.
echo ============================================
echo.

echo Step 4: Looking for Visual Studio solution...
for %%f in (*.sln) do set SOLUTION=%%f

if "%SOLUTION%"=="" (
    echo WARNING: No .sln file found!
    echo Please open the project manually.
    pause
    exit /b 0
)

echo Found: %SOLUTION%
echo.

echo Step 5: Building the solution...
echo This may take several minutes...
echo.

REM Try to find MSBuild
set MSBUILD=""
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)

if %MSBUILD%=="" (
    echo MSBuild not found automatically.
    echo.
    echo Please build manually:
    echo 1. Open %SOLUTION% in Visual Studio
    echo 2. Press Ctrl+Shift+B to build
    echo 3. Wait for build to complete
    echo 4. Then open Unreal Editor
    echo.
    start "" "%SOLUTION%"
    pause
    exit /b 0
)

echo Using MSBuild at: %MSBUILD%
%MSBUILD% "%SOLUTION%" /t:Build /p:Configuration="Development Editor" /p:Platform=Win64 /m

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo BUILD SUCCESSFUL!
    echo ============================================
    echo.
    echo You can now open the project in Unreal Editor:
    echo - Double-click %UPROJECT%
    echo.
) else (
    echo.
    echo ============================================
    echo BUILD FAILED!
    echo ============================================
    echo.
    echo Please check the errors above and:
    echo 1. Fix any compilation errors
    echo 2. Run this script again
    echo.
    echo Or open %SOLUTION% in Visual Studio for detailed errors.
    echo.
)

pause
