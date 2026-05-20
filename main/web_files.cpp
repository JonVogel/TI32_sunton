// Web file manager — implementation. See web_files.h for the API.
//
// Step 4a (this commit): scaffolding only. Includes the async-server
// headers so the submodules are pulled into the link, and provides
// init/tick stubs. No HTTP behavior yet — that's step 4b.

#include "web_files.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

namespace webfiles
{

// Server lives in this translation unit so we can wire up routes in
// step 4b without exposing the AsyncWebServer type through web_files.h.
// Port 80 = plain HTTP. No TLS — the file manager is for LAN use.
static AsyncWebServer s_server(80);

void init()
{
  // Stub for step 4a. Step 4b adds:
  //   s_server.on("/", HTTP_GET, &handleIndex);
  //   s_server.on("/upload", HTTP_POST, ..., &handleUpload);
  //   s_server.on("/delete", HTTP_POST, &handleDelete);
  //   s_server.begin();
}

void tick()
{
  // Stub for step 4a. Most AsyncWebServer paths are event-driven and
  // don't need a per-loop tick, but reserve the hook so step 4b can
  // do periodic cleanup of in-flight uploads etc. if needed.
}

} // namespace webfiles
