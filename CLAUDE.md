# TI Extended BASIC for ESP32

TI Extended BASIC interpreter targeting the **ESP32-8048S043C** dev board (ESP32-S3 + 800×480 RGB panel). Sketch: `ti-extended-basic-esp32.ino`. Built with arduino-cli via `build.bat`.

This project was split out from `c:\dev\repos\ti-99-keyboard\ti-basic\` on 2026-04-28 via `git subtree split --prefix=ti-basic`, so all sprite / DB / file-IO / open-modes history is preserved. **Future work happens here, not in the keyboard repo** — the keyboard repo's copies are stale as of split commit `b85c14f` on `TI_Extended_BASIC`.

## Build / upload

- `build.bat compile` — compile only.
- `build.bat upload` — compile + upload. Default port is **COM17** (set inside the script, no need to pass).
- `build.bat monitor` — serial monitor.
- `build.bat killmonitor` — kill any lingering `arduino-cli.exe` holding the port (typical after `build.bat monitor`); the upload step calls this automatically before each upload so the COM port is free.
- `--libraries .` is passed so the vendored `BleHidHost/` resolves.

The sketch file is `ti-extended-basic-esp32.ino` (renamed from `ti-basic.ino`) so arduino-cli's "folder name = sketch name" rule is satisfied.

`BleHidHost/` is **vendored** (copy, not submodule). The keyboard repo also has its own copy from before the split — updates need to be applied to whichever is active.

### Auto-upload after a clean compile

After a clean compile (`errorlevel 0`), go ahead and upload — don't ask whether to upload. **But** ask "Ready to upload — close the serial monitor on COM17?" first, because the user usually has a monitor holding the port open and silent "port busy" failures waste a build cycle.

Don't auto-upload if a `build.bat compile` was run incidentally (e.g. as part of investigation where no upload was requested) — the rule is for explicit "build this" requests. And confirm before any *destructive* action beyond a normal upload (erasing flash, reformatting LittleFS, changing partition scheme).

## Known: NEWDISK on FLASH tears the display

`NEWDISK FLASH.*.DSK` causes the 800×480 RGB panel to tear / glitch for the 1–2 seconds the flash is being written. The final state repaints cleanly, but the create window is visually ugly.

**Why:** Flash erase/program on the ESP32-S3 masks interrupts. The RGB panel driver's bounce-buffer refill runs in an interrupt handler, so when flash blocks those interrupts the DMA keeps pushing stale buffer contents → visible tearing. **The mechanism is the flash ISR mask, not scheduling — `yield()` / `delay()` between writes does not help.** Don't chase yield-based fixes.

What's been tried (still glitches): 1 KB chunk writes with 2 ms delays, `fh.flush()` before close, full `paintBorder() + redrawScreen()` after create, Y/N overwrite confirmation.

Possible real fixes (not attempted):

- Larger bounce buffer (currently 32 KB SRAM — BLE competes for SRAM).
- Write on SDCARD instead of FLASH (SD SPI may not mask interrupts the same way; SDCARD `NEWDISK` may already be clean — needs verification).
- Background task for disk writes so the main task keeps the display fed (cleanest path if this becomes painful).
- `esp_flash_os_functions_init` with "allow-interrupt" mode (risky).
- Disable display DMA during create, draw a "Please wait…" first.

User asked for this to be tracked rather than fixed (2026-04-20).

## CALL SOUND — silent stub

`CALL SOUND` in `token_parser.h::execCall` is parsed and timed correctly but produces no audio. As of 2026-04-20:

- Parses `CALL SOUND(duration, freq, vol [, freq, vol ...])` with up to 4 voices.
- Tracks scheduled end time in `TokenParser::m_soundEndTime`.
- **Positive duration:** waits for any prior sound to finish, then schedules and returns immediately (matches TI semantics).
- **Negative duration:** cancels any in-flight sound and schedules the new one immediately.
- Frequency and volume are ignored — no tone is produced.

The wait semantics took a correction: TI waits for the **previous** sound, not the current one. The stub is now behaviorally correct, just silent.

**Why still a stub:** the 8048S043C has no onboard speaker / amp wired. ESP32-S3 native DAC pins (GPIO17/18) are already used by the display, so a free GPIO needs to be picked for either DAC+amp, I²S DAC, or piezo+PWM. Four simultaneous voices (matching SN76489) needs either software mixing to one DAC channel or an external sound chip.

When revisiting: pick an audio path → build a mixer for 3 tone + 1 noise voice at ~22 kHz → replace the wait loop in `execCall SOUND` with a call into an audio task. Audio runs on a timer interrupt and must not be starved during LittleFS writes (see NEWDISK note above).

## CALL JOYST — keyboard mirror works, gamepad pending

**Current behavior:** `bleKbOnReport` mirrors keyboard arrow keys into `joyArrow*`, so `CALL JOYST` works from the existing BLE keyboard. This is the workaround in production.

**Single-pad scope.** User stated "I never used 2" — `CALL JOYST(unit, ...)` with `unit ≠ 1` should just return centered. Don't bother with dual-connection state.

**Gamepad scaffolding is committed and ready** — `ble_gamepad.h` plus the wiring in `token_parser` and `ti-extended-basic-esp32.ino`. Only the BLE pairing to a working gamepad is missing. A different controller should slot straight into the existing `bleGpOnReport` path.

**Hardware history:**

- 8BitDo Zero 2 (~$20, ordered 2026-04-26) — **failed.** After firmware update + name-filter fix (`Q352020` model code added to `BleHidHost` name hits) and a 3-attempt connect retry, the gamepad is recognized in scans (advertises as `Q352020`) but BLE `connect()` consistently fails on all 3 attempts. Likely a NimBLE/Bluedroid edge case with this specific unit.
- Next to try: **8BitDo Ultimate (~$25)** — documented as ESP32-S3-BLE compatible. Recommended over retrying the Zero 2.

TI's `CALL JOYST(unit, X, Y)` returns -4/0/+4 per axis; a digital BLE HID D-pad maps cleanly with no deadband math. Map one face button to FIRE so `CALL KEY` can detect it.

## Code style

- Allman braces.
- Mandatory braces on **all** control flow (no single-statement `if`/`for`/`while` without braces).
