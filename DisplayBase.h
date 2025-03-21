#ifndef __DISPLAY_BASE_H
#define __DISPLAY_BASE_H

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "Band.h"
#include "DialogBase.h"
#include "IDialogParent.h"
#include "IGuiEvents.h"
#include "Si4735Utils.h"
#include "TftButton.h"
#include "utils.h"

// Képernyőgombok mérete
#define SCRN_BTN_H 35      // Gombok magassága
#define SCRN_BTN_W 75      // Gombok szélessége
#define SCREEN_BTNS_GAP 8  // Gombok közötti gap

// Vizszintes gombok definíciói
#define SCRN_HBTNS_ID_START 25    // A horizontális képernyő menübuttonok kezdő ID-je
#define SCREEN_HBTNS_X_START 5    // Gombok kezdő X koordinátája
#define SCREEN_BUTTONS_PER_ROW 6  // Egy sorban hány gomb van
#define SCREEN_BTN_ROW_SPACING 5  // Gombok sorai közötti távolság

// Vertical gombok definíciói
#define SCRN_VBTNS_ID_START 50       // A vertikális képernyő menübuttonok kezdő ID-je
#define SCREEN_BUTTONS_PER_COLUMN 4  // Egy oszlopban hány gomb van

// A screen button x koordinátájának kiszámítása az 'n' sorszáma alapján
#define SCREEN_BTNS_X(n) (SCREEN_HBTNS_X_START + (SCRN_BTN_W + SCREEN_BTNS_GAP) * n)

// A képernyő változó adatok frissítési ciklusideje
#define SCREEN_COMPS_REFRESH_TIME_MSEC 500

/**
 * DisplayBase base osztály
 */
class DisplayBase : public Si4735Utils, public IGuiEvents, public IDialogParent {

   public:
    // Lehetséges képernyő típusok
    enum DisplayType { none, fm, am, freqScan, screenSaver };

   private:
    // Gombok orientációja
    enum ButtonOrientation { Horizontal, Vertical };

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

    /**
     * Gombok automatikus pozicionálása
     */
    uint16_t getAutoButtonPosition(ButtonOrientation orientation, uint8_t index, bool isX);

    /**
     * Gombok legyártása
     */
    TftButton **buildScreenButtons(ButtonOrientation orientation, BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId, uint8_t &buttonsCount);

    /**
     * Képernyő menügombok legyártása
     */
    inline void buildHorizontalScreenButtons(BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId) {
        horizontalScreenButtons = buildScreenButtons(ButtonOrientation::Horizontal, buttonsData, buttonsDataLength, startId, horizontalScreenButtonsCount);
    }

    /**
     * Képernyő menügombok legyártása (vertikális)
     */
    inline void buildVerticalScreenButtons(BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId) {
        verticalScreenButtons = buildScreenButtons(ButtonOrientation::Vertical, buttonsData, buttonsDataLength, startId, verticalScreenButtonsCount);
    }

    /**
     * Gombok kirajzolása
     */
    void drawButtons(TftButton **buttons, uint8_t buttonsCount);

    /**
     * Képernyő menügombok kirajzolása
     */
    void drawScreenButtons();

    /**
     * Gombok törlése
     */
    void deleteButtons(TftButton **buttons, uint8_t buttonsCount);

    /**
     * Gombok touch eseményének kezelése
     */
    bool handleButtonTouch(TftButton **buttons, uint8_t buttonsCount, bool touched, uint16_t tx, uint16_t ty);

    /**
     * Megkeresi a gombot a label alapján
     * @param label A keresett gomb label-je
     * @return A TftButton pointere, vagy nullptr, ha nincs ilyen gomb
     */
    TftButton *findButtonByLabel(const char *label);

   private:
    /**
     * Megkeresi a gombot a label alapján a megadott tömbben
     * @param buttons A gombok tömbje
     * @param buttonsCount A gombok száma
     * @param label A keresett gomb label-je
     * @return A TftButton pointere, vagy nullptr, ha nincs ilyen gomb
     */
    TftButton *findButtonInArray(TftButton **buttons, uint8_t buttonsCount, const char *label);

   public:
    /**
     * Konstruktor (üres)
     */
    DisplayBase(TFT_eSPI &tft, SI4735 &si4735) : Si4735Utils(si4735), tft(tft), pDialog(nullptr) {}

    /**
     * Destruktor
     */
    virtual ~DisplayBase();

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
    virtual bool loop(RotaryEncoder::EncoderState encoderState) final;
};

// Globális változó az aktuális kijelző váltásának jelzésére (a főprogramban implementálva)
extern DisplayBase::DisplayType newDisplay;

#endif  //__DISPLAY_BASE_H
