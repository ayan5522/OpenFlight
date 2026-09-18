#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// WIFI
// =====================================================

const char* WIFI_SSID = "AAM";
const char* WIFI_PASSWORD = "qawsedrf";

#define TCP_PORT 5000

WiFiServer server(TCP_PORT);

// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// TELEMETRY DATA
// =====================================================

float altitude = 0;
float airspeed = 0;
float heading = 0;
float verticalSpeed = 0;
float pitch = 0;
float roll = 0;
float throttle = 0;
float flaps = 0;

String gear = "---";

bool telemetryReceived = false;
bool tcpConnected = false;

// =====================================================
// GET VALUE FROM TELEMETRY
// Example:
// ALT:1200,SPD:150,HDG:270
// =====================================================

String getValue(String data, String key) {

  int start = data.indexOf(key);

  if (start == -1) {
    return "";
  }

  start += key.length();

  int end = data.indexOf(",", start);

  if (end == -1) {
    end = data.length();
  }

  return data.substring(start, end);
}

// =====================================================
// PARSE TELEMETRY
// =====================================================

bool parseTelemetry(String data) {

  data.trim();

  if (data.length() == 0) {
    return false;
  }

  String altValue   = getValue(data, "ALT:");
  String spdValue   = getValue(data, "SPD:");
  String hdgValue   = getValue(data, "HDG:");
  String vsValue    = getValue(data, "VS:");
  String pitchValue = getValue(data, "PITCH:");
  String rollValue  = getValue(data, "ROLL:");
  String thrValue   = getValue(data, "THR:");

  String gearValue  = getValue(data, "GEAR:");
  String flapValue  = getValue(data, "FLAPS:");

  // Required fields

  if (
    altValue == "" ||
    spdValue == "" ||
    hdgValue == "" ||
    vsValue == "" ||
    pitchValue == "" ||
    rollValue == "" ||
    thrValue == ""
  ) {

    Serial.println("Invalid telemetry packet");

    return false;
  }

  // Convert values

  altitude      = altValue.toFloat();
  airspeed      = spdValue.toFloat();
  heading       = hdgValue.toFloat();
  verticalSpeed = vsValue.toFloat();
  pitch         = pitchValue.toFloat();
  roll          = rollValue.toFloat();

  // FlightGear throttle is normally 0.0 - 1.0
  throttle = thrValue.toFloat() * 100.0;

  if (gearValue != "") {
    gear = gearValue;
  }

  if (flapValue != "") {
    flaps = flapValue.toFloat();
  }

  telemetryReceived = true;

  return true;
}

// =====================================================
// OLED HEADER
// =====================================================

void drawHeader() {

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("OPENFLIGHT");

  display.setCursor(91, 0);

  if (telemetryReceived) {
    display.print("LIVE");
  } else {
    display.print("WAIT");
  }

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
}

// =====================================================
// DRAW MAIN TELEMETRY
// =====================================================

void drawDashboard() {

  display.clearDisplay();

  drawHeader();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // ---------------------------------------------------
  // ALTITUDE
  // ---------------------------------------------------

  display.setCursor(0, 14);
  display.print("ALT");

  display.setCursor(0, 24);
  display.setTextSize(2);
  display.print((int)altitude);

  display.setTextSize(1);
  display.print(" ft");

  // ---------------------------------------------------
  // AIRSPEED
  // ---------------------------------------------------

  display.setCursor(67, 14);
  display.setTextSize(1);
  display.print("SPD");

  display.setCursor(67, 24);
  display.setTextSize(2);
  display.print((int)airspeed);

  display.setTextSize(1);
  display.print(" kt");

  display.drawLine(0, 39, 127, 39, SSD1306_WHITE);

  // ---------------------------------------------------
  // HEADING
  // ---------------------------------------------------

  display.setCursor(0, 43);
  display.setTextSize(1);
  display.print("HDG ");

  display.setTextSize(1);
  display.print((int)heading);
  display.print((char)247);

  // ---------------------------------------------------
  // VERTICAL SPEED
  // ---------------------------------------------------

  display.setCursor(65, 43);
  display.print("VS ");

  if (verticalSpeed >= 0) {
    display.print("+");
  }

  display.print((int)verticalSpeed);
  display.print(" f");

  display.display();
}

// =====================================================
// DETAILED OLED SCREEN
// =====================================================

void drawDetailedTelemetry() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("OPENFLIGHT");

  display.setCursor(91, 0);
  display.print("LIVE");

  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // Row 1

  display.setCursor(0, 13);
  display.print("ALT:");
  display.print((int)altitude);
  display.print("ft");

  display.setCursor(67, 13);
  display.print("SPD:");
  display.print((int)airspeed);
  display.print("kt");

  // Row 2

  display.setCursor(0, 24);
  display.print("HDG:");
  display.print((int)heading);

  display.setCursor(67, 24);
  display.print("VS:");

  if (verticalSpeed >= 0) {
    display.print("+");
  }

  display.print((int)verticalSpeed);

  // Row 3

  display.setCursor(0, 35);
  display.print("PIT:");
  
  if (pitch >= 0) {
    display.print("+");
  }

  display.print(pitch, 1);

  display.setCursor(67, 35);
  display.print("ROL:");

  if (roll >= 0) {
    display.print("+");
  }

  display.print(roll, 1);

  // Row 4

  display.setCursor(0, 46);
  display.print("THR:");
  display.print((int)throttle);
  display.print("%");

  display.setCursor(67, 46);
  display.print("GEAR:");
  display.print(gear);

  // Row 5

  display.setCursor(0, 57);
  display.print("FLAPS:");
  display.print((int)flaps);
  display.print((char)247);

  display.display();
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("        OPENFLIGHT");
  Serial.println("================================");

  // ===================================================
  // OLED START
  // ===================================================

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("OLED NOT FOUND!");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("OLED Connected!");

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(22, 15);
  display.println("OPEN");

  display.setCursor(22, 35);
  display.println("FLIGHT");

  display.display();

  delay(2000);

  // ===================================================
  // WIFI
  // ===================================================

  Serial.println("Connecting to WiFi...");

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(20, 20);
  display.println("Connecting WiFi...");

  display.display();

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

  Serial.print("TCP Server Started on port ");
  Serial.println(TCP_PORT);

  Serial.println("Waiting for Python...");

  // Show IP on OLED

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("OPENFLIGHT");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.println("WiFi Connected");

  display.setCursor(0, 30);
  display.print("IP:");

  display.println(WiFi.localIP());

  display.setCursor(0, 45);
  display.println("Waiting Python...");

  display.display();

  delay(2000);

  drawDashboard();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  WiFiClient client = server.available();

  if (client) {

    tcpConnected = true;

    Serial.println();
    Serial.println("Python Connected!");

    while (client.connected()) {

      if (client.available()) {

        String data = client.readStringUntil('\n');

        data.trim();

        Serial.print("Received: ");
        Serial.println(data);

        // Parse telemetry

        if (parseTelemetry(data)) {

          Serial.println(
            "Telemetry parsed successfully."
          );

          // Print values for debugging

          Serial.print("ALT: ");
          Serial.println(altitude);

          Serial.print("SPD: ");
          Serial.println(airspeed);

          Serial.print("HDG: ");
          Serial.println(heading);

          Serial.print("VS: ");
          Serial.println(verticalSpeed);

          Serial.print("PITCH: ");
          Serial.println(pitch);

          Serial.print("ROLL: ");
          Serial.println(roll);

          Serial.print("THROTTLE: ");
          Serial.println(throttle);

          // Update OLED

          drawDetailedTelemetry();

          client.println("DISPLAY UPDATED");
        }
      }

      delay(10);
    }

    client.stop();

    tcpConnected = false;

    Serial.println("Python Disconnected");

    drawDashboard();
  }
}