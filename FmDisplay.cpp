#include "FmDisplay.h"
#include <Arduino.h>

struct ButtonData {
    const char *label;
    TftButton::ButtonType type;
    TftButton::ButtonState state;
};

ButtonData SCREEN_BUTTONS_DATA[] = {
    {"Start", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},
    {"Stop", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},
    {"Pause", TftButton::ButtonType::Toggleable, TftButton::ButtonState::On},
    {"Reset", TftButton::ButtonType::Pushable, TftButton::ButtonState::Disabled} //
};

/**
 *
 */
FmDisplay::FmDisplay(TFT_eSPI &tft) : DisplayBase(tft) {
#define FM_SCRN_BTNS_CNT 1

    // Dinamikusan létrehozzuk a gombokat
    DisplayBase::screenButtonsCount = ARRAY_ITEM_COUNT(SCREEN_BUTTONS_DATA);
    uint8_t id = SCRN_MENU_BTN_ID_START; // Kezdő sreenButton ID érték

    DisplayBase::screenButtons = new TftButton *[FM_SCRN_BTNS_CNT]; // Lefoglaljuk a gombok tömbjét

    for (uint8_t i = 0; i < DisplayBase::screenButtonsCount; i++) { // Létrehozzuk a gombokat
        DisplayBase::screenButtons[i] = new TftButton(
            id++,                         // A gomb ID-je
            tft,                          // TFT objektum
            DisplayBase::getAutoX(i),     // X koordináta
            DisplayBase::getAutoY(i),     // Y koordináta
            SCRN_BTN_W,                   // Gomb szélessége
            SCRN_BTN_H,                   // Gomb magassága
            SCREEN_BUTTONS_DATA[i].label, // Gomb szövege (label)
            SCREEN_BUTTONS_DATA[i].type,  // Gomb típusa
            SCREEN_BUTTONS_DATA[i].state  // Gomb állapota
        );
    }
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

    // Megjelenítjük a képernyő gombokat, ha vannak
    if (DisplayBase::screenButtons) {
        for (uint8_t i = 0; i < DisplayBase::screenButtonsCount; i++) {
            DisplayBase::screenButtons[i]->draw();
        }
    }
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
 * Touch esemény lekezelése
 */
void FmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {

    // Töröljük  a korábban lenyomott gomb adatait
    if (DisplayBase::buttonTouchEvent != TftButton::noTouchEvent) {
        DisplayBase::buttonTouchEvent = TftButton::noTouchEvent;
    }

    // Elküldjük a touch eseményt a képernyő gomboknak
    if (DisplayBase::screenButtons) {
        for (uint8_t i = 0; i < DisplayBase::screenButtonsCount; i++) {

            // Ha valamelyik viszajelez hogy felengedték, akkor rámozdulunk
            TftButton::ButtonTouchEvent touchEvent = DisplayBase::screenButtons[i]->handleTouch(touched, tx, ty);
            if (touchEvent != TftButton::noTouchEvent) {
                DisplayBase::buttonTouchEvent = touchEvent;
                break;
            }
        }
    }

    if (DisplayBase::buttonTouchEvent != TftButton::noTouchEvent) {
        DEBUG("Button Pressed: id->%d, label->%s, state->%s\n",
              DisplayBase::buttonTouchEvent.id, DisplayBase::buttonTouchEvent.label, TftButton::decodeState(DisplayBase::buttonTouchEvent.state));
    }
}
