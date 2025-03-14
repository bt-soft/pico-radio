/**
 * Timer-based rotary encoder
 * Rotary Encoder Driver with Acceleration
 * Supports Click, DoubleClick, Long Click
 *
 * inspired:  http://www.mikrocontroller.net/articles/Drehgeber
 *
 */

#include "RotaryEncoder.h"

// ----------------------------------------------------------------------------
// Button configuration (values for 1ms timer service calls)
//
#define ENC_BUTTONINTERVAL 10   // check button every x milliseconds, also debouce time
#define ENC_DOUBLECLICKTIME 600 // second click within 600ms
#define ENC_HOLDTIME 1200       // report held button after 1.2s

// ----------------------------------------------------------------------------
// Acceleration configuration (for 1000Hz calls to ::service())
//
#define ENC_ACCEL_TOP 3072 // max. acceleration: *12 (val >> 8)
#define ENC_ACCEL_INC 25
#define ENC_ACCEL_DEC 2

// ----------------------------------------------------------------------------

#if ENC_DECODER != ENC_NORMAL
#ifdef ENC_HALFSTEP
// decoding table for hardware with flaky notch (half resolution)
const int8_t ClickEncoder::table[16] __attribute__((__progmem__)) = {
    0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, -1, 0, 0};
#else
// decoding table for normal hardware
const int8_t ClickEncoder::table[16] __attribute__((__progmem__)) = {
    0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
#endif
#endif

/**
 * Konstruktor
 */
RotaryEncoder::RotaryEncoder(uint8_t A, uint8_t B, uint8_t BTN, uint8_t stepsPerNotch, bool pinsActive)
    : pinA(A), pinB(B), pinBTN(BTN), steps(stepsPerNotch), pinsActive(pinsActive),
      doubleClickEnabled(true), accelerationEnabled(true), delta(0), last(0), acceleration(0), buttonState(ButtonState::Open) {

    uint8_t mode = (pinsActive == LOW) ? INPUT_PULLUP : INPUT;
    pinMode(pinA, mode);
    pinMode(pinB, mode);
    pinMode(pinBTN, mode);

    if (digitalRead(pinA) == pinsActive) {
        last = 3;
    }

    if (digitalRead(pinB) == pinsActive) {
        last ^= 1;
    }

    oldValue = getValue();
}

/**
 * Megszakításból hívogatjuk 1msec-enként
 */
void RotaryEncoder::service() {
    bool moved = false;
    unsigned long now = millis();

    if (accelerationEnabled) { // decelerate every tick
        acceleration -= ENC_ACCEL_DEC;
        if (acceleration & 0x8000) { // handle overflow of MSB is set
            acceleration = 0;
        }
    }

#if ENC_DECODER == ENC_FLAKY
    last = (last << 2) & 0x0F;

    if (digitalRead(pinA) == pinsActive) {
        last |= 2;
    }

    if (digitalRead(pinB) == pinsActive) {
        last |= 1;
    }

    uint8_t tbl = pgm_read_byte(&table[last]);
    if (tbl) {
        delta += tbl;
        moved = true;
    }
#elif ENC_DECODER == ENC_NORMAL
    int8_t curr = 0;

    if (digitalRead(pinA) == pinsActive) {
        curr = 3;
    }

    if (digitalRead(pinB) == pinsActive) {
        curr ^= 1;
    }

    int8_t diff = last - curr;

    if (diff & 1) { // bit 0 = step
        last = curr;
        delta += (diff & 2) - 1; // bit 1 = direction (+/-)
        moved = true;
    }
#else
#error "Error: define ENC_DECODER to ENC_NORMAL or ENC_FLAKY"
#endif

    if (accelerationEnabled && moved) {
        // increment accelerator if encoder has been moved
        if (acceleration <= (ENC_ACCEL_TOP - ENC_ACCEL_INC)) {
            acceleration += ENC_ACCEL_INC;
        }
    }

    // handle button
    //
    if (pinBTN > 0                                        // check button only, if a pin has been provided
        && (now - lastButtonCheck) >= ENC_BUTTONINTERVAL) // checking button is sufficient every 10-30ms
    {
        lastButtonCheck = now;

        if (digitalRead(pinBTN) == pinsActive) { // key is down
            keyDownTicks++;
            if (keyDownTicks > (ENC_HOLDTIME / ENC_BUTTONINTERVAL)) {
                buttonState = ButtonState::Held;
            }
        }

        if (digitalRead(pinBTN) == !pinsActive) { // key is now up
            if (keyDownTicks /*> ENC_BUTTONINTERVAL*/) {
                if (buttonState == ButtonState::Held) {
                    buttonState = ButtonState::Released;
                    doubleClickTicks = 0;
                } else {
#define ENC_SINGLECLICKONLY 1
                    if (doubleClickTicks > ENC_SINGLECLICKONLY) { // prevent trigger in single click mode
                        if (doubleClickTicks < (ENC_DOUBLECLICKTIME / ENC_BUTTONINTERVAL)) {
                            buttonState = ButtonState::DoubleClicked;
                            doubleClickTicks = 0;
                        }
                    } else {
                        doubleClickTicks = (doubleClickEnabled) ? (ENC_DOUBLECLICKTIME / ENC_BUTTONINTERVAL) : ENC_SINGLECLICKONLY;
                    }
                }
            }

            keyDownTicks = 0;
        }

        if (doubleClickTicks > 0) {
            doubleClickTicks--;
            if (--doubleClickTicks == 0) {
                buttonState = ButtonState::Clicked;
            }
        }
    }
}

/**
 * Tekergetés értékének lekérése
 */
int16_t RotaryEncoder::getValue(void) {
    int16_t val;

    cli();
    val = delta;

    if (steps == 2)
        delta = val & 1;
    else if (steps == 4)
        delta = val & 3;
    else
        delta = 0; // default to 1 step per notch

    sei();

    if (steps == 4)
        val >>= 2;
    if (steps == 2)
        val >>= 1;

    int16_t r = 0;
    int16_t accel = ((accelerationEnabled) ? (acceleration >> 8) : 0);

    if (val < 0) {
        r -= 1 + accel;
    } else if (val > 0) {
        r += 1 + accel;
    }

    return r;
}

#ifndef WITHOUT_BUTTON
/**
 * Gomb állapotának lekérése
 */
RotaryEncoder::ButtonState RotaryEncoder::getButton(void) {
    ButtonState ret = buttonState;
    if (buttonState != ButtonState::Held) {
        buttonState = ButtonState::Open; // reset
    }
    return ret;
}
#endif

// ----------------------------------------------------------------------------
//
// Az enkóder állapotának lekérdezése
//
RotaryEncoder::EncoderState RotaryEncoder::read() {

    EncoderState result = {Direction::None, ButtonState::Open};

    // Gomb állapotának lekérdezése
    result.buttonState = getButton();

    // Forgás detektálása
    if (result.buttonState == ButtonState::Open) { // Tekerni és klikkelni egyszerre nem lehet

        value += getValue();

        if (value / 2 > oldValue) {
            oldValue = value / 2;
            result.direction = Direction::Down;
            // delay(50);
        } else if (value / 2 < oldValue) {
            oldValue = value / 2;
            result.direction = Direction::Up;
            // delay(50);
        }
    }

    return result;
}
