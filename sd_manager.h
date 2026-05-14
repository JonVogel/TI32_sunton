// Lazy SD-card mount manager. The Sunton 8048S043C has no card-detect
// pin, so we can't get an interrupt when the user inserts a card. Two
// design options were considered:
//
//   1. Poll SD.begin() at ~1 Hz from the main loop.
//   2. Lazy mount: every SD-touching path probes SD.begin() first.
//
// Picked (2) because it matches the real-TI behavior (DSK1 spins up on
// access and takes a moment when no disk is present) and it doesn't
// burn CPU when nobody's asking for SD. The cost is a one-time
// SD.begin() delay (~tens of ms when a card is in, longer when not)
// on the first access after the slot becomes available or unavailable.
//
// requireSD() is the only function call sites need. It returns true
// if the card is mounted and ready to use. On failure (no card, or
// card was yanked and the FS calls are now stale) it returns false
// and the caller emits the usual TI device-error path.
//
// Call sites in this project:
//   - cmdSave / cmdOld / cmdMerge / cmdDelete for SDCARD.*
//   - cmdDir for SDCARD
//   - cmdMount / cmdCopy when the source/dest involves SDCARD
//   - shimFileOpen indirectly via fio::openFile / mountDskImage
//     (these check fio::g_sdOk, which sdmgr keeps in sync)

#pragma once

namespace sdmgr
{
  // Probe and (re)mount the SD card if needed. Returns true when the
  // card is mounted and the global fio::g_sdFs pointer is valid.
  //
  // First call: runs SD.begin() and SD.cardType() to confirm a real
  // card. On success, registers it with fio and runs loadMounts() so
  // persisted DSK1..N reattach.
  //
  // Subsequent calls: fast path — returns the cached state. If the
  // caller previously called invalidate() (because an I/O op failed
  // mid-flight), this retries SD.begin() with the TI-style delay the
  // user expects when no disk is present.
  bool requireSD();

  // Mark the current mount stale. The next requireSD() will tear down
  // and retry. Call this from the file_io layer when an in-flight SD
  // operation fails — that's a signal the card was yanked or the FAT
  // state got corrupt.
  void invalidate();
}
