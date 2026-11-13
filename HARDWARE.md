# Waveshare ESP32-S3-ePaper-1.54 V2

SKU **32298**, non-touch model. Manufacturer documentation says V2 replaced V1 from 2025-11-01. V2 uses ESP32-S3-PICO-1-N8R8 (8MB flash, 8MB PSRAM). V1 uses ESP32-S3FH4R2 (4MB flash, 2MB PSRAM). Select V2 for this environment; confirm PCB silkscreen before physical use.

## Provenance

- Product: https://www.waveshare.com/esp32-s3-epaper-1.54.htm?sku=32298
- Current documentation: https://docs.waveshare.com/ESP32-S3-ePaper-1.54
- Legacy wiki: https://www.waveshare.com/wiki/ESP32-S3-ePaper-1.54 (returned HTTP 403 during research; current docs used)
- Manufacturer source: https://github.com/waveshareteam/ESP32-S3-ePaper-1.54
- Local manufacturer checkout inspected: commit `9957d0f4fc7cd40d1d42880cb1b74a8d6782a6c2` in the existing IoT project vendor directory.
- Pin evidence: `02_Example/ESP-IDF/V2/03_I2C_SHTC3/main/user_config.h`, `11_FactoryProgram/components/port_bsp/epaper_config.h`, `08_Audio_Test/components/codec_board/board_cfg.txt`, section `S3_ePaper_1_54`.
- Current resources: https://docs.waveshare.com/ESP32-S3-ePaper-1.54/Resources-And-Documents
- Linked schematic: https://files.waveshare.com/wiki/ESP32-S3-ePaper-1.54/ESP32-S3-Touch-ePaper-1.54-Schematic.pdf . Manufacturer's current resource link is the shared Touch schematic; common signals cross-checked against non-touch V2 source. Touch-only wiring is not assigned to SKU 32298.
- SHTC3: https://files.waveshare.com/wiki/common/SHTC3_Datasheet.pdf
- RTC: https://files.waveshare.com/wiki/common/Pcf85063atl1118-NdPQpTGE-loeW7GbZ7.pdf
- Display: https://files.waveshare.com/wiki/common/1.54inch_e-paper_V2_Datasheet.pdf

## Pin map

| Subsystem | Signals / GPIO |
|---|---|
| e-paper SPI2 | MOSI 13, clock 12, CS 11, DC 10 |
| e-paper control | RESET 9, BUSY 8 (high busy), power enable 6 |
| I2C0 | SDA 47, SCL 48 |
| SHTC3 | address 0x70 on I2C0 |
| PCF85063 RTC | address 0x51 on I2C0, interrupt 5 |
| BOOT / PWR | 0 / 18, active-low inputs |
| SD card | 1-bit SDMMC: clock 39, CMD 41, D0 40 |
| ES8311 audio | I2C0, default address 0x18; MCLK 14, BCLK 15, WS 38 |
| Speaker / mic | codec DOUT from ESP 45, DIN into ESP 16; analog mic/speaker terminate at codec/amplifier |
| Audio controls | PA 46, audio power 42 |
| Battery | VBAT power hold 17, battery ADC 4 |
| UART | TX 43, RX 44 |
| USB | D− 19, D+ 20 |
| Exposed GPIO | 1, 2, 3; headers also expose I2C/UART/power |
| Reset | chip enable/reset net, not an invented numbered GPIO |

The declarative JSON profile contains the same map. Pins 7/21 belong to touch-related wiring in the shared schematic and are not enabled here. No generic ESP32 pin map was substituted for this board.

## Driver model

Display: 200×200 monochrome, 25 bytes per row, white=1. The physical adapter follows the manufacturer's V2 initialization/write sequence and full-refresh waveform table, with bounded BUSY waits. The LUT provenance is retained in `device/waveshare_lut.h`.

SHTC3: raw temperature `-45 + 175 × raw / 65536`; relative humidity `100 × raw / 65536`; CRC8 polynomial 0x31, initial 0xff. Simulator handles the dashboard's measurement transactions, not every SHTC3 mode.

RTC: dashboard reads seven BCD fields starting at register 0x04. Simulator converts its UTC epoch to those fields. Oscillator-invalid bit on physical hardware causes an error display. RTC setting/alarms are outside this adapter.
