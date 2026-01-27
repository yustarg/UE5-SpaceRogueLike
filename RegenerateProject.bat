@echo off
echo ============================================
echo Regenerating Unreal Engine Project Files
echo ============================================
echo.

REM Find the .uproject file
for %%f in (*.uproject) do set UPROJECT=%%f

if "%UPROJECT%"=="" (
    echo ERROR: No .uproject file found!
    pause
    exit /b 1
)

echo Found project: %UPROJECT%
echo.

REM Try to find UnrealVersionSelector
set UVS="C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"

if not exist %UVS% (
    echo ERROR: UnrealVersionSelector not found!
    echo Please right-click on %UPROJECT% and select "Generate Visual Studio project files"
    pause
    exit /b 1
)

echo Generating project files...
%UVS% /projectfiles "%CD%\%UPROJECT%"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo SUCCESS! Project files regenerated.
    echo ============================================
    echo.
    echo Next steps:
    echo 1. Open VibeCoding.sln in Visual Studio
    echo 2. Build the solution (F7)
    echo 3. Run the project in Unreal Editor
    echo.
) else (
    echo.
    echo ERROR: Failed to regenerate project files!
    echo Please try manually:
    echo 1. Right-click on %UPROJECT%
    echo 2. Select "Generate Visual Studio project files"
    echo.
)

pause
