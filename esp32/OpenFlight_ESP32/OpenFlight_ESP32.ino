#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// =====================================================
// WIFI SETTINGS
// =====================================================

const char* WIFI_SSID = "AAM";
const char* WIFI_PASSWORD = "qawsedrf";

// =====================================================
// TCP SERVER
// =====================================================

WiFiServer server(5000);
WiFiClient client;

// =====================================================
// ST7789 PINS
// =====================================================

#define TFT_CS    5
#define TFT_DC    27
#define TFT_RST   26

#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_MISO  19

Adafruit_ST7789 tft = Adafruit_ST7789(
  TFT_CS,
  TFT_DC,
  TFT_RST
);

// =====================================================
// TELEMETRY VARIABLES
// =====================================================

float altitude = 0;
float airspeed = 0;
float heading = 0;
float verticalSpeed = 0;
float pitch = 0;
float roll = 0;
float throttle = 0;
float flaps = 0;

String gear = "UP";
String engine = "OFF";

// =====================================================
// CACHED VALUES
// =====================================================

int oldAltitude = -99999;
int oldAirspeed = -99999;
int oldHeading = -99999;
int oldVerticalSpeed = -99999;
int oldPitch = -99999;
int oldRoll = -99999;
int oldThrottle = -99999;
int oldFlaps = -99999;

String oldGear = "";
String oldEngine = "";

// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

void drawWaitingScreen();
void drawConnectedScreen();
void drawTelemetryUI();
void updateTelemetry();
void parseTelemetry(String data);

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("       OPENFLIGHT ESP32");
  Serial.println("================================");

  // ===================================================
  // TFT INITIALIZATION
  // ===================================================

  SPI.begin(
    TFT_SCLK,
    TFT_MISO,
    TFT_MOSI,
    TFT_CS
  );

  tft.init(240, 320);

  // ===================================================
  // LANDSCAPE MODE
  // ===================================================

  tft.setRotation(3);

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextWrap(false);

  drawWaitingScreen();

  // ===================================================
  // WIFI
  // ===================================================

  Serial.println("Connecting to WiFi...");

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // ===================================================
  // TCP SERVER
  // ===================================================

  server.begin();

  Serial.println("TCP Server Started");
  Serial.println("Port: 5000");
  Serial.println("Waiting for Python...");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // CHECK CLIENT CONNECTION
  // ===================================================

  if (!client || !client.connected()) {

    client = server.available();

    if (client) {

      Serial.println();
      Serial.println("Python Simulator Connected!");

      client.setTimeout(100);

      drawConnectedScreen();

      delay(500);

      // Draw complete UI only once
      drawTelemetryUI();

      // Reset cached values
      oldAltitude = -99999;
      oldAirspeed = -99999;
      oldHeading = -99999;
      oldVerticalSpeed = -99999;
      oldPitch = -99999;
      oldRoll = -99999;
      oldThrottle = -99999;
      oldFlaps = -99999;

      oldGear = "";
      oldEngine = "";
    }
  }

  // ===================================================
  // RECEIVE TELEMETRY
  // ===================================================

  if (client && client.connected()) {

    if (client.available()) {

      String data = client.readStringUntil('\n');

      data.trim();

      if (data.length() > 0) {

        // Uncomment if you want serial telemetry debugging
        // Serial.println(data);

        parseTelemetry(data);

        updateTelemetry();
      }
    }
  }
}

// =====================================================
// WAITING SCREEN
// =====================================================

void drawWaitingScreen() {

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);

  tft.setCursor(85, 80);
  tft.print("OPENFLIGHT");

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(95, 115);
  tft.print("Waiting for");

  tft.setCursor(90, 130);
  tft.print("Python Simulator");

  tft.setTextColor(ST77XX_YELLOW);

  tft.setCursor(125, 165);
  tft.print("TCP : 5000");
}

// =====================================================
// CONNECTED SCREEN
// =====================================================

void drawConnectedScreen() {

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);

  tft.setCursor(105, 90);
  tft.print("CONNECTED");

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(110, 125);
  tft.print("Python Simulator");

  tft.setCursor(130, 145);
  tft.print("ONLINE");
}

// =====================================================
// STATIC TELEMETRY UI
//
// IMPORTANT:
// This function runs only once when Python connects.
// It does NOT run for every telemetry packet.
// =====================================================

void drawTelemetryUI() {

  // Full screen clear ONLY ONCE
  tft.fillScreen(ST77XX_BLACK);

  // ===================================================
  // HEADER
  // ===================================================

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);

  tft.setCursor(105, 8);
  tft.print("OPENFLIGHT");

  tft.drawLine(
    10,
    32,
    310,
    32,
    ST77XX_WHITE
  );

  // ===================================================
  // LEFT COLUMN LABELS
  // ===================================================

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);

  tft.setCursor(15, 48);
  tft.print("ALT");

  tft.setCursor(15, 75);
  tft.print("SPD");

  tft.setCursor(15, 102);
  tft.print("HDG");

  tft.setCursor(15, 129);
  tft.print("V/S");

  tft.setCursor(15, 156);
  tft.print("PITCH");

  tft.setCursor(15, 183);
  tft.print("ROLL");

  tft.setCursor(15, 210);
  tft.print("THR");

  // ===================================================
  // RIGHT COLUMN LABELS
  // ===================================================

  tft.setCursor(170, 48);
  tft.print("GEAR");

  tft.setCursor(170, 102);
  tft.print("FLAP");

  tft.setCursor(170, 156);
  tft.print("ENG");

  // ===================================================
  // UNITS
  // ===================================================

  tft.setTextSize(1);

  tft.setCursor(125, 53);
  tft.print("FT");

  tft.setCursor(125, 80);
  tft.print("KT");

  tft.setCursor(125, 107);
  tft.print("DEG");

  tft.setCursor(125, 134);
  tft.print("FPM");

  tft.setCursor(125, 161);
  tft.print("DEG");

  tft.setCursor(125, 188);
  tft.print("DEG");

  tft.setCursor(125, 215);
  tft.print("%");

  tft.setCursor(270, 107);
  tft.print("DEG");

  // ===================================================
  // STATUS
  // ===================================================

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);

  tft.setCursor(170, 205);
  tft.print("TCP");

  tft.setCursor(170, 220);
  tft.print("LIVE");

  // Connection indicator
  tft.fillCircle(
    220,
    218,
    4,
    ST77XX_GREEN
  );
}

// =====================================================
// UPDATE TELEMETRY
//
// Only small areas are cleared/redrawn.
// NO fillScreen() here.
// =====================================================

void updateTelemetry() {

  // ===================================================
  // ALTITUDE
  // ===================================================

  int newAltitude = (int)altitude;

  if (newAltitude != oldAltitude) {

    tft.fillRect(
      58,
      45,
      65,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(58, 48);
    tft.print(newAltitude);

    oldAltitude = newAltitude;
  }

  // ===================================================
  // AIRSPEED
  // ===================================================

  int newAirspeed = (int)airspeed;

  if (newAirspeed != oldAirspeed) {

    tft.fillRect(
      58,
      72,
      65,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(58, 75);
    tft.print(newAirspeed);

    oldAirspeed = newAirspeed;
  }

  // ===================================================
  // HEADING
  // ===================================================

  int newHeading = (int)heading;

  if (newHeading != oldHeading) {

    tft.fillRect(
      58,
      99,
      65,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(58, 102);
    tft.print(newHeading);

    oldHeading = newHeading;
  }

  // ===================================================
  // VERTICAL SPEED
  // ===================================================

  int newVerticalSpeed = (int)verticalSpeed;

  if (newVerticalSpeed != oldVerticalSpeed) {

    tft.fillRect(
      58,
      126,
      65,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(58, 129);

    if (newVerticalSpeed >= 0) {
      tft.print("+");
    }

    tft.print(newVerticalSpeed);

    oldVerticalSpeed = newVerticalSpeed;
  }

  // ===================================================
  // PITCH
  // ===================================================

  int newPitch = (int)pitch;

  if (newPitch != oldPitch) {

    tft.fillRect(
      58,
      153,
      65,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(58, 156);

    if (newPitch >= 0) {
      tft.print("+");
    }

    tft.print(newPitch);

    oldPitch = newPitch;
  }

  // ===================================================
  // ROLL
  // ===================================================

  int newRoll = (int)roll;

  if (newRoll != oldRoll) {

    tft.fillRect(
      58,
      180,
      65,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(58, 183);

    if (newRoll >= 0) {
      tft.print("+");
    }

    tft.print(newRoll);

    oldRoll = newRoll;
  }

  // ===================================================
  // THROTTLE
  // ===================================================

  int newThrottle = (int)throttle;

  if (newThrottle != oldThrottle) {

    tft.fillRect(
      58,
      207,
      65,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(58, 210);
    tft.print(newThrottle);

    oldThrottle = newThrottle;
  }

  // ===================================================
  // GEAR
  // ===================================================

  if (gear != oldGear) {

    tft.fillRect(
      230,
      45,
      80,
      20,
      ST77XX_BLACK
    );

    tft.setTextSize(2);

    tft.setCursor(230, 48);

    if (gear == "DOWN") {

      tft.setTextColor(ST77XX_GREEN);
      tft.print("DOWN");
    }
    else {

      tft.setTextColor(ST77XX_RED);
      tft.print("UP");
    }

    oldGear = gear;
  }

  // ===================================================
  // FLAPS
  // ===================================================

  int newFlaps = (int)flaps;

  if (newFlaps != oldFlaps) {

    tft.fillRect(
      230,
      99,
      40,
      20,
      ST77XX_BLACK
    );

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    tft.setCursor(230, 102);
    tft.print(newFlaps);

    oldFlaps = newFlaps;
  }

  // ===================================================
  // ENGINE
  // ===================================================

  if (engine != oldEngine) {

    tft.fillRect(
      230,
      153,
      80,
      20,
      ST77XX_BLACK
    );

    tft.setTextSize(2);

    tft.setCursor(230, 156);

    if (engine == "ON") {

      tft.setTextColor(ST77XX_GREEN);
      tft.print("ON");
    }
    else {

      tft.setTextColor(ST77XX_RED);
      tft.print("OFF");
    }

    oldEngine = engine;
  }

  // Restore default text color
  tft.setTextColor(ST77XX_WHITE);
}

// =====================================================
// TELEMETRY PARSER
// =====================================================

void parseTelemetry(String data) {

  int start;
  int end;

  // ===================================================
  // ALTITUDE
  // ===================================================

  start = data.indexOf("ALT=");

  if (start >= 0) {

    start += 4;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    altitude =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // AIRSPEED
  // ===================================================

  start = data.indexOf("SPD=");

  if (start >= 0) {

    start += 4;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    airspeed =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // HEADING
  // ===================================================

  start = data.indexOf("HDG=");

  if (start >= 0) {

    start += 4;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    heading =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // VERTICAL SPEED
  // ===================================================

  start = data.indexOf("VS=");

  if (start >= 0) {

    start += 3;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    verticalSpeed =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // PITCH
  // ===================================================

  start = data.indexOf("PITCH=");

  if (start >= 0) {

    start += 6;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    pitch =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // ROLL
  // ===================================================

  start = data.indexOf("ROLL=");

  if (start >= 0) {

    start += 5;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    roll =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // THROTTLE
  // ===================================================

  start = data.indexOf("THR=");

  if (start >= 0) {

    start += 4;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    throttle =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // GEAR
  // ===================================================

  start = data.indexOf("GEAR=");

  if (start >= 0) {

    start += 5;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    gear =
      data.substring(start, end);
  }

  // ===================================================
  // FLAPS
  // ===================================================

  start = data.indexOf("FLAPS=");

  if (start >= 0) {

    start += 6;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    flaps =
      data.substring(start, end).toFloat();
  }

  // ===================================================
  // ENGINE
  // ===================================================

  start = data.indexOf("ENGINE=");

  if (start >= 0) {

    start += 7;

    end = data.indexOf(",", start);

    if (end < 0)
      end = data.length();

    engine =
      data.substring(start, end);
  }
}