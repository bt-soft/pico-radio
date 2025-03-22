#include "SetupDisplay.h"

/**
 * Konstruktor
 */
SetupDisplay::SetupDisplay(TFT_eSPI &tft, SI4735 &si4735) : DisplayBase(tft, si4735) {

    // Horizontális képernyőgombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"Return", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
    };

    // Horizontális képernyőgombok legyártása
    DisplayBase::buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData));
}

/**
 * Destruktor
 */
SetupDisplay::~SetupDisplay() {}

/**
 * Képernyő kirajzolása
 * A ScreenSaver rögtön a képernyő törlésével kezd
 */
void SetupDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    // Gombok kirajzolása
    DisplayBase::drawScreenButtons();

    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);  // Középre igazítás
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Setup display", tft.width() / 2, tft.height() / 2);
}

/**
 * Esemény nélküli display loop - ScreenSaver futtatása
 * Nem kell figyelni a touch eseményt, azt már a főprogram figyeli és leállítja/törli a ScreenSaver-t
 */
void SetupDisplay::displayLoop() {}

/**
 * Rotary encoder esemény lekezelése
 */
bool SetupDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) { return false; }

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool SetupDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) { return false; }

/**
 * Képernyő menügomb esemény feldolgozása
 */
void SetupDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {

    DEBUG("SetupDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    if (STREQ("Return", event.label)) {
        ::newDisplay = prevDisplay;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!
    }
}
