#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
APP="dist/WaveSim.app/Contents"
mkdir -p "$APP/MacOS" "$APP/Resources/Source"
clang -fobjc-arc apps/desktop/main.m -o "$APP/MacOS/WaveSim" -framework Cocoa -framework WebKit
cp build/wavesim-engine build/*.dylib apps/desktop/index.html "$APP/Resources/"
python3 -c 'import json,os;json.dump({"sourceRoot":os.getcwd()},open("dist/WaveSim.app/Contents/Resources/development.json","w"))'
cp -R core adapters tools "$APP/Resources/Source/"
mkdir -p "$APP/Resources/Source/third_party/arduino"
cp third_party/json.hpp "$APP/Resources/Source/third_party/"
cp -R "$HOME/Documents/Arduino/libraries/Adafruit_GFX_Library" "$HOME/Documents/Arduino/libraries/ArduinoJson" "$APP/Resources/Source/third_party/arduino/"
mkdir -p "$APP/Resources/Source/examples"
for example in environment hello network; do mkdir -p "$APP/Resources/Source/examples/$example"; cp -R "examples/$example/main" "$APP/Resources/Source/examples/$example/"; done
for f in README.md ARCHITECTURE.md HARDWARE.md DEVELOPMENT.md STATUS.md KNOWN_LIMITATIONS.md; do [ ! -f "$f" ] || cp "$f" "$APP/Resources/Source/"; done
cat > "$APP/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><dict><key>CFBundleExecutable</key><string>WaveSim</string><key>CFBundleIdentifier</key><string>local.wavesim.desktop</string><key>CFBundleName</key><string>WaveSim</string><key>CFBundleVersion</key><string>1</string><key>CFBundleShortVersionString</key><string>0.1.0</string><key>NSHighResolutionCapable</key><true/><key>LSMinimumSystemVersion</key><string>13.0</string></dict></plist>
PLIST
codesign --force --deep --sign - "dist/WaveSim.app"
