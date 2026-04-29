# TI Extended BASIC for ESP32-S3

A from-scratch TI Extended BASIC interpreter targeting the Sunton
ESP32-8048S043C 4.3" RGB-display dev board. Tokenized REPL, V9T9
disk-image support, software sprite layer over a double-buffered
800×480 RGB panel, and BLE-HID keyboard input.

Where to buy the dev board:

- <https://www.amazon.com/dp/B0CLGCMWQ7>
- <https://www.aliexpress.us/item/3256809024564764.html>
- <https://www.aliexpress.us/item/3256808406435888.html>
- <https://www.aliexpress.us/item/3256809832274384.html>

> **⚠ Work in progress.** This is an active, evolving project.
> The language coverage and runtime are usable today (see
> `KEYWORDS.md` for the implementation status of every feature),
> but expect rough edges, partially-implemented corners, and
> behavior that may change between commits. Bug reports and PRs
> welcome.

## Hardware

- **MCU**: ESP32-S3 with 16 MB flash and 8 MB octal PSRAM.
- **Display**: 800×480 RGB panel via the parallel RGB interface,
  rendered at 32×24 TI-style char cells (16×16 px each).
- **Storage**: internal LittleFS (`FLASH.`), micro-SD on SPI
  (`SDCARD.`), and mountable V9T9 `.dsk` images (`DSK1.` … `DSKn.`).
- **Input**: any BLE HID keyboard (tested with ProtoArc L75) via the
  bundled `BleHidHost/` library.

## Prerequisites

### Arduino IDE / arduino-cli

Either works. The build scripts use `arduino-cli` and assume
`COM17` by default (override with `build.bat upload COM7`).

### ESP32 board package

Install the **`esp32:esp32`** platform, version **3.x** (3.3.8+
verified). For arduino-cli:

```bash
arduino-cli core update-index
arduino-cli core install esp32:esp32
```

Or in the Arduino IDE, add this Boards Manager URL:
`https://raw.githubusercontent.com/espressif/arduino-esp32/master/package/package_esp32_index.json`
and install **esp32 by Espressif Systems**.

### Arduino libraries

Install via Library Manager (or `arduino-cli lib install`):

| Library | Tested version | Notes |
|---|---|---|
| **GFX Library for Arduino** (`Arduino_GFX`) | 1.6.5 | Provides `Arduino_RGB_Display` and the RGB-panel data-bus; the project also vendors a forked panel (`rgb_db.h/.cpp`) that adds double buffering. |

Built-in (ship with the ESP32 core, no separate install):
`SD`, `SPI`, `LittleFS`, `FS`, `BLE`, `Preferences`, `Wire`.

The repo also vendors **`BleHidHost/`** (BLE HID host for keyboards
and gamepads) at the top level — it's picked up automatically via
the `--libraries` flag in `build.bat`, no manual install needed.

### Board configuration (FQBN)

The build scripts use:

```
esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=default
```

If using the Arduino IDE: select **ESP32S3 Dev Module**, set
**PSRAM = OPI PSRAM**, **Flash Size = 16MB**, **Partition Scheme =
3M App / 9M FATFS**, **USB CDC On Boot = Disabled** (the default).

## Building & flashing

```bash
build.bat                # compile + upload + monitor
build.bat compile        # compile only
build.bat upload         # upload only (auto-kills any open serial monitor)
build.bat monitor        # serial monitor on COM17
build.bat all COM7       # everything, on a non-default port
```

The script defaults to `COM17`; pass any other port as the second
arg. The `:killmonitor` helper finds and terminates any lingering
`arduino-cli.exe` (typically a stale `build.bat monitor`) before
each upload so the COM port is free.

## Reference docs

- **`KEYWORDS.md`** — implementation status of every TI BASIC / XB
  keyword and CALL subprogram.
- **`EXTENSIONS.md`** — the simulator's additions beyond TI XB
  (CALL SPEED / DELAY / TIMER, `!` tail comments, MOUNT/UNMOUNT/
  NEWDISK/COPY/CAT, FLASH./SDCARD./DSK*. device prefixes, sprite
  layer, file-handle limits, etc.).
- **`TEST_PROGRAMS.txt`** — 40+ self-contained test programs
  covering language features, graphics, sprites, file I/O, etc.
- **`PORT_NOTES.md`** — notes on porting to other ESP32-S3 boards.
- **`GROM_NOTES.md`** — research notes on the original TI Extended
  BASIC GROMs.
- **`TI Extended Basic Manual.pdf`** — Texas Instruments' original
  TI Extended BASIC user reference manual. The canonical source of
  truth for what each statement / function / CALL subprogram is
  supposed to do, against which this simulator's behavior is
  measured.

## Utilities

### `char_editor.py` — TI character pattern editor

GUI tool for drawing 8×8 character bitmaps and exporting them as
16-char hex strings ready to paste into a `CALL CHAR(n, "…")`
statement. Useful for designing custom sprite glyphs (the Pac-Man /
Ghost / Space-Invader patterns in the chaos demo were built this way).

```bash
python char_editor.py
```

Stdlib-only (uses `tkinter`), see `requirements.txt`. On Linux you
may need to install tkinter separately (`apt install python3-tk` on
Debian/Ubuntu).

## License

MIT — see `LICENSE` if/when added.
