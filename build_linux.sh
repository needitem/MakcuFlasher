#!/bin/bash

# MakcuFlasher Linux Build Script

set -e

BUILD_DIR="build_linux"
BUILD_TYPE="${1:-Release}"

echo "Building MakcuFlasher for Linux..."
echo "Build type: $BUILD_TYPE"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

# Build
cmake --build . --config "$BUILD_TYPE" -j$(nproc)

echo ""
echo "Build complete!"
echo ""
echo "Executable: $BUILD_DIR/MakcuFlasher"
echo ""
echo "Usage:"
echo "  Interactive mode (recommended): ./MakcuFlasher"
echo "  Manual mode:                    ./MakcuFlasher <SERIAL_PORT> <FIRMWARE_FILE>"
echo ""
echo "Example:"
echo "  ./$BUILD_DIR/MakcuFlasher                              # Auto-detect device and firmware"
echo "  ./$BUILD_DIR/MakcuFlasher /dev/ttyUSB0 firmware/V3.8.bin  # Manual mode"
