# Smart Aquarium CYD V2

UI-preview firmware for the common ESP32-2432S028 / CYD 2.8-inch board.

## What this build does
- Landscape 320x240 dashboard
- Home / Water / Temperature / Manual / Settings screens
- Touch navigation
- Weekly water-change UI with adjustable percentage
- Temperature target UI
- Manual-control preview
- Safety/status placeholders
- GitHub Actions build and downloadable firmware artifact

## Important safety design
This V2 intentionally does **not** energize real valves, heaters, chillers, pumps, or mains relays.
The next hardware stage should use an isolated I/O/driver board (for example MCP23017 + suitable MOSFET/relay drivers), flyback protection for DC coils, and an independent emergency high-level cutoff.

## GitHub
Upload the CONTENTS of this folder to the repository root. Do not upload the outer folder itself.

Required layout:
- `.github/workflows/build.yml`
- `platformio.ini`
- `include/cyd_pins.h`
- `src/main.cpp`

Push to `main`, then open Actions > Build Smart Aquarium CYD.
Download artifact `Smart-Aquarium-CYD-Firmware`.

## Flash offsets
For a full manual flash:
- bootloader.bin -> 0x1000
- partitions.bin -> 0x8000
- boot_app0.bin -> 0xE000
- firmware.bin -> 0x10000

## Board variants
CYD boards have multiple revisions. This project targets the common ESP32-2432S028 pinout. Touch calibration/orientation can be fine-tuned after the first real-device test.
