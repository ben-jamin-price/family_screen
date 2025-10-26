# ESP32-P4 Display Demo

A lightweight graphics demo suite for the **ESP32-P4-Nano** board with a **JD9365 10.1" MIPI-DSI panel** and **GT911 touch controller**.  
Includes framebuffer drawing utilities, touch input, and several interactive display demos.

---

## Features

- ✅ **Display bring-up** for JD9365 MIPI-DSI panel (2-lane, RGB565, DMA2D)
- ✅ **GT911 touch support** over I²C (with reliable press/release detection)
- ✅ **Framebuffer utilities** and 5×7 ASCII font renderer
- ✅ **Interactive menu** to launch demos
- ✅ **Four sample demos**:
  - Color bars  
  - Vertical gradient  
  - Bouncing square  
  - Checkerboard sleep/wake

---

## Directory Layout
```
components/
├── board/          # Hardware configuration (pins, I2C, DSI lanes)
├── display_panel/  # JD9365 display driver (esp_lcd + DMA2D)
├── touch_gt9xx/    # GT911 touch initialization and read helpers
├── ui_gfx/         # 2D primitives + 5×7 bitmap font
├── ui_menu/        # Interactive 2×2 menu with touch highlighting
├── demos/          # Example graphics demos
└── util/           # Framebuffer allocator + timing helpers
main/
└── main.c          # Entry point: init, menu loop
```
---
# Build & Flash

## Prerequisites
- **ESP-IDF 5.2+** (tested with 5.2 / P4-SDK release)
- ESP32-P4-Nano development board
- JD9365-based 10.1" MIPI-DSI display connected to DSI0
- GT911 touch controller on I²C1

## Steps

### Configure the project (adjust COM port / target)

```bash
idf.py set-target esp32p4
idf.py menuconfig
```

### Build and flash

```bash
idf.py build flash monitor
```

# Running

On boot, the menu appears with four tiles:

| Tile | Demo Function | Description |
|------|----------------|-------------|
| **Color Bars** | `demo_color_bars()` | RGB test pattern |
| **Gradient** | `demo_vertical_gradient()` | Green intensity ramp |
| **Bounce 5s** | `demo_bounce_seconds()` | Animated moving square |
| **Sleep/Wake** | `demo_checker_sleep_wake()` | Panel suspend/resume test |

Touch a tile, lift to confirm selection, and the corresponding demo runs for a few seconds before returning to the menu.

---

## Licensing & Notes

All code is original and intended for educational and evaluation use.  
Font data in `ui_gfx/font5x7.c` is derived from public-domain 5×7 ASCII bitmaps.

---

## Acknowledgments

This project builds on:
- Espressif’s **ESP-LCD** and **MIPI DSI** driver stack  
- FreeRTOS task and timer APIs  
- Public-domain 5×7 ASCII font tables

---

*Author: Ben Price  
Platform: ESP-IDF 5.2 / ESP32-P4-Nano  
License: MIT (unless otherwise noted)*
