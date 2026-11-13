# Architecture and execution decisions

## Vertical path

Native: C application → WaveHAL → C++ board models → packed 1bpp framebuffer → JSON lines over pipes → Objective-C/WebKit desktop.

QEMU: ESP-IDF Xtensa binary → UART HAL RPC → Python transport → the same C++ board models → desktop JSON protocol. Application source is shared with native and physical builds.

Physical: the same application C → WaveHAL → ESP-IDF I2C/SPI/GPIO/HTTP drivers → V2 board.

`core/engine.cpp` owns all hardware state and events. The browser does not compute sensor readings or draw firmware text. `core/drawing.h` is compiled into the application for both targets. `device/` contains physical and QEMU transport HALs. `testing/` supplies a TypeScript client. `tests/` contains Python integration tests and independent glyph pixel assertions.

## Why native WebKit and C++

This Mac already has a C compiler and system WebKit; no web runtime distribution or JavaScript package install is necessary. A thin Objective-C application controls child processes and native file panels. Swift compilation was attempted and encountered conflicting SwiftBridging modules in the installed command-line SDK; Objective-C uses the same native frameworks successfully. The engine is separate from the UI and testable with stdin/stdout.

## Execution options actually investigated

- Espressif QEMU Apple Silicon release esp-develop-9.2.2-20260417 was downloaded, installed and run. `-machine help` includes esp32s3. Manufacturer V2 factory firmware boots then aborts at octal PSRAM init. A freshly compiled ESP-IDF 5.5.2 minimal program boots and prints repeated heartbeats. The bundled Arduino 3.3.11 minimal build was also attempted; it asserts during startup. These logs are in research/.
- Native C HAL substitution was compiled and tested end-to-end. It is the default interactive execution path.
- ESP-IDF host target is officially a Linux preview target with selective component support, not a drop-in macOS virtual board. https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/host-apps.html
- Upstream QEMU's Xtensa target docs list generic Xtensa evaluation machines. The maintained Espressif fork is the practical ESP32-S3 choice. https://www.qemu.org/docs/master/system/target-xtensa.html
- Wokwi CLI requires a token; offline mode has a separate licensing workflow. No account/license was requested or installed. https://docs.wokwi.com/wokwi-ci/cli-usage and https://docs.wokwi.com/vscode/offline-mode
- Renode's supported-board listing did not provide an immediately usable Waveshare S3 target; no Renode executable was tested. https://renode.readthedocs.io/en/latest/introduction/supported-boards.html

No third-party S3 fork was preferred over Espressif's maintained implementation. Research-only alternatives are not described as runtime-tested.

## Fidelity boundaries

Native ticks are cooperative calls, not FreeRTOS scheduling. Model time advances explicitly, independently of wall-clock firmware execution. Realistic display mode schedules a completion event after approximate latency (2s full, 0.3s partial), retaining the last image until then. These durations are visual approximations, not measured hardware timing. I2C transaction events describe HAL transfers, not electrical waveforms. The SHTC3 conversion formulas and CRC are modelled; its complete command/state machine and acquisition timing are not.

The QEMU guest is suspended between requested ticks to avoid consuming a CPU core while the bench is idle. Virtual peripheral time is advanced by the host protocol, not inferred from guest CPU cycles.

The profile is declarative documentation for this implemented board. Additional profiles alone do not instantiate new board plugins yet.
