# Third-party provenance

- nlohmann/json 3.12.0: MIT, license retained in third_party/json.hpp.
- Espressif QEMU esp-develop-9.2.2-20260417: QEMU GPLv2 and component licences. Official release downloaded to third_party; upstream source at https://github.com/espressif/qemu . It is used locally, not embedded in the standalone native app.
- ESP-IDF v5.5.2: Apache-2.0 plus component licences, full local checkout in third_party/esp-idf.
- Waveshare full-refresh waveform table is copied from the existing manufacturer checkout, with exact provenance in device/waveshare_lut.h. Review the manufacturer's licensing before redistributing that physical driver.
- Drawing glyphs are a compact standard 5×7 bitmap representation written into drawing.h for this application. No external fonts or UI image assets are bundled.
