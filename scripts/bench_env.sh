#!/usr/bin/env bash
set -euo pipefail

echo "## System"
hostnamectl || true
echo

echo "## CPU"
lscpu | grep -E 'Model name|Architecture|CPU\(s\)|Thread|Core|Socket|L1d|L1i|L2|L3' || true
echo

echo "## Compiler"
if command -v g++ >/dev/null 2>&1; then
    g++ --version | head -1
fi

if command -v clang++ >/dev/null 2>&1; then
    clang++ --version | head -1
fi
echo

echo "## CMake"
cmake --version | head -1
echo

echo "## perf"
if command -v perf >/dev/null 2>&1; then
    perf --version
fi

if [[ -f /proc/sys/kernel/perf_event_paranoid ]]; then
    echo "perf_event_paranoid: $(cat /proc/sys/kernel/perf_event_paranoid)"
fi
echo

echo "## Git"
echo "commit: $(git rev-parse HEAD)"
echo "branch: $(git branch --show-current)"
echo "dirty files:"
git status --short
echo

echo "## CMake cache"
CACHE_FILE="build/release/CMakeCache.txt"

if [[ -f "$CACHE_FILE" ]]; then
    grep -E 'CMAKE_BUILD_TYPE|CMAKE_CXX_FLAGS|CMAKE_CXX_COMPILER' "$CACHE_FILE" || true
else
    echo "No CMake cache found at $CACHE_FILE"
fi
