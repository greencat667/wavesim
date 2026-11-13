#!/bin/bash
# Recreate dependencies in a fresh checkout; downloads only official upstream releases.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p third_party research
[ -f third_party/json.hpp ] || curl -fL https://raw.githubusercontent.com/nlohmann/json/v3.12.0/single_include/nlohmann/json.hpp -o third_party/json.hpp
if [ ! -d third_party/esp-idf ]; then git clone --depth 1 --branch v5.5.2 --recursive --shallow-submodules https://github.com/espressif/esp-idf.git third_party/esp-idf; fi
export IDF_TOOLS_PATH="$PWD/third_party/idf-tools"
./third_party/esp-idf/install.sh esp32s3
# Explicit Python 3.12 environment is used by the desktop hardware helper.
python3.12 third_party/esp-idf/tools/idf_tools.py install-python-env
third_party/idf-tools/python_env/idf5.5_py3.12_env/bin/python -m pip install ninja
if [ ! -x third_party/qemu/bin/qemu-system-xtensa ]; then
 curl -fL https://github.com/espressif/qemu/releases/download/esp-develop-9.2.2-20260417/qemu-xtensa-softmmu-esp_develop_9.2.2_20260417-aarch64-apple-darwin.tar.xz -o third_party/qemu.tar.xz
 tar -xf third_party/qemu.tar.xz -C third_party
fi
./tools/build.sh
./tools/idf.sh -C examples/qemu-environment build
./tools/package.sh
