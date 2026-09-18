#ifndef OPENFLIGHT_DISPLAY_H
#define OPENFLIGHT_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

void displayBegin();

/** Phase 4: one-shot static dashboard with sample values. */
void displayDrawStaticDashboard();

Adafruit_ST7789 &displayTft();

#endif
