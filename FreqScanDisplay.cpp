#include "FreqScanDisplay.h"

#include <Arduino.h>

/**
 * Konstruktor
 */
FreqScanDisplay::FreqScanDisplay(TFT_eSPI &tft, SI4735 &si4735) : DisplayBase(tft, si4735) {

    // Horizontális képernyőgombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"FM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"AM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
    };

    // Horizontális képernyőgombok legyártása
    DisplayBase::buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData));
}

/**
 * Destruktor
 */
FreqScanDisplay::~FreqScanDisplay() {}

/**
 * Rotary encoder esemény lekezelése
 */
bool FreqScanDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) { return false; }

/**
 * Képernyő kirajzolása
 * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
 */
void FreqScanDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    // Gombok kirajzolása
    DisplayBase::drawScreenButtons();

    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);  // Középre igazítás
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("FreqScan display", tft.width() / 2, tft.height() / 2);
}

/**
 * Esemény nélküli display loop -> Adatok periódikus megjelenítése
 */
void FreqScanDisplay::displayLoop() {}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool FreqScanDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) { return false; }

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FreqScanDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {

    DEBUG("FreqScanDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    if (STREQ("FM", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::fm;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!

    } else if (STREQ("AM", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::am;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!
    }
}

/**
 * Dialóg Button touch esemény feldolgozása
 */
void FreqScanDisplay::processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {}
