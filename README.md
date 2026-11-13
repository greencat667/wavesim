# WaveSim

A working local Apple Silicon macOS bench for the Waveshare ESP32-S3-ePaper-1.54 V2 (SKU 32298).

Open **dist/WaveSim.app**. No server, browser setup or npm install is required for the native bench.

## First run

1. Open the app. The Environment dashboard is already compiled and loaded.
2. In **Scenarios & Faults**, click **Load acceptance values**: 23.4°C, 71%, 2030-06-10 14:35 UTC.
3. Change Temperature to 31.2 and click **Refresh firmware**. The compiled C application reads SHTC3 bytes and redraws the e-paper framebuffer.
4. Click **Run** for repeated execution; Stop suspends application ticks while the virtual RTC continues.
5. Try sensor faults, time jumps, BOOT (hold B), power off/on, or the display diff inspector.

## What executes

The default backend compiles real C application source into a native ARM64 dynamic library. The C++ engine calls `wavesim_tick(const WaveHAL*)`. The app's input values are converted into SHTC3 protocol bytes including CRC, and RTC BCD registers. The firmware converts these readings and rasterizes its own 200×200 monochrome buffer. The UI only displays the resulting bytes.

The experimental **Xtensa QEMU** backend executes the same Environment C source compiled into an ESP32-S3 ESP-IDF binary. UART RPC bridges its HAL calls to the same C++ virtual board. This is CPU emulation with a peripheral bridge, not transparent MMIO emulation of the Waveshare board.

Existing ESP-IDF projects need an adapter to the small HAL. WaveSim does **not** run arbitrary ESP32 binaries against all board peripherals. See KNOWN_LIMITATIONS.md before choosing a project.

## CLI

From this directory:

```sh
./wavesim build examples/environment
./wavesim run examples/environment --steps 3
./wavesim run --target qemu
./wavesim test
node testing/environment.test.ts
./wavesim scenario run scenarios/rainy-morning.json
./wavesim device list
./wavesim build examples/environment --target physical
./wavesim flash examples/environment --port /dev/cu.usbmodemYOURDEVICE
./wavesim qemu research/idf-hello-flash.bin
```

`run` prints framebuffer hex and structured events as JSON. `build` compiles a project's `main/app.c` with the HAL header. Tests launch the same engine as the desktop. The HTTP test binds only localhost:8765 and sends only a firmware-requested GET.

## Editing firmware

Edit `examples/environment/main/app.c`, then **Open project** and choose the Environment directory. **Build** compiles and loads your changes. A project must expose `wavesim_tick(const WaveHAL*)`; normal ESP-IDF CMake project files are included. The physical build replaces the HAL, not the application source.

The native runner is cooperative: each tick must return. Use LLDB with the engine and `-g` compiled library for native breakpoints. There is no embedded debugger panel.

## Real device

The physical panel detects USB serial ports, builds ESP-IDF firmware, flashes only after **Flash selected device**, and opens a serial monitor. No device was flashed automatically. The packaged app points to this development repository for ESP-IDF/QEMU tools; keep it in place or repackage after moving it.

The physical driver targets V2. On-device verification requires a connected V2 board. Network credentials and URL are configured with `tools/idf.sh -C examples/network menuconfig` under **WaveSim physical networking**. The default URL is localhost for the simulator and must be changed for real hardware. Wi-Fi is disabled until an SSID is configured. SD physical adapters are not implemented.

## Rebuild and package

```sh
./tools/build.sh
./tools/package.sh
open dist/WaveSim.app
```

For a fresh source checkout, `tools/bootstrap.sh` installs the pinned dependencies (requires Homebrew Python 3.12; QEMU also uses Homebrew pixman, libgcrypt and SDL2).

Native prerequisites: macOS command-line developer tools. CLI tests use Python 3. TypeScript tests use Node 22.18+ or `tsx`. ESP-IDF and QEMU live under third_party. The app uses an ad-hoc local signature, not Apple notarization.

Read STATUS.md for tested capabilities, HARDWARE.md for sources, DEVELOPMENT.md for the engine API, and ARCHITECTURE.md for execution decisions.

## Licence

MIT, see [LICENSE](LICENSE). Third-party components and their licences are listed in [THIRD_PARTY.md](THIRD_PARTY.md). The Waveshare waveform table in `device/waveshare_lut.h` comes from the manufacturer's public demo repository, which carries no explicit licence; it is a small hardware configuration table, included with attribution.
