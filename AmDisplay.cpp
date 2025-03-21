#include "AmDisplay.h"

#include <Arduino.h>

/**
 * Konstruktor
 */
AmDisplay::AmDisplay(TFT_eSPI &tft, SI4735 &si4735) : DisplayBase(tft, si4735) {

    // Képernyőgombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"FM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},    //
        {"Scan", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
    };

    // Képernyőgombok legyártása
    DisplayBase::buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData), SCRN_MENU_BTN_ID_START);
}

/**
 * Destruktor
 */
AmDisplay::~AmDisplay() {}

/**
 * Rotary encoder esemény lekezelése
 */
bool AmDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) { return false; }

/**
 * Képernyő kirajzolása
 * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
 */
void AmDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    // Gombok kirajzolása
    DisplayBase::drawScreenButtons();

    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);  // Középre igazítás
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("AM display", tft.width() / 2, tft.height() / 2);
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool AmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) { return false; }

/**
 * Esemény nélküli display loop -> Adatok periódikus megjelenítése
 */
void AmDisplay::displayLoop() {}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void AmDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {
    DEBUG("AmDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    if (STREQ("FM", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::fm;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!

    } else if (STREQ("Scan", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::freqScan;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!
    }
}

/**
 * Dialóg Button touch esemény feldolgozása
 */
void AmDisplay::processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {}
