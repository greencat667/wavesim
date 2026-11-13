#!/bin/bash
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export IDF_PATH="$ROOT/third_party/esp-idf"
export IDF_TOOLS_PATH="$ROOT/third_party/idf-tools"
export IDF_PYTHON_ENV_PATH="$IDF_TOOLS_PATH/python_env/idf5.5_py3.12_env"
TOOL="$IDF_TOOLS_PATH/tools/xtensa-esp-elf/esp-14.2.0_20251107/xtensa-esp-elf/bin"
if [ ! -x "$TOOL/xtensa-esp-elf-gcc" ]; then
 TOOL="$HOME/Library/Arduino15/packages/esp32/tools/esp-x32/2601/bin"
 export IDF_MAINTAINER=1
 echo "Using installed Arduino GCC 14.2 (20260121); IDF recommends 20251107. Build validation required." >&2
fi
export PATH="$IDF_PYTHON_ENV_PATH/bin:$TOOL:/opt/homebrew/bin:$PATH"
exec "$IDF_PYTHON_ENV_PATH/bin/python" "$IDF_PATH/tools/idf.py" "$@"
