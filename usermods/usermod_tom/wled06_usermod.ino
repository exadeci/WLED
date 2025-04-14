#include "wled.h"

static const char customPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body style="background-color:#ccc;font-family:Arial;">
<h2>Custom LED Control</h2>
<form method="POST" action="/myui">
<div>
<label>Brightness</label>
<input type="range" name="brightness" min="0" max="255">
</div>
<div>
<label>Snd React</label>
<input type="range" name="sndReact" min="0" max="255">
</div>
<div style="margin-top:10px;">
<input type="submit" value="Green" name="color">
<input type="submit" value="Pink" name="color">
<input type="submit" value="Blue" name="color">
<input type="submit" value="Red" name="color">
<input type="submit" value="Orange" name="color">
</div>
<div style="margin-top:10px;">
<input type="submit" value="Turn around" name="action">
<input type="submit" value="Blink" name="action">
<input type="submit" value="Blink Around" name="action">
</div>
<div style="margin-top:10px;">
<input type="submit" value="Black Out" name="action" style="background-color:#000;color:#fff;">
</div>
<div style="margin-top:10px;">
<input type="submit" value="SMOKE" name="action" style="background-color:#f00;color:#fff;">
</div>
</form>
</body>
</html>
)=====";

class WLED06Usermod : public Usermod {
  byte brightness = 128;
  byte sndReact = 0;

  void handleColors(const String& c) {
    if (c == "Green") for (int i = 0; i < strip.getLengthTotal(); i++) strip.setPixelColor(i, 0x00FF00);
    if (c == "Pink") for (int i = 0; i < strip.getLengthTotal(); i++) strip.setPixelColor(i, 0xFF1493);
    if (c == "Blue") for (int i = 0; i < strip.getLengthTotal(); i++) strip.setPixelColor(i, 0x0000FF);
    if (c == "Red") for (int i = 0; i < strip.getLengthTotal(); i++) strip.setPixelColor(i, 0xFF0000);
    if (c == "Orange") for (int i = 0; i < strip.getLengthTotal(); i++) strip.setPixelColor(i, 0xFFA500);
    strip.setBrightness(brightness);
    strip.show();
  }

  void handleActions(const String& a) {
    if (a == "Turn around") strip.setMode(11);
    if (a == "Blink") strip.setMode(2);
    if (a == "Blink Around") strip.setMode(14);
    if (a == "Black Out") {
      strip.setBrightness(0);
      strip.show();
    }
    if (a == "SMOKE") {
      for (int i = 0; i < strip.getLengthTotal(); i++) strip.setPixelColor(i, 0x808080);
      strip.setBrightness(brightness);
      strip.show();
    }
  }

 public:
  void setup() {}
  void loop() {}
  void addToJsonInfo(JsonObject& root) {}
  void addToJsonState(JsonObject& root) {}
  void readFromJsonState(JsonObject& root) {}
  void handleOverlayDraw() {}

  void handleWebServer(AsyncWebServerRequest *request) {
    if (request->url() == F("/myui")) {
      if (request->method() == HTTP_POST) {
        if (request->hasParam("brightness", true)) {
          brightness = request->getParam("brightness", true)->value().toInt();
          strip.setBrightness(brightness);
          strip.show();
        }
        if (request->hasParam("sndReact", true)) {
          sndReact = request->getParam("sndReact", true)->value().toInt();
        }
        if (request->hasParam("color", true)) {
          handleColors(request->getParam("color", true)->value());
        }
        if (request->hasParam("action", true)) {
          handleActions(request->getParam("action", true)->value());
        }
        request->send_P(200, "text/html", customPage);
        return;
      }
      request->send_P(200, "text/html", customPage);
    }
  }

  uint16_t getId() { return USERMOD_ID_RESERVED; }
};

WLED06Usermod wled06Usermod;
