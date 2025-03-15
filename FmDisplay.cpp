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
 * Konstruktor
 */
FmDisplay::FmDisplay(TFT_eSPI &tft) : DisplayBase(tft) {

    // Dinamikusan létrehozzuk a gombokat
    DisplayBase::screenButtonsCount = ARRAY_ITEM_COUNT(SCREEN_BUTTONS_DATA);
    // Kezdő sreenButton ID érték
    uint8_t id = SCRN_MENU_BTN_ID_START;

    // Lefoglaljuk a gombok tömbjét
    DisplayBase::screenButtons = new TftButton *[DisplayBase::screenButtonsCount];

    // Létrehozzuk a gombokat
    for (uint8_t i = 0; i < DisplayBase::screenButtonsCount; i++) {
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
void FmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {
}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FmDisplay::handleScreenButtonTouchEvent(TftButton::ButtonTouchEvent &screenButtonTouchEvent) {
    DEBUG("FmDisplay::handleScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n",
          screenButtonTouchEvent.id, screenButtonTouchEvent.label, TftButton::decodeState(screenButtonTouchEvent.state));
}
