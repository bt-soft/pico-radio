#include "FmDisplay.h"

#include <Arduino.h>

#include "MessageDialog.h"
#include "MultiButtonDialog.h"
#include "ValueChangeDialog.h"

/**
 * Konstruktor
 */
FmDisplay::FmDisplay(TFT_eSPI &tft) : DisplayBase(tft) {
    // Képernyőgombok definiálása
    DisplayBase::BuildButtonData buttonsData[] = {
        {"Popup", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"Multi", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //

        {"b-Val", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"i-Val", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"f-Val", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //

        {"Pause", TftButton::ButtonType::Toggleable, TftButton::ButtonState::On},     //
        {"Reset", TftButton::ButtonType::Pushable, TftButton::ButtonState::Disabled}  //
    };

    // Képernyőgombok legyártása
    DisplayBase::buildScreenButtons(buttonsData, ARRAY_ITEM_COUNT(buttonsData), SCRN_MENU_BTN_ID_START);
}

/**
 *
 */
FmDisplay::~FmDisplay() {}

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
bool FmDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) {
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

    return false;
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool FmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) { return false; }

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FmDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {

    DEBUG("FmDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    if (STREQ("Popup", event.label)) {
        // Popup
        DisplayBase::pDialog = new MessageDialog(this, DisplayBase::tft, 280, 130, F("Dialog title"), F("Folytassuk?"), "Aha", "Ne!!");

    } else if (STREQ("Multi", event.label)) {
        // Multi button Dialog
        const char *buttonLabels[] = {"Opt-1", "Opt-2", "Opt-3", "Opt-4", "Opt-5", "Opt-6", "Opt-7", "Opt-8", "Opt-9", "Opt-10", "Opt-11", "Opt-12"};
        int buttonsCount = ARRAY_ITEM_COUNT(buttonLabels);

        DisplayBase::pDialog = new MultiButtonDialog(this, DisplayBase::tft, 400, 180, F("Valasszon opciot!"), buttonLabels, buttonsCount);

    } else if (STREQ("b-Val", event.label)) {
        // b-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("LED state"), F("Value:"), &ledState);

    } else if (STREQ("i-Val", event.label)) {
        // i-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("Volume"), F("Value:"), &volume, 0, 63, 1);

    } else if (STREQ("f-Val", event.label)) {
        // f-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("Temperature"), F("Value:"), &temperature, -15.0, +30.0, 0.05);
    }
}

/**
 * Dialóg Button touch esemény feldolgozása
 */
void FmDisplay::processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {

    DEBUG("FmDisplay::processDialogButtonResponse() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));
    DEBUG("FmDisplay::processDialogButtonResponse() -> ledState: %s, volume: %d, state: %.02f\n", ledState ? "ON" : "OFF", volume, temperature);

    // Töröljük a dialógot
    delete DisplayBase::pDialog;
    DisplayBase::pDialog = nullptr;

    // Újrarajzoljuk a képernyőt
    drawScreen();
};
