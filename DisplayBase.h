#ifndef __DISPLAY_BASE_H
#define __DISPLAY_BASE_H

#include <Arduino.h>
#include <SI4735.h>
#include <TFT_eSPI.h>

#include "Band.h"
#include "DialogBase.h"
#include "IDialogParent.h"
#include "IGuiEvents.h"
#include "TftButton.h"
#include "utils.h"

// Képernyőgombok mérete
#define SCRN_BTN_H 30       // Gombok magassága
#define SCRN_BTN_W 70       // Gombok szélessége
#define SCREEN_BTNS_GAP 10  // Gombok közötti gap

// Vizszintes gombok definíciói
#define SCRN_HBTNS_ID_START 25    // A horizontális képernyő menübuttonok kezdő ID-je
#define SCREEN_HBTNS_X_START 5    // Gombok kezdő X koordinátája
#define SCREEN_BUTTONS_PER_ROW 6  // Egy sorban hány gomb van
#define SCREEN_BTN_ROW_SPACING 5  // Gombok sorai közötti távolság

// Vertical gombok definíciói
#define SCRN_VBTNS_ID_START 50        // A vertikális képernyő menübuttonok kezdő ID-je
#define SCREEN_VBTNS_X_START 250      // Gombok kezdő X koordinátája (jobb oldal)
#define SCREEN_VBTNS_Y_START 5        // Gombok kezdő Y koordinátája
#define SCREEN_BUTTONS_PER_COLUMN 4   // Egy oszlopban hány gomb van
#define SCREEN_VBTN_COLUMN_SPACING 5  // Gombok oszlopai közötti távolság

// A screen button x koordinátájának kiszámítása az 'n' sorszáma alapján
#define SCREEN_BTNS_X(n) (SCREEN_HBTNS_X_START + (SCRN_BTN_W + SCREEN_BTNS_GAP) * n)

// A képernyő változó adatok frissítési ciklusideje
#define SCREEN_COMPS_REFRESH_TIME_MSEC 500

/**
 * DisplayBase base osztály
 */
class DisplayBase : public IGuiEvents, public IDialogParent {

   public:
    // Lehetséges képernyő típusok
    enum DisplayType { none, fm, am, freqScan, screenSaver };

   private:
    // Vízszintes gombsor
    TftButton **horizontalScreenButtons = nullptr;  // A dinamikusan létrehozott gombok tömbjére mutató pointer
    uint8_t horizontalScreenButtonsCount = 0;       // A dinamikusan létrehozott gombok száma

    // Függőleges gombsor
    TftButton **verticalScreenButtons = nullptr;  // Új: Vertikális gombok tömbje
    uint8_t verticalScreenButtonsCount = 0;       // Új: Vertikális gombok száma

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

    // SI4735
    SI4735 &si4735;

    /**
     * Screen gombok automatikus X koordinátájának kiszámítása
     * Ha nem férnek el egy sorban a gombok, akkor nyit egy új sort
     */
    inline uint16_t getAutoX(uint8_t index) {
        uint8_t buttonsPerRow = tft.width() / (SCRN_BTN_W + SCREEN_BTNS_GAP);
        return SCREEN_HBTNS_X_START + ((SCRN_BTN_W + SCREEN_BTNS_GAP) * (index % buttonsPerRow));
    }

    /**
     * Screen gombok automatikus Y koordinátájának kiszámítása
     * A gombok több sorban is elhelyezkedhetnek, az alsó sor a képernyő aljához igazodik
     */
    inline uint16_t getAutoY(uint8_t index) {
        uint8_t row = index / SCREEN_BUTTONS_PER_ROW;  // Hányadik sorban van a gomb

        // Teljes gombterület kiszámítása
        uint8_t totalRows = (horizontalScreenButtonsCount + SCREEN_BUTTONS_PER_ROW - 1) / SCREEN_BUTTONS_PER_ROW;
        uint16_t totalHeight = totalRows * SCRN_BTN_H + (totalRows - 1) * SCREEN_BTN_ROW_SPACING;

        // Első sor pozíciója, hogy az utolsó sor alja a kijelző aljára essen
        uint16_t firstRowY = tft.height() - totalHeight;

        // Az adott sor Y koordinátája
        return firstRowY + row * (SCRN_BTN_H + SCREEN_BTN_ROW_SPACING);
    }

    /**
     * Vertical Screen gombok automatikus X koordinátájának kiszámítása
     * A gombok a képernyő jobb széléhez igazodnak
     */
    inline uint16_t getAutoVerticalX(uint8_t index) { return tft.width() - SCRN_BTN_W; }

    /**
     * Vertical Screen gombok automatikus Y koordinátájának kiszámítása
     * A gombok több oszlopban is elhelyezkedhetnek, az alsó oszlop a képernyő aljához igazodik
     */
    inline uint16_t getAutoVerticalY(uint8_t index) {
        uint8_t col = index / SCREEN_BUTTONS_PER_COLUMN;  // Hányadik oszlopban van a gomb
        return SCREEN_VBTNS_Y_START + ((SCRN_BTN_H + SCREEN_BTNS_GAP) * (index % SCREEN_BUTTONS_PER_COLUMN));
    }

    /**
     * Képernyő menügombok legyártása
     */
    void buildHorizontalScreenButtons(BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId) {
        // Dinamikusan létrehozzuk a gombokat
        horizontalScreenButtonsCount = buttonsDataLength;

        // Ha nincsenek képernyő gombok, akkor nem megyünk tovább
        if (horizontalScreenButtonsCount == 0) {
            return;
        }

        // Lefoglaljuk a gombok tömbjét
        horizontalScreenButtons = new TftButton *[horizontalScreenButtonsCount];

        // Létrehozzuk a gombokat
        for (uint8_t i = 0; i < horizontalScreenButtonsCount; i++) {
            horizontalScreenButtons[i] = new TftButton(startId++,             // A gomb ID-je
                                                       tft,                   // TFT objektum
                                                       getAutoX(i),           // Gomb X koordinátájának kiszámítása
                                                       getAutoY(i),           // Gomb Y koordinátájának kiszámítása
                                                       SCRN_BTN_W,            // Gomb szélessége
                                                       SCRN_BTN_H,            // Gomb magassága
                                                       buttonsData[i].label,  // Gomb szövege (label)
                                                       buttonsData[i].type,   // Gomb típusa
                                                       buttonsData[i].state   // Gomb állapota
            );
        }
    }

    /**
     * Képernyő menügombok legyártása (vertikális)
     */
    void buildVerticalScreenButtons(BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId) {
        // Dinamikusan létrehozzuk a gombokat
        verticalScreenButtonsCount = buttonsDataLength;

        // Ha nincsenek képernyő gombok, akkor nem megyünk tovább
        if (verticalScreenButtonsCount == 0) {
            return;
        }

        // Lefoglaljuk a gombok tömbjét
        verticalScreenButtons = new TftButton *[verticalScreenButtonsCount];

        // Létrehozzuk a gombokat
        for (uint8_t i = 0; i < verticalScreenButtonsCount; i++) {
            verticalScreenButtons[i] = new TftButton(startId++,             // A gomb ID-je
                                                     tft,                   // TFT objektum
                                                     getAutoVerticalX(i),   // Gomb X koordinátájának kiszámítása
                                                     getAutoVerticalY(i),   // Gomb Y koordinátájának kiszámítása
                                                     SCRN_BTN_W,            // Gomb szélessége
                                                     SCRN_BTN_H,            // Gomb magassága
                                                     buttonsData[i].label,  // Gomb szövege (label)
                                                     buttonsData[i].type,   // Gomb típusa
                                                     buttonsData[i].state   // Gomb állapota
            );
        }
    }

    /**
     * Képernyő menügombok kirajzolása
     */
    inline void drawScreenButtons() {

        // Megjelenítjük a vízszintes képernyő gombokat, ha vannak
        if (horizontalScreenButtons) {
            for (uint8_t i = 0; i < horizontalScreenButtonsCount; i++) {
                horizontalScreenButtons[i]->draw();
            }
        }

        // Megjelenítjük a függőleges képernyő gombokat, ha vannak
        if (verticalScreenButtons) {
            for (uint8_t i = 0; i < verticalScreenButtonsCount; i++) {
                verticalScreenButtons[i]->draw();
            }
        }
    }

   public:
    /**
     * Konstruktor
     */
    DisplayBase(TFT_eSPI &tft, SI4735 &si4735) : tft(tft), si4735(si4735), pDialog(nullptr) {}

    /**
     * Destruktor
     */
    virtual ~DisplayBase() {

        // Képernyőgombok törlése (horizontális)
        if (horizontalScreenButtons) {
            // A TftButton objektumok törlése
            for (int i = 0; i < horizontalScreenButtonsCount; i++) {
                delete horizontalScreenButtons[i];
            }
            // A pointerek tömbjének törlése
            delete[] horizontalScreenButtons;
            horizontalScreenButtons = nullptr;
        }

        // Képernyőgombok törlése (vertikális)
        if (verticalScreenButtons) {
            // A TftButton objektumok törlése
            for (int i = 0; i < verticalScreenButtonsCount; i++) {
                delete verticalScreenButtons[i];
            }
            // A pointerek tömbjének törlése
            delete[] verticalScreenButtons;
            verticalScreenButtons = nullptr;
        }

        // Dialóg törlése
        if (pDialog) {
            delete pDialog;
            pDialog = nullptr;
        }
    }

    /**
     * Aktuális képernyő típusának lekérdezése, implemnetálnia kell a leszármazottnak
     */
    virtual inline DisplayType getDisplayType() = 0;

    /**
     * Dialóg pointer lekérése
     */
    inline DialogBase *getPDialog() { return pDialog; }

    /**
     * A dialog által átadott megnyomott gomb adatai
     * Az IDialogParent-ből jön, a dialóg hívja, ha nyomtak rajta valamit
     */
    inline void setDialogResponse(TftButton::ButtonTouchEvent event) override {
        // A dialogButtonResponse saját másolatot kap, független az eredeti event forrástól, a dialogot lehet törölni
        dialogButtonResponse = event;
    }

    /**
     * Cancelt vagy 'X'-et nyomtak a dialogon?
     */
    inline bool isDialogResponseCancelOrCloseX() override {
        // Ha 'Cancel'-t vagy 'X'-et nyomtak, akkor true-val térünk vissza
        return (dialogButtonResponse.id == DLG_CLOSE_BUTTON_ID or dialogButtonResponse.id == DLG_CANCEL_BUTTON_ID);
    }

    /**
     * Képernyő kirajzolása, implemnetálnia kell a leszármazottnak
     */
    virtual void drawScreen() = 0;

    /**
     * ScreenButton touch esemény feldolgozása
     */
    virtual void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {};

    /**
     * Dialóg Button touch esemény feldolgozása
     */
    virtual void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {};

    /**
     * Esemény nélküli display loop -> Adatok periódikus megjelenítése, implemnetálnia kell a leszármazottnak
     */
    virtual void displayLoop() = 0;

    /**
     * Arduino loop hívás (a leszármazott nem írhatja felül)
     * @return true -> ha volt valalami touch vagy rotary esemény kezelés, a screensavert resetelni kell ilyenkor
     */
    virtual bool loop(RotaryEncoder::EncoderState encoderState) final {

        // Touch adatok változói
        uint16_t tx, ty;
        bool touched = false;

        // Ha van az előző körből feldolgozandó esemény, akkor azzal foglalkozunk először
        if (screenButtonTouchEvent == TftButton::noTouchEvent and dialogButtonResponse == TftButton::noTouchEvent) {

            // Ha nincs feldolgozandó képernyő vagy dialóg gomb esemény, akkor ...

            //
            // Rotary esemény vizsgálata (ha nem tekergetik vagy nem nyomogatják, akkor nem reagálunk rá)
            //
            if (encoderState.buttonState != RotaryEncoder::Open or encoderState.direction != RotaryEncoder::Direction::None) {

                // Ha van dialóg, akkor annak passzoljuk a rotary eseményt, de csak ha van esemény
                if (pDialog) {
                    pDialog->handleRotary(encoderState);
                } else {
                    // Ha nincs dialóg, akkor a leszármazott képernyőnek, de csak ha van esemény
                    this->handleRotary(encoderState);  // Az IGuiEvents interfészből
                }

                // Egyszerre tekergetni vagy gombot nyomogatni nem lehet a Touch-al
                // Ha volt rotary esemény, akkor nem lehet touch, így nem megyünk tovább
                return true;
            }

            //
            // Touch esemény vizsgálata
            //
            touched = tft.getTouch(&tx, &ty, 40);  // A treshold értékét megnöveljük a default 20msec-ről 40-re

            // Ha van dialóg, de még nincs dialogButtonResponse, akkor meghívjuk a dialóg touch handlerét
            if (pDialog != nullptr and dialogButtonResponse == TftButton::noTouchEvent and pDialog->handleTouch(touched, tx, ty)) {

                // Ha ide értünk, akkor be van állítva a dialogButtonResponse
                return true;

            } else if (pDialog == nullptr and screenButtonTouchEvent == TftButton::noTouchEvent) {
                // Ha nincs dialóg, de vannak képernyő gombok és még nincs scrrenButton esemény, akkor azok kapják meg a touch adatokat

                // Elküldjük a touch adatokat a függőleges gomboknak
                if (verticalScreenButtons) {
                    for (uint8_t i = 0; i < verticalScreenButtonsCount; i++) {

                        // Ha valamelyik viszajelez hogy felengedték, akkor rámozdulunk arra és nem megyünk tovább a többi gombbal
                        if (verticalScreenButtons[i]->handleTouch(touched, tx, ty)) {
                            screenButtonTouchEvent = verticalScreenButtons[i]->buildButtonTouchEvent();
                            break;
                        }
                    }
                }

                // Elküldjük a touch adatokat a vízszintes gomboknak ha még nics a függőleges gomboktól esemény
                if (horizontalScreenButtons and screenButtonTouchEvent == TftButton::noTouchEvent) {
                    for (uint8_t i = 0; i < horizontalScreenButtonsCount; i++) {

                        // Ha valamelyik viszajelez hogy felengedték, akkor rámozdulunk arra és nem megyünk tovább a többi gombbal
                        if (horizontalScreenButtons[i]->handleTouch(touched, tx, ty)) {
                            screenButtonTouchEvent = horizontalScreenButtons[i]->buildButtonTouchEvent();
                            break;
                        }
                    }
                }
            }
        }

        // Ha volt screenButton touch event, akkor azt továbbítjuk a képernyőnek
        if (screenButtonTouchEvent != TftButton::noTouchEvent) {
            this->processScreenButtonTouchEvent(screenButtonTouchEvent);

            // Töröljük a screenButton eseményt
            screenButtonTouchEvent = TftButton::noTouchEvent;

        } else if (dialogButtonResponse != TftButton::noTouchEvent) {

            // Volt dialóg touch response, feldolgozzuk
            this->processDialogButtonResponse(dialogButtonResponse);

            // Töröljük a dialogButtonResponse eseményt
            dialogButtonResponse = TftButton::noTouchEvent;

        } else if (touched) {
            // Ha nincs screeButton touch event, de nyomtak valamit a képernyőn

            this->handleTouch(touched, tx, ty);  // Az IGuiEvents interfészből

        } else {
            // Semmilyen touch esemény nem volt, meghívjuk a képernyő loop-ját
            this->displayLoop();
        }

        return touched;
    }
};

// Globális változó az aktuális kijelző váltásának jelzésére (a főprogramban implementálva)
extern DisplayBase::DisplayType newDisplay;

#endif  //__DISPLAY_BASE_H