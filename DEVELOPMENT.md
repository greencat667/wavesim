# Development

## Engine protocol

Launch `build/wavesim-engine build/environment.dylib`. One JSON command per stdin line yields one JSON response on stdout. Responses have `ok`, `state`, and optionally `error`. State includes a 10,000-character hex framebuffer (5,000 bytes), previous framebuffer, events, GPIO values, RTC epoch, and fidelity table. White bits are 1, MSB first, row-major, 25 bytes per row. PBM exports invert these bits because PBM uses 1=black.

Examples:

```json
{"op":"set","temperature":23.4,"humidity":71,"epoch":1907332500,"rate":0}
{"op":"run"}
{"op":"set","temperature":31.2}
{"op":"run"}
{"op":"advance","seconds":1,"run":false}
{"op":"jump","seconds":86400}
{"op":"button","pin":0,"down":true}
{"op":"button","pin":0,"down":false}
{"op":"power","on":false}
{"op":"sleep","on":true}
{"op":"set","fault":"sensor_missing"}
{"op":"set","fault":""}
```

`advance` adds elapsed model seconds and multiplies RTC progression by rate. `jump` changes RTC directly. GUI month jump uses JavaScript UTC calendar normalization. The RTC is displayed in UTC to avoid DST-dependent tests. PCF85063's two-digit year is best used for 2000–2099.

Other commands: load, reset, gpio, display_sleep, clear (events), record, replay, sd_read, sd_write, state. The internal `io` command exposes HAL I2C, display and GPIO operations for the QEMU bridge. Errors return `ok:false`; unsupported I2C transactions return negative codes.

## C application contract

Export `void wavesim_tick(const WaveHAL *h)`. It is called for every Run/refresh operation. Keep persistent application data in static storage; Reset unloads/reloads the library in native mode. A tick must return. HAL callbacks are synchronous. `h->display` takes exactly 5,000 bytes; `partial=1` selects the shorter simulated completion time. Physical partial refresh returns an error.

Firmware can call I2C 0x70 commands 0x3517 and 0x7866, with a six-byte measurement read. RTC 0x51 reads register 4 onward for seven BCD fields. GPIO callbacks support digital values. SD paths are relative to the selected directory and cannot traverse outside it. HTTP/HTTPS is a real host curl request made only when firmware calls the HAL.

## Tools

ESP-IDF 5.5.2 and its recommended compiler are installed locally. `tools/idf.sh` sets the project tool environment. If the recommended compiler is absent, it can use the already installed Arduino GCC 14.2 variant and prints an explicit warning; normal validated builds use the recommended toolchain.

QEMU firmware: `tools/idf.sh -C examples/qemu-environment build`. `tools/qemu_bridge.py` speaks the same GUI protocol and bridges to a real Xtensa guest. Minimal binary boot proof is `examples/qemu-hello`; `research/qemu-idf-hello-boot.log` records it.

Debug native code with `lldb -- build/wavesim-engine build/environment.dylib` and a breakpoint at `wavesim_tick`. QEMU CLI `--gdb` starts paused on the default GDB port 1234. Neither debugger is embedded in the desktop UI.
