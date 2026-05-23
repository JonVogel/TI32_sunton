#!/usr/bin/env python3
"""Headless serial capture for capturing boot logs in CI-style runs.

Usage: serial_capture.py <port> <duration_seconds> [out_file]

Toggles DTR/RTS to reset the ESP32-S3 then reads bytes for `duration_seconds`,
prints them to stdout and tees to `out_file` if given. Avoids miniterm-style
interactive monitor which fails without a TTY.
"""
import sys
import time
import serial


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__, file=sys.stderr)
        return 2
    port = sys.argv[1]
    duration = float(sys.argv[2])
    out_path = sys.argv[3] if len(sys.argv) > 3 else None

    s = serial.Serial(port, 115200, timeout=0.1)
    # Standard esptool/IDF "default_reset": RTS=False, DTR=True, sleep,
    # then DTR=False to enter normal boot.
    s.setDTR(False)
    s.setRTS(True)
    time.sleep(0.1)
    s.setDTR(False)
    s.setRTS(False)
    time.sleep(0.05)
    # Hard reset via EN: RTS controls EN on most ESP boards.
    s.setRTS(True)
    time.sleep(0.1)
    s.setRTS(False)

    out_fh = open(out_path, "wb") if out_path else None
    deadline = time.time() + duration
    try:
        while time.time() < deadline:
            data = s.read(4096)
            if data:
                sys.stdout.buffer.write(data)
                sys.stdout.buffer.flush()
                if out_fh:
                    out_fh.write(data)
                    out_fh.flush()
    finally:
        if out_fh:
            out_fh.close()
        s.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
