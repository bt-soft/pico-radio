#include "FreqScanDisplay.h"

#include <Arduino.h>

/**
 * Konstruktor
 */
FreqScanDisplay::FreqScanDisplay(TFT_eSPI &tft) : DisplayBase(tft) {

    // Képernyőgombok definiálása
    DisplayBase::BuildButtonData buttonsData[] = {
        {"FM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"AM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
    };

    // Képernyőgombok legyártása
    DisplayBase::buildScreenButtons(buttonsData, ARRAY_ITEM_COUNT(buttonsData), SCRN_MENU_BTN_ID_START);
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
