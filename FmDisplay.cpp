#include "FmDisplay.h"
#include <Arduino.h>

#include "MessageDialog.h"

/**
 * Konstruktor
 */
FmDisplay::FmDisplay(TFT_eSPI &tft) : DisplayBase(tft) {

    // Képernyőgombok definiálása
    DisplayBase::BuildButtonData buttonsData[] = {
        {"Hello", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},
        {"Value", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},
        {"Pause", TftButton::ButtonType::Toggleable, TftButton::ButtonState::On},
        {"Reset", TftButton::ButtonType::Pushable, TftButton::ButtonState::Disabled} //
    };

    // Képernyőgombok legyártása
    DisplayBase::buildScreenButtons(buttonsData, ARRAY_ITEM_COUNT(buttonsData), SCRN_MENU_BTN_ID_START);
}

/**
 *
 */
FmDisplay::~FmDisplay() {
}

/**
 * Képernyő kirajzolása
 * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
 */
void FmDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    // Gombok kirajzolása
    DisplayBase::drawScreenButtons();
}

/**
 * Rotary encoder esemény lekezelése
 */
void FmDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) {
    // Ha klikkeltek VAGY van tekerés, akkor bizony piszkáltuk
    if (encoderState.buttonState != RotaryEncoder::Open or encoderState.direction != RotaryEncoder::Direction::None) {

        if (encoderState.buttonState != RotaryEncoder::Open) {
            switch (encoderState.buttonState) {
            case RotaryEncoder::ButtonState::Pressed:
                DEBUG("Rotary -> Pressed\n");
                break;
            case RotaryEncoder::ButtonState::Held:
                DEBUG("Rotary -> Held\n");
                break;
            case RotaryEncoder::ButtonState::Released:
                DEBUG("Rotary -> Released\n");
                break;
            case RotaryEncoder::ButtonState::Clicked:
                DEBUG("Rotary -> Clicked\n");
                break;
            case RotaryEncoder::ButtonState::DoubleClicked:
                DEBUG("Rotary -> DoubleClicked\n");
                break;
            }
        }

        if (encoderState.direction != RotaryEncoder::Direction::None) {
            switch (encoderState.direction) {
            case RotaryEncoder::Direction::Up:
                DEBUG("Rotary -> UP\n");
                break;
            case RotaryEncoder::Direction::Down:
                DEBUG("Rotary -> DOWN\n");
                break;
            }
        }
    }
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 */
bool FmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {
    return false;
}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FmDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {
    DEBUG("FmDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n",
          event.id, event.label, TftButton::decodeState(event.state));

    if (event.id == SCRN_MENU_BTN_ID_START) {
        DisplayBase::pDialog = new MessageDialog(this, DisplayBase::tft, 300, 150, F("Dialog title"), F("Folytassuk?"), "Aha", "Ne!!");
    }
}

/**
 * Dialóg Button touch esemény feldolgozása
 */
void FmDisplay::processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {
    DEBUG("FmDisplay::processDialogButtonResponse() -> id: %d, label: %s, state: %s\n",
          event.id, event.label, TftButton::decodeState(event.state));

    // Töröljük a dialógot
    delete DisplayBase::pDialog;
    DisplayBase::pDialog = nullptr;

    // Újrarajzoljuk a képernyőt
    drawScreen();
};
