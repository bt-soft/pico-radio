#include <Arduino.h>
#include <TFT_eSPI.h> // TFT_eSPI könyvtár

#include "DialogBase.h"
#include "IDialogParent.h"
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
class DisplayBase : public IGuiEvents, public IDialogParent {

private:
    // A dinamikusan létrehozott gombok tömbjére mutató pointer
    TftButton **screenButtons = nullptr;
    // A dinamikusan létrehozott gombok száma
    uint8_t screenButtonsCount = 0;
    // A lenyomott képernyő menügomb adatai
    TftButton::ButtonTouchEvent screenButtonTouchEvent = TftButton::noTouchEvent;

    // A dialógban megnyomott gomb adatai
    TftButton::ButtonTouchEvent dialogButtonResponse = TftButton::noTouchEvent;

protected:
    // Képernyőgombok legyártását segítő rekord
    struct BuildButtonData {
        const char *label;
        TftButton::ButtonType type;
        TftButton::ButtonState state;
    };

    // TFT objektum
    TFT_eSPI &tft;

    // A képernyőn megjelenő dialog pointere
    DialogBase *pDialog = nullptr;

    /**
     * A dialog által átadott megnyomott gomb adatai
     * Az IDialogParent-ből jön, a dialóg hívja, ha nyomtak rajta valamit
     */
    void setDialogResponse(TftButton::ButtonTouchEvent event) override {
        // A dialogButtonResponse saját másolatot kap, független az eredeti event forrástól, a dialogot lehet törölni
        dialogButtonResponse = event;
    }

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

    /**
     * Képernyő menügombok legyártása
     */
    void buildScreenButtons(BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId) {
        // Dinamikusan létrehozzuk a gombokat
        screenButtonsCount = buttonsDataLength;

        // Lefoglaljuk a gombok tömbjét
        screenButtons = new TftButton *[screenButtonsCount];

        // Létrehozzuk a gombokat
        for (uint8_t i = 0; i < screenButtonsCount; i++) {
            screenButtons[i] = new TftButton(
                startId++,            // A gomb ID-je
                tft,                  // TFT objektum
                getAutoX(i),          // Gomb X koordinátájának kiszámítása
                getAutoY(i),          // Gomb Y koordinátájának kiszámítása
                SCRN_BTN_W,           // Gomb szélessége
                SCRN_BTN_H,           // Gomb magassága
                buttonsData[i].label, // Gomb szövege (label)
                buttonsData[i].type,  // Gomb típusa
                buttonsData[i].state  // Gomb állapota
            );
        }
    }

    /**
     * Képernyő menügombok kirajzolása
     */
    void drawScreenButtons() {

        // Megjelenítjük a képernyő gombokat, ha vannak
        if (screenButtons) {
            for (uint8_t i = 0; i < screenButtonsCount; i++) {
                screenButtons[i]->draw();
            }
        }
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
     * Képernyő kirajzolása
     */
    virtual void drawScreen() = 0;

    /**
     * ScreenButton touch esemény feldolgozása
     */
    virtual void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) = 0;

    /**
     * Dialóg Button touch esemény feldolgozása
     */
    virtual void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) = 0;

    /**
     * Arduino loop hívás
     */
    virtual void loop(RotaryEncoder::EncoderState encoderState) final {

        // Touch adatok változói
        uint16_t tx, ty;
        bool touched = false;

        // Ha van az előző körből feldolgozandó esemény, akkor azzal foglalkozunk először
        if (screenButtonTouchEvent == TftButton::noTouchEvent and dialogButtonResponse == TftButton::noTouchEvent) {

            // Ha nincs feldolgozandó képernyő vagy dialóg gomb esemény, akkor ...

            //
            // Rotary esemény vizsgálata
            //
            if (encoderState.buttonState != RotaryEncoder::Open or encoderState.direction != RotaryEncoder::Direction::None) {
                // Ha van dialóg, akkor annak passzoljuk a rotary eseményt
                if (pDialog) {
                    pDialog->handleRotary(encoderState);
                } else {
                    // Ha nincs dialóg, akkor a leszármazott képernyőnek
                    this->handleRotary(encoderState);
                }

                // Egyszerre tekergetni vagy gombot nyomogatni nem lehet a Touch-al
                // Ha volt rotary esemény, akkor nem lehet touch, így nem megyünk tovább
                return;
            }

            //
            // Touch esemény vizsgálata
            //
            touched = tft.getTouch(&tx, &ty, 40); // A treshold értékét megnöveljük a default 20msec-ről 40-re

            // Ha van dialóg, de még nincs dialogButtonResponse, akkor meghívjuk a dialóg touch handlerét
            if (pDialog != nullptr and dialogButtonResponse == TftButton::noTouchEvent and pDialog->handleTouch(touched, tx, ty)) {

                // Ha ide értünk, akkor be van állítva a dialogButtonResponse
                return;

            } else if (pDialog == nullptr and screenButtonTouchEvent == TftButton::noTouchEvent and screenButtons) {
                // Ha nincs dialóg, de vannak képernyő menügombok és még nincs scrrenButton esemény, akkor azok kapják meg a touch adatokat

                // Elküldjük a touch adatokat a képernyő gomboknak
                for (uint8_t i = 0; i < screenButtonsCount; i++) {

                    // Ha valamelyik viszajelez hogy felengedték, akkor rámozdulunk arra és nem megyünk tovább a többi gombbal
                    if (screenButtons[i]->handleTouch(touched, tx, ty)) {
                        screenButtonTouchEvent = screenButtons[i]->buildButtonTouchEvent();
                        break;
                    }
                }
            }
        }

        // Ha volt screenButton touch event, akkor azt továbbítjuk a képernyőnek
        if (screenButtonTouchEvent != TftButton::noTouchEvent) {
            processScreenButtonTouchEvent(screenButtonTouchEvent);

            // Töröljük a screenButton eseményt
            screenButtonTouchEvent = TftButton::noTouchEvent;

        } else if (dialogButtonResponse != TftButton::noTouchEvent) {
            processDialogButtonResponse(dialogButtonResponse);

            // Töröljük a dialogButtonResponse eseményt
            dialogButtonResponse = TftButton::noTouchEvent;

        } else if (touched) { // Ha nincs screeButton touch event, de nyomtak valamit, akkor azt továbbítjuk a képernyőnek

            handleTouch(touched, tx, ty);
        }
    }
};