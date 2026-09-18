#include "display.h"

#include <SPI.h>

#include "config.h"

namespace {

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

constexpr int16_t SCREEN_W = 320;
constexpr int16_t SCREEN_H = 240;

constexpr int16_t HEADER_H = 40;
constexpr int16_t FOOTER_H = 22;
constexpr int16_t GRID_TOP = HEADER_H + 2;
constexpr int16_t GRID_BOTTOM = SCREEN_H - FOOTER_H;
constexpr int16_t COL_SPLIT = 160;
constexpr int16_t ROW_H = (GRID_BOTTOM - GRID_TOP) / 4;

constexpr uint16_t COL_BG = ST77XX_BLACK;
constexpr uint16_t COL_TITLE = ST77XX_CYAN;
constexpr uint16_t COL_SUBTITLE = ST77XX_WHITE;
constexpr uint16_t COL_LABEL = ST77XX_YELLOW;
constexpr uint16_t COL_VALUE = ST77XX_WHITE;
constexpr uint16_t COL_DIM = 0x8410;  // gray
constexpr uint16_t COL_LINE = 0x4208;
constexpr uint16_t COL_OK = ST77XX_GREEN;
constexpr uint16_t COL_WARN = ST77XX_YELLOW;
constexpr uint16_t COL_VS_POS = ST77XX_GREEN;
constexpr uint16_t COL_VS_NEG = ST77XX_RED;

void drawHeader() {
  tft.fillRect(0, 0, SCREEN_W, HEADER_H, COL_BG);

  tft.setTextSize(2);
  tft.setTextColor(COL_TITLE);
  tft.setCursor(78, 6);
  tft.print(F("OPENFLIGHT"));

  tft.setTextSize(1);
  tft.setTextColor(COL_SUBTITLE);
  tft.setCursor(96, 28);
  tft.print(F("FLIGHT TELEMETRY"));

  tft.fillCircle(222, 32, 3, COL_OK);
  tft.setTextColor(COL_OK);
  tft.setCursor(230, 28);
  tft.print(F("LIVE"));

  tft.drawFastHLine(0, HEADER_H, SCREEN_W, COL_LINE);
}

void drawFooterStatic() {
  const int16_t y = SCREEN_H - FOOTER_H;
  tft.fillRect(0, y, SCREEN_W, FOOTER_H, COL_BG);
  tft.drawFastHLine(0, y, SCREEN_W, COL_LINE);

  tft.setTextSize(1);

  auto statusDot = [&](int16_t dx, uint16_t color) {
    tft.fillCircle(dx, y + 11, 2, color);
  };

  tft.setTextColor(COL_SUBTITLE);
  tft.setCursor(4, y + 7);
  tft.print(F("Wi-Fi"));
  statusDot(34, COL_OK);
  tft.print(F(" CONN"));

  tft.setCursor(88, y + 7);
  tft.print(F("TCP"));
  statusDot(110, COL_OK);
  tft.print(F(" CONN"));

  tft.setCursor(164, y + 7);
  tft.print(F("FG"));
  statusDot(180, COL_OK);
  tft.print(F(" LIVE"));
}

void drawGridLines() {
  tft.drawFastVLine(COL_SPLIT, GRID_TOP, GRID_BOTTOM - GRID_TOP, COL_LINE);
  for (int i = 1; i < 4; ++i) {
    const int16_t y = GRID_TOP + i * ROW_H;
    tft.drawFastHLine(0, y, SCREEN_W, COL_LINE);
  }
}

void drawCell(int16_t row, int16_t col, const char *label, const char *value,
              uint16_t valueColor) {
  const int16_t x = (col == 0) ? 6 : COL_SPLIT + 6;
  const int16_t y = GRID_TOP + row * ROW_H;

  tft.setTextSize(1);
  tft.setTextColor(COL_LABEL);
  tft.setCursor(x, y + 6);
  tft.print(label);

  tft.setTextSize(2);
  tft.setTextColor(valueColor);
  tft.setCursor(x, y + 20);
  tft.print(value);
}

void drawStateCell(int16_t row, int16_t col, const char *label,
                   const char *line1, const char *line2) {
  const int16_t x = (col == 0) ? 6 : COL_SPLIT + 6;
  const int16_t y = GRID_TOP + row * ROW_H;

  tft.setTextSize(1);
  tft.setTextColor(COL_LABEL);
  tft.setCursor(x, y + 6);
  tft.print(label);

  tft.setTextSize(2);
  tft.setTextColor(COL_VALUE);
  tft.setCursor(x, y + 20);
  tft.print(line1);

  tft.setTextSize(1);
  tft.setTextColor(COL_DIM);
  tft.setCursor(x, y + 38);
  tft.print(line2);
}

}  // namespace

void displayBegin() {
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.init(240, 320);
  tft.setRotation(1);
  tft.fillScreen(COL_BG);
}

void displayDrawStaticDashboard() {
  tft.fillScreen(COL_BG);
  drawHeader();
  drawGridLines();

  drawCell(0, 0, "ALTITUDE", "12,450 ft", COL_VALUE);
  drawCell(0, 1, "AIRSPEED", "245 kt", COL_VALUE);

  drawCell(1, 0, "HEADING", "273", COL_VALUE);
  drawCell(1, 1, "VERTICAL SPEED", "+650 fpm", COL_VS_POS);

  drawCell(2, 0, "PITCH", "+3.2", COL_VALUE);
  drawCell(2, 1, "ROLL", "-8.4", COL_VALUE);

  drawCell(3, 0, "THROTTLE", "72%", COL_VALUE);
  drawStateCell(3, 1, "AIRCRAFT STATE", "GEAR UP", "FLAPS 5");

  drawFooterStatic();
}

Adafruit_ST7789 &displayTft() { return tft; }
