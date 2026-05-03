#!/bin/bash

set -euo pipefail

QT_DIR="${QT_DIR:-/home/user/Qt/6.8.3/gcc_64}"
BUILD_TYPE=""

print_usage() {
    echo "Usage: $0 --type <debug|release> [--qt <path-to-qt>]"
    echo "Example: $0 --type debug --qt /home/user/Qt/6.8.3/gcc_64"
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --qt)
            QT_DIR="$2"
            shift 2
            ;;
        --type)
            BUILD_TYPE="$2"
            if [[ "$BUILD_TYPE" != "debug" && "$BUILD_TYPE" != "release" ]]; then
                echo "Error: --type must be 'debug' or 'release'."
                print_usage
                exit 1
            fi
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

if [ -z "$BUILD_TYPE" ]; then
    echo "Error: --type is required."
    print_usage
    exit 1
fi

if [ "$BUILD_TYPE" = "debug" ]; then
    CMAKE_BUILD_TYPE="Debug"
    CONAN_DIR=".conan/Debug"
    BUILD_DIR="build/linux-debug"
else
    CMAKE_BUILD_TYPE="Release"
    CONAN_DIR=".conan/Release"
    BUILD_DIR="build/linux-release"
fi

if [ ! -f "$CONAN_DIR/conan_toolchain.cmake" ]; then
    echo "Missing Conan toolchain at $CONAN_DIR/conan_toolchain.cmake"
    echo "Run ./scripts/setup.sh first."
    exit 1
fi

cmake \
    -S . \
    -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$CONAN_DIR/conan_toolchain.cmake" \
    -DCMAKE_PREFIX_PATH="$QT_DIR;$CONAN_DIR" \
    -DOpenCV_DIR="$CONAN_DIR"

cmake --build "$BUILD_DIR" --parallel
