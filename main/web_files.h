// Web file manager — small HTTP server for uploading/downloading/
// deleting BASIC programs on the board's LittleFS partition. Lets the
// user drop a .bas file from a browser instead of going through the
// serial paste path. Only active when WiFi is connected (STA mode).
//
// This file is the public API; web_files.cpp has the implementation.
// This commit lands the scaffolding only — init() and tick() are stubs;
// the actual upload/download/delete handlers come in the next commit.

#pragma once

namespace webfiles
{

// Called once from setup() after tiWifiOn(). Safe to call regardless
// of WiFi state; the server starts unconditionally and serves once
// WiFi associates and we have an IP.
void init();

// Called once per main loop() iteration. Some handler types in
// ESPAsyncWebServer (notably non-async chunked responses) need
// per-loop tending; tick() is the host's anchor for that. Currently
// a no-op stub.
void tick();

} // namespace webfiles
