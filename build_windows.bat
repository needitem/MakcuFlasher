@echo off
REM MakcuFlasher Windows Build Script

setlocal

set BUILD_DIR=build
set BUILD_TYPE=Release

if not "%1"=="" set BUILD_TYPE=%1

echo Building MakcuFlasher for Windows...
echo Build type: %BUILD_TYPE%

REM Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Configure with CMake
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..

REM Build
cmake --build . --config %BUILD_TYPE%

echo.
echo Build complete!
echo.
echo Executable: %BUILD_DIR%\%BUILD_TYPE%\MakcuFlasher.exe
echo.
echo Usage: MakcuFlasher.exe ^<SERIAL_PORT^> ^<FIRMWARE_FILE^>
echo Example: MakcuFlasher.exe COM3 firmware_v3.8.bin

cd ..
