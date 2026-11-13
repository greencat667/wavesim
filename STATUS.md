# Development status — 2026-09-07

## Verified working

- Apple Silicon desktop application compiled, launched and visually inspected.
- Native compiled C application → virtual SHTC3/RTC transactions → 200×200 framebuffer → real desktop display.
- Acceptance values 14:35, 23.4°C, 71%, then 31.2°C verified in the desktop and independent framebuffer pixel assertions.
- Eleven Python integration tests, including QEMU/native parity. Native cases: sensor changes, year rollover at 60×, offline/online real HTTP recovery, retained image without power, fault response, refresh latency, buttons, scenario replay, SD containment/read-only, sleep/wake.
- TypeScript hardware client test with framebuffer hash/pixel and event assertions.
- CLI build, run, tests, scenarios and USB enumeration.
- Display previous/diff views, zoom/grid/rotation and PNG export controls.
- Sensor ramp/noise; RTC pause/rate/time jumps; digital GPIO; directory-backed SD read/write.
- Native project compilation and dynamic loading.
- ESP-IDF 5.5.2 physical Environment, Hello and Network firmware compiled successfully; manufacturer-derived V2 pin map and display sequence.
- QEMU Environment: native-identical framebuffer and live 23.4 → 31.2°C changes verified headlessly and in the desktop.
- Espressif QEMU installed and minimal ESP32-S3 firmware boot verified with repeated heartbeats.

## Experimental / partial

- QEMU Environment binary uses UART HAL bridging to the same virtual board; see research/test logs for runtime validation.
- Physical build/explicit flash/serial controls are implemented; no automatic flash and no hardware claim without the board.
- E-paper full/partial completion timing is approximate; no calibrated ghosting.
- Network is real host HTTP/HTTPS with offline/DNS-failure injection; not an ESP Wi-Fi emulator.
- SD is directory backed; physical SD adapter is unsupported.
- Profile is declarative but additional-board plugin loading is not implemented.

## Not implemented

Bluetooth, audio, 3D device, ADC/PWM/electrical GPIO, complete FreeRTOS/ESP-IDF host compatibility, general arbitrary-binary board emulation, embedded debugger, serial RX adapter, full physical-device validation, sophisticated battery discharge, packet-loss shaping, FAT images.

See KNOWN_LIMITATIONS.md for precise boundaries. UI capability labels are supplied by the engine, not inferred from the existence of controls.
