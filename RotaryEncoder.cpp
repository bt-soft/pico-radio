/**
 * Időzítő alapú forgó jeladó
 * Forgó jeladó vezérlő gyorsítással
 * Támogatja a kattintást, dupla kattintást és a hosszan tartó kattintást
 *
 * inspired:  http://www.mikrocontroller.net/articles/Drehgeber
 *
 */

#include "RotaryEncoder.h"

// ----------------------------------------------------------------------------
// Gomb konfiguráció (értékek 1ms-os időzítő szolgáltatás hívásokhoz)
//
#define ENC_BUTTONINTERVAL 10   // gomb ellenőrzése x milliszekundumonként, egyben a pattogásgátló idő is
#define ENC_DOUBLECLICKTIME 600 // második kattintás 600ms-on belül
#define ENC_HOLDTIME 1200       // a gomb lenyomva tartásának jelentése 1.2s után

// ----------------------------------------------------------------------------
// Gyorsulás konfiguráció (1000Hz-es ::service() hívásokhoz)
//
#define ENC_ACCEL_TOP 3072 // max. gyorsulás: *12 (val >> 8)
#define ENC_ACCEL_INC 25
#define ENC_ACCEL_DEC 2

// ----------------------------------------------------------------------------

#if ENC_DECODER != ENC_NORMAL
#ifdef ENC_HALFSTEP
// dekódolási táblázat hibás léptető hardverhez (fél felbontás)
const int8_t ClickEncoder::table[16] __attribute__((__progmem__)) = {
    0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, -1, 0, 0};
#else
// dekódolási táblázat normál hardverhez
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

    if (accelerationEnabled) { // lassítás minden ciklusban
        acceleration -= ENC_ACCEL_DEC;
        if (acceleration & 0x8000) { // MSB be van állítva, túlcsordulás kezelése
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

    if (diff & 1) { // 0. bit = lépés
        last = curr;
        delta += (diff & 2) - 1; // 1. bit = irány (+/-)
        moved = true;
    }
#else
#error "Hiba: definiáld az ENC_DECODER-t ENC_NORMAL-ra vagy ENC_FLAKY-re"
#endif

    if (accelerationEnabled && moved) {
        // növeli a gyorsítót, ha az enkóder el lett mozdítva
        if (acceleration <= (ENC_ACCEL_TOP - ENC_ACCEL_INC)) {
            acceleration += ENC_ACCEL_INC;
        }
    }

    // gomb kezelése
    //
    if (pinBTN > 0                                        // csak akkor ellenőrizzük a gombot, ha megadtuk a lábát
        && (now - lastButtonCheck) >= ENC_BUTTONINTERVAL) // a gomb ellenőrzése elegendő 10-30ms-ként
    {
        lastButtonCheck = now;

        if (digitalRead(pinBTN) == pinsActive) { // a gomb le van nyomva
            keyDownTicks++;
            if (keyDownTicks > (ENC_HOLDTIME / ENC_BUTTONINTERVAL)) {
                buttonState = ButtonState::Held;
            }
        }

        if (digitalRead(pinBTN) == !pinsActive) { // most engedték fel a gombot
            if (keyDownTicks /*> ENC_BUTTONINTERVAL*/) {
                if (buttonState == ButtonState::Held) {
                    buttonState = ButtonState::Released;
                    doubleClickTicks = 0;
                } else {
#define ENC_SINGLECLICKONLY 1
                    if (doubleClickTicks > ENC_SINGLECLICKONLY) { // megakadályozza az aktiválást egyszrű kattintás módban
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
        delta = 0; // alapértelmezés szerint 1 lépés minden beugró ponthoz

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
        buttonState = ButtonState::Open; // visszaállítás
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
