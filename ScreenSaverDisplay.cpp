#include "ScreenSaverDisplay.h"

#include <Arduino.h>

/**
 * Konstruktor
 */
ScreenSaverDisplay::ScreenSaverDisplay(TFT_eSPI &tft) : DisplayBase(tft) {}

/**
 * Destruktor
 */
ScreenSaverDisplay::~ScreenSaverDisplay() {}

/**
 * Rotary encoder esemény lekezelése
 */
bool ScreenSaverDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) { return false; }

/**
 * Képernyő kirajzolása
 * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
 */
void ScreenSaverDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);  // Középre igazítás
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("ScreenSaver display", tft.width() / 2, tft.height() / 2);
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool ScreenSaverDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) { return false; }

/**
 * Képernyő menügomb esemény feldolgozása
 */
void ScreenSaverDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {}

/**
 * Dialóg Button touch esemény feldolgozása
 */
void ScreenSaverDisplay::processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {}
