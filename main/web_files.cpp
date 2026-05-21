// Web file manager — implementation. See web_files.h for the API.
//
// Routes on port 80 (LAN, no TLS — for casual use on a trusted network):
//   GET  /             - HTML index of LittleFS root: list of files
//                         with download links + delete buttons + upload form.
//   POST /upload       - multipart file upload, written to LittleFS root.
//   GET  /file/<name>  - download a file (serveStatic).
//   POST /delete       - remove a file (name in form body).
//
// All file ops are against LittleFS only — SD use stays via CALL FILES
// because the SD slot can be hot-swapped and the lazy-mount semantics
// don't fit cleanly into the request/response cycle.

#include "web_files.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>

namespace webfiles
{

static AsyncWebServer s_server(80);
static bool s_started = false;

// In-flight upload state. ESPAsyncWebServer calls the upload handler
// in chunks; we hold the destination File between calls.
static File s_uploadFile;

// Build the index HTML page in memory. Sized for LittleFS at typical
// fill (dozens of .bas files); if you ever hit hundreds of files,
// switch to AsyncResponseStream for chunked emission.
static String buildIndexHtml()
{
  String html;
  html.reserve(4096);
  html += F("<!DOCTYPE html><html><head><meta charset=utf-8>"
            "<title>TI BASIC Files</title>"
            "<style>"
            "body{font-family:monospace;max-width:640px;margin:2em auto;padding:0 1em;background:#0a0a3d;color:#e0e0e0;}"
            "h1{color:#fff;border-bottom:1px solid #444;padding-bottom:0.3em;}"
            "table{width:100%;border-collapse:collapse;margin-top:1em;}"
            "td{padding:0.4em;border-bottom:1px solid #333;}"
            "td.size{text-align:right;color:#888;width:6em;}"
            "td.del{width:5em;text-align:right;}"
            "a{color:#7af;}"
            "form.inline{display:inline;margin:0;}"
            "button{background:#222;color:#eee;border:1px solid #555;padding:0.2em 0.6em;cursor:pointer;}"
            "button:hover{background:#333;}"
            ".upload{margin-top:1em;padding:0.8em;background:#11115a;border:1px solid #333;}"
            "input[type=file]{color:#ccc;}"
            "</style></head><body>"
            "<h1>TI BASIC Files</h1>"
            "<div class=upload>"
            "<form method=post action=/upload enctype=multipart/form-data>"
            "<input type=file name=file required> "
            "<button>Upload</button>"
            "</form></div>"
            "<table>");

  File root = LittleFS.open("/");
  File f = root.openNextFile();
  int fileCount = 0;
  while (f)
  {
    const char* name = f.name();
    // Skip directories and System Volume Information (none on LittleFS
    // but defensive).
    bool hide = f.isDirectory() || name[0] == '.' ||
                strcasecmp(name, "System Volume Information") == 0;
    if (!hide)
    {
      html += F("<tr><td><a href=\"/file/");
      html += name;
      html += F("\">");
      html += name;
      html += F("</a></td><td class=size>");
      html += (uint32_t)f.size();
      html += F("</td><td class=del>"
                "<form class=inline method=post action=/delete>"
                "<input type=hidden name=name value=\"");
      html += name;
      html += F("\"><button onclick=\"return confirm('Delete ");
      html += name;
      html += F("?')\">Delete</button></form>"
                "</td></tr>");
      fileCount++;
    }
    f = root.openNextFile();
    yield();
  }
  if (fileCount == 0)
  {
    html += F("<tr><td colspan=3 style=\"color:#888;text-align:center;\">"
              "(no files yet — upload one above)</td></tr>");
  }
  html += F("</table></body></html>");
  return html;
}

static void handleIndex(AsyncWebServerRequest* req)
{
  req->send(200, "text/html; charset=utf-8", buildIndexHtml());
}

// Upload: ESPAsyncWebServer calls this once per chunk. index==0 marks
// the first chunk (open the file); final==true marks the last (close).
static void handleUploadChunk(AsyncWebServerRequest* /*req*/,
                              const String& filename,
                              size_t index, uint8_t* data, size_t len,
                              bool final)
{
  if (index == 0)
  {
    // Sanitize: strip any leading path components the browser may
    // send; LittleFS lives in a flat namespace.
    String baseName = filename;
    int slash = baseName.lastIndexOf('/');
    if (slash >= 0) baseName = baseName.substring(slash + 1);
    int back = baseName.lastIndexOf('\\');
    if (back >= 0) baseName = baseName.substring(back + 1);
    String path = "/" + baseName;
    s_uploadFile = LittleFS.open(path, "w");
  }
  if (s_uploadFile)
  {
    s_uploadFile.write(data, len);
  }
  if (final && s_uploadFile)
  {
    s_uploadFile.close();
  }
}

static void handleUploadDone(AsyncWebServerRequest* req)
{
  // After the body finishes, send the user back to the index so they
  // see their newly-uploaded file in the list.
  AsyncWebServerResponse* res = req->beginResponse(303);
  res->addHeader("Location", "/");
  req->send(res);
}

static void handleDelete(AsyncWebServerRequest* req)
{
  if (!req->hasParam("name", /*post=*/true))
  {
    req->send(400, "text/plain", "missing name");
    return;
  }
  String name = req->getParam("name", true)->value();
  // Same sanitization as upload — never let a request escape the
  // LittleFS root.
  int slash = name.lastIndexOf('/');
  if (slash >= 0) name = name.substring(slash + 1);
  int back = name.lastIndexOf('\\');
  if (back >= 0) name = name.substring(back + 1);
  String path = "/" + name;
  bool ok = LittleFS.remove(path);
  if (ok)
  {
    AsyncWebServerResponse* res = req->beginResponse(303);
    res->addHeader("Location", "/");
    req->send(res);
  }
  else
  {
    req->send(404, "text/plain", "file not found");
  }
}

void init()
{
  if (s_started) return;
  s_started = true;

  s_server.on("/", HTTP_GET, handleIndex);
  s_server.on("/upload", HTTP_POST, handleUploadDone, handleUploadChunk);
  s_server.on("/delete", HTTP_POST, handleDelete);
  // Serve /file/<name> straight from LittleFS root.
  s_server.serveStatic("/file/", LittleFS, "/");
  // Fallback 404
  s_server.onNotFound([](AsyncWebServerRequest* req) {
    req->send(404, "text/plain", "not found");
  });

  s_server.begin();
}

void tick()
{
  // ESPAsyncWebServer is event-driven on top of AsyncTCP; no per-loop
  // work needed. Hook kept for future use (e.g. periodic cleanup of
  // long-lived chunked responses).
}

} // namespace webfiles
