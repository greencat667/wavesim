#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
clang++ -std=c++17 -g -O1 core/engine.cpp -o build/wavesim-engine
for example in environment hello network; do
 clang -dynamiclib -g -O1 -Icore "examples/$example/main/app.c" -o "build/$example.dylib"
done
