# Known limitations

- This is a working scoped simulator, not a complete emulator of every peripheral on the physical board.
- Default execution is native ARM64 C, not Xtensa. ESP-IDF projects need the provided HAL boundary; arbitrary prebuilt firmware is not transparently supported.
- Experimental QEMU dashboard uses UART RPC. It does not emulate the board's external devices at MMIO level. QEMU supports only the shared Environment example through this bridge. CPU reset/state snapshots and exact timing are not modelled by the bridge's reset command.
- Real board validation and flashing require the user's connected board and explicit Flash action. Compilation does not prove physical display orientation, RTC initialization or electrical behaviour.
- Physical network code needs Wi-Fi credentials and a reachable URL. Physical SD callbacks return unsupported. Physical partial refresh is unsupported.
- Digital GPIO only; no mode enforcement, pull resistors, contention, ADC, PWM or interrupt scheduler. Profile swapping/plugin loading is not implemented.
- SHTC3 conversion/CRC and selected commands are supported; full wake/sleep/measurement timing is not. RTC alarms, register writes and oscillator drift are not modelled. Dates are UTC and intended for 2000–2099.
- E-paper latency is approximate. Realistic mode shows an inverse-and-wipe transition during its logical busy interval, but does not model calibrated ghosting, LUTs, electrophoretic behavior, or region-based partial refresh. Rotation is an inspector transform, not a change to raw framebuffer memory.
- Power lifecycle works; battery percentage is state only. No battery discharge/capacity/current estimator or electrical model.
- SD is a directory bridge, not FAT emulation. No desktop file browser/delete action, corrupt-file or slow-card model.
- HTTP/HTTPS host bridge only; no ESP Wi-Fi stack emulation, MQTT/WebSocket/TCP/UDP adapters, packet loss or latency shaping. HTTP has a three-second timeout and 1MB cap.
- Serial log capture uses the HAL logger; arbitrary printf/stderr from a native plugin is not redirected into the console. Serial input to simulated firmware is unsupported. Hardware monitor is read-only.
- No 3D model, audio, microphone, Bluetooth, embedded debugger or general Arduino/PlatformIO compatibility. There is a scoped Arduino source adapter (adapters/arduino) covering a small subset of the Arduino API; HTTP is fixture-only.
- Desktop ticks are serialized. A non-returning/crashing native firmware tick can stop its child engine; there is not yet a watchdog/restart UI.
- Scenario replay records hardware commands, not complete CPU/process snapshots. QEMU replay/reset behaviour has a narrower scope than native execution.
- App is ad-hoc signed, not notarized. Native simulator is self-contained; QEMU and physical-development actions use this repository's installed tools and path recorded during packaging.
