#include <Arduino.h>
#include <TFT_eSPI.h> // TFT_eSPI könyvtár

#include "DialogBase.h"
#include "IGuiEvents.h"
#include "TftButton.h"
#include "utils.h"

// A képernyő menübuttonok kezdő ID-je
#define SCRN_MENU_BTN_ID_START 50

// Képernyő gomb méret és pozíció definíciók
#define SCREEN_BTNS_X_START 5    // Gombok kezdő X koordinátája
#define SCREEN_BTNS_Y_START 250  // Gombok kezdő Y koordinátája
#define SCRN_BTN_H 30            // Gombok magassága
#define SCRN_BTN_W 70            // Gombok szélessége
#define SCREEN_BTNS_GAP 10       // Gombok közötti gap
#define SCREEN_BUTTONS_PER_ROW 6 // Egy sorban hány gomb van
#define SCREEN_BTN_ROW_SPACING 5 // Gombok sorai közötti távolság

// A screen button x koordinátájának kiszámítása az 'n' sorszáma alapján
#define SCREEN_BTNS_X(n) (SCREEN_BTNS_X_START + (SCRN_BTN_W + SCREEN_BTNS_GAP) * n)

// A képernyő változó adatok frissítési ciklusideje
#define SCREEN_COMPS_REFRESH_TIME_MSEC 500

/**
 *
 */
class DisplayBase : public IGuiEvents {

protected:
    // A lenyomott képernyő menügomb adatai
    TftButton::ButtonTouchEvent buttonTouchEvent = TftButton::noTouchEvent;

    // TFT objektum
    TFT_eSPI &tft;

    // Dinamikusan létrehozott gombok tömbje
    uint8_t screenButtonsCount = 0;
    TftButton **screenButtons = nullptr; // Pointerek tömbjére mutató pointer

    // A képernyőn megjelenő dialog pointere
    DialogBase *pDialog = nullptr;

    /**
     * Screen gombok automatikus X koordinátájának kiszámítása
     * Ha nem férnek el egy sorban a gombok, akkor nyit egy új sort
     */
    uint16_t getAutoX(uint8_t index) {
        uint8_t buttonsPerRow = tft.width() / (SCRN_BTN_W + SCREEN_BTNS_GAP);
        return SCREEN_BTNS_X_START + ((SCRN_BTN_W + SCREEN_BTNS_GAP) * (index % buttonsPerRow));
    }

    /**
     * Screen gombok automatikus Y koordinátájának kiszámítása
     * A gombok több sorban is elhelyezkedhetnek, az alsó sor a képernyő aljához igazodik
     */
    uint16_t getAutoY(uint8_t index) {
        uint8_t row = index / SCREEN_BUTTONS_PER_ROW; // Hányadik sorban van a gomb

        // Teljes gombterület kiszámítása
        uint8_t totalRows = (screenButtonsCount + SCREEN_BUTTONS_PER_ROW - 1) / SCREEN_BUTTONS_PER_ROW;
        uint16_t totalHeight = totalRows * SCRN_BTN_H + (totalRows - 1) * SCREEN_BTN_ROW_SPACING;

        // Első sor pozíciója, hogy az utolsó sor alja a kijelző aljára essen
        uint16_t firstRowY = tft.height() - totalHeight;

        // Az adott sor Y koordinátája
        return firstRowY + row * (SCRN_BTN_H + SCREEN_BTN_ROW_SPACING);
    }

public:
    /**
     * Konstruktor
     */
    DisplayBase(TFT_eSPI &tft) : tft(tft), pDialog(nullptr) {
    }

    /**
     * Destruktor
     */
    virtual ~DisplayBase() {

        // Képernyőgombok törlése
        if (screenButtons) {
            // A TftButton objektumok törlése
            for (int i = 0; i < screenButtonsCount; i++) {
                delete screenButtons[i];
            }
            // A pointerek tömbjének törlése
            delete[] screenButtons;
            screenButtons = nullptr;
        }

        // Dialóg törlése
        if (pDialog) {
            delete pDialog;
            pDialog = nullptr;
        }
    }

    /**
     *
     */
    virtual void drawScreen() = 0;

    /**
     * Arduino loop
     */
    virtual void loop(RotaryEncoder::EncoderState encoderState) final {

        if (encoderState.buttonState != RotaryEncoder::Open or encoderState.direction != RotaryEncoder::Direction::None) {
            // Ha van dialóg, akkor annak passzoljuk a rotary eseményt
            if (pDialog) {
                pDialog->handleRotary(encoderState);
            } else {
                // Ha nincs dialóg, akkor a képernyőnek
                this->handleRotary(encoderState);
            }

            // Egyszerre tekergetni vagy gombot nyomogatni nem lehet a Touch-al, így visszatérhetünk
            return;
        }

        // Megnézzük, hogy nyomkodták-e a képernyőt?
        uint16_t tx, ty;
        bool touched = tft.getTouch(&tx, &ty, 40); // A treshold értékét megnöveljük a default 20msec-ről 40-re

        // Ha van dialóg, akkor annak passzoljuk a touch eseményt
        if (pDialog) {
            pDialog->handleTouch(touched, tx, ty);
        } else {
            // Ha nincs, akkor a képernyőnek
            this->handleTouch(touched, tx, ty);
        }
    }
};