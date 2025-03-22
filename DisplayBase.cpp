#include "DisplayBase.h"

// Vízszintes gombok definíciói
#define SCREEN_HBTNS_X_START 5    // Horizontális gombok kezdő X koordinátája
#define SCREEN_HBTNS_Y_MARGIN 5   // Horizontális gombok alsó margója
#define SCREEN_BTN_ROW_SPACING 5  // Gombok sorai közötti távolság

// Vertical gombok definíciói
#define SCREEN_VBTNS_X_MARGIN 5  // A vertikális gombok jobb oldali margója

/**
 *  BFO Status kirajzolása
 */
void DisplayBase::drawBfoStatus(bool initFont) {

    // Fontot kell váltani?
    if (initFont) {
        tft.setFreeFont();
        tft.setTextSize(1);
        tft.setTextDatum(BC_DATUM);
    }

    // BFO Step
    uint16_t bfoStepColor = TFT_SILVER;
    if ((band.currentMode == LSB or band.currentMode == USB or band.currentMode == CW) and config.data.currentBFOmanu) {
        bfoStepColor = TFT_ORANGE;
    }
    tft.setTextColor(bfoStepColor, TFT_BLACK);
    if (rtv::bfoOn) {
#ifdef IhaveCrystal
        tft.drawString(String(config.data.currentBFOStep) + " Hz", 20, 15);
#endif
    } else {
        tft.drawString(F(" BFO "), 20, 15);
    }
    tft.drawRect(0, 2, 39, 16, bfoStepColor);
}

/**
 * AGC / ATT Status kirajzolása
 */
void DisplayBase::drawAgcAttStatus(bool initFont) {

    // Fontot kell váltani?
    if (initFont) {
        tft.setFreeFont();
        tft.setTextSize(1);
        tft.setTextDatum(BC_DATUM);
    }

    // AGC / ATT
    uint16_t agcColor = config.data.agcGain == 0 ? TFT_SILVER : TFT_COLOR(255, 130, 0);
    tft.setTextColor(agcColor, TFT_BLACK);
    if (config.data.agcGain > 1) {
        tft.drawString("ATT" + String(config.data.currentAGCgain < 9 ? " " : "") + String(config.data.currentAGCgain), 60, 15);
    } else {
        tft.drawString(F(" AGC "), 60, 15);
    }
    tft.drawRect(40, 2, 39, 16, agcColor);
}

/**
 * Státusz a képernyő tetején
 */
void DisplayBase::dawStatusLine() {
    tft.fillRect(0, 0, 240, 16, TFT_COLOR_BACKGROUND);  // Teljes statusline törlése

    tft.setFreeFont();
    tft.setTextSize(1);
    tft.setTextDatum(BC_DATUM);

    // BFO Step
    drawBfoStatus();

    // AGC Status
    drawAgcAttStatus();

// Band MODE
#define TFT_COLOR_STATUSLINE_MODE TFT_YELLOW
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    String modtext = band.getCurrentBandModeDesc();
    if ((modtext == "USB") and (rtv::CWShift == true)) {
        modtext = "CW";
    }
    tft.drawString(modtext, 95, 15);
    tft.drawRect(80, 2, 29, 16, TFT_COLOR_STATUSLINE_MODE);

// BANDWIDTH
#define TFT_COLOR_STATUSLINE_BANDW TFT_COLOR(255, 127, 255)  // magenta?
    tft.setTextColor(TFT_COLOR_STATUSLINE_BANDW, TFT_BLACK);

    String bwText = band.getCurrentBandWithPstr();
    if (bwText == "AUTO") {
        tft.drawString("F AUTO", 135, 15);
    } else {
        tft.drawString("F" + bwText + "KHz", 135, 15);
    }
    tft.drawRect(110, 2, 49, 16, TFT_COLOR_STATUSLINE_BANDW);

// BAND NAME
#define TFT_COLOR_STATUSLINE_BAND TFT_CYAN
    tft.setTextColor(TFT_COLOR_STATUSLINE_BAND, TFT_BLACK);
    tft.drawString(band.getBandByIdx(config.data.bandIdx).bandName, 180, 15);
    tft.drawRect(160, 2, 39, 16, TFT_COLOR_STATUSLINE_BAND);

// STEP
#define TFT_COLOR_STATUSLINE_STEP TFT_SKYBLUE
    tft.setTextColor(TFT_COLOR_STATUSLINE_STEP, TFT_BLACK);
    uint8_t currentStep = band.getBandByIdx(config.data.bandIdx).currentStep;
    tft.drawString(String(currentStep * (band.currentMode == FM ? 10 : 1)) + "kHz", 220, 15);
    tft.drawRect(200, 2, 39, 16, TFT_COLOR_STATUSLINE_STEP);
}

/**
 * Gombok automatikus pozicionálása
 */
uint16_t DisplayBase::getAutoButtonPosition(ButtonOrientation orientation, uint8_t index, bool isX) {

    if (orientation == ButtonOrientation::Horizontal) {
        if (isX) {
            uint8_t buttonsPerRow = tft.width() / (SCRN_BTN_W + SCREEN_BTNS_GAP);
            return SCREEN_HBTNS_X_START + ((SCRN_BTN_W + SCREEN_BTNS_GAP) * (index % buttonsPerRow));
        } else {
            uint8_t buttonsPerRow = tft.width() / (SCRN_BTN_W + SCREEN_BTNS_GAP);
            uint8_t row = index / buttonsPerRow;                                                               // Hányadik sorban van a gomb
            uint8_t rowCount = (horizontalScreenButtonsCount + buttonsPerRow - 1) / buttonsPerRow;             // Összes sor száma
            uint16_t totalHeight = rowCount * (SCRN_BTN_H + SCREEN_BTN_ROW_SPACING) - SCREEN_BTN_ROW_SPACING;  // Az összes sor magassága
            uint16_t startY = tft.height() - totalHeight - SCREEN_HBTNS_Y_MARGIN;                              // A legalsó sor kezdő Y koordinátája
            return startY + row * (SCRN_BTN_H + SCREEN_BTN_ROW_SPACING);                                       // Az adott sor Y koordinátája
        }
    } else {  // Vertical
        if (isX) {
            // Új X koordináta számítás
            uint8_t buttonsPerColumn = tft.height() / (SCRN_BTN_H + SCREEN_BTNS_GAP);
            uint8_t requiredColumns = (verticalScreenButtonsCount + buttonsPerColumn - 1) / buttonsPerColumn;
            uint8_t column = index / buttonsPerColumn;

            // Ha több oszlop van, akkor az oszlopokat jobbra igazítjuk
            if (requiredColumns > 1) {
                // Ha az utolsó oszlopban vagyunk, akkor a képernyő jobb oldalához igazítjuk
                if (column == requiredColumns - 1) {
                    return tft.width() - SCREEN_VBTNS_X_MARGIN - SCRN_BTN_W;
                } else {
                    // Ha nem az utolsó oszlopban vagyunk, akkor a következő oszlop bal oldalához igazítjuk, gap-el
                    return tft.width() - SCREEN_VBTNS_X_MARGIN - SCRN_BTN_W - ((requiredColumns - column - 1) * (SCRN_BTN_W + SCREEN_BTNS_GAP));
                }
            } else {
                // Ha csak egy oszlop van, akkor a jobb oldalra igazítjuk
                return tft.width() - SCREEN_VBTNS_X_MARGIN - SCRN_BTN_W;
            }
        } else {
            // Új Y koordináta számítás
            uint8_t buttonsPerColumn = tft.height() / (SCRN_BTN_H + SCREEN_BTNS_GAP);
            uint8_t row = index % buttonsPerColumn;
            return row * (SCRN_BTN_H + SCREEN_BTNS_GAP);
        }
    }
}

/**
 * Gombok legyártása
 */
TftButton **DisplayBase::buildScreenButtons(ButtonOrientation orientation, BuildButtonData buttonsData[], uint8_t buttonsDataLength, uint8_t startId, uint8_t &buttonsCount) {
    // Dinamikusan létrehozzuk a gombokat
    buttonsCount = buttonsDataLength;

    // Ha nincsenek képernyő gombok, akkor nem megyünk tovább
    if (buttonsCount == 0) {
        return nullptr;
    }

    // Lefoglaljuk a gombok tömbjét
    TftButton **screenButtons = new TftButton *[buttonsCount];

    // Létrehozzuk a gombokat
    for (uint8_t i = 0; i < buttonsCount; i++) {
        screenButtons[i] = new TftButton(startId++,                                     // A gomb ID-je
                                         tft,                                           // TFT objektum
                                         getAutoButtonPosition(orientation, i, true),   // Gomb X koordinátájának kiszámítása
                                         getAutoButtonPosition(orientation, i, false),  // Gomb Y koordinátájának kiszámítása
                                         SCRN_BTN_W,                                    // Gomb szélessége
                                         SCRN_BTN_H,                                    // Gomb magassága
                                         buttonsData[i].label,                          // Gomb szövege (label)
                                         buttonsData[i].type,                           // Gomb típusa
                                         buttonsData[i].state                           // Gomb állapota
        );
    }
    return screenButtons;
}

/**
 * Gombok kirajzolása
 */
void DisplayBase::drawButtons(TftButton **buttons, uint8_t buttonsCount) {
    if (buttons) {
        for (uint8_t i = 0; i < buttonsCount; i++) {
            buttons[i]->draw();
        }
    }
}

/**
 * Képernyő gombok kirajzolása
 */
void DisplayBase::drawScreenButtons() {
    drawButtons(horizontalScreenButtons, horizontalScreenButtonsCount);
    drawButtons(verticalScreenButtons, verticalScreenButtonsCount);
}

/**
 * Gombok törlése
 */
void DisplayBase::deleteButtons(TftButton **buttons, uint8_t buttonsCount) {
    if (buttons) {
        for (int i = 0; i < buttonsCount; i++) {
            delete buttons[i];
        }
        delete[] buttons;
    }
}

/**
 * Gombok touch eseményének kezelése
 */
bool DisplayBase::handleButtonTouch(TftButton **buttons, uint8_t buttonsCount, bool touched, uint16_t tx, uint16_t ty) {
    if (buttons) {
        for (uint8_t i = 0; i < buttonsCount; i++) {
            if (buttons[i]->handleTouch(touched, tx, ty)) {
                screenButtonTouchEvent = buttons[i]->buildButtonTouchEvent();
                return true;
            }
        }
    }
    return false;
}

/**
 * Megkeresi a gombot a label alapján a megadott tömbben
 *
 * @param buttons A gombok tömbje
 * @param buttonsCount A gombok száma
 * @param label A keresett gomb label-je
 * @return A TftButton pointere, vagy nullptr, ha nincs ilyen gomb
 */
TftButton *DisplayBase::findButtonInArray(TftButton **buttons, uint8_t buttonsCount, const char *label) {

    if (buttons == nullptr || label == nullptr) {
        return nullptr;
    }

    for (uint8_t i = 0; i < buttonsCount; ++i) {
        if (buttons[i] != nullptr && STREQ(buttons[i]->getLabel(), label)) {
            return buttons[i];
        }
    }

    return nullptr;
}

/**
 * Megkeresi a gombot a label alapján
 *
 * @param label A keresett gomb label-je
 * @return A TftButton pointere, vagy nullptr, ha nincs ilyen gomb
 */
TftButton *DisplayBase::findButtonByLabel(const char *label) {
    // Először a horizontális gombok között keresünk
    TftButton *button = findButtonInArray(horizontalScreenButtons, horizontalScreenButtonsCount, label);
    if (button != nullptr) {
        return button;
    }

    // Ha nem találtuk meg, akkor a vertikális gombok között keresünk
    return findButtonInArray(verticalScreenButtons, verticalScreenButtonsCount, label);
}

/**
 * Destruktor
 */
DisplayBase::~DisplayBase() {

    deleteButtons(horizontalScreenButtons, horizontalScreenButtonsCount);
    horizontalScreenButtons = nullptr;
    horizontalScreenButtonsCount = 0;

    deleteButtons(verticalScreenButtons, verticalScreenButtonsCount);
    verticalScreenButtons = nullptr;
    verticalScreenButtonsCount = 0;

    // Dialóg törlése, ha van
    if (pDialog) {
        delete pDialog;
        pDialog = nullptr;
    }
}

/**
 * Arduino loop hívás (a leszármazott nem írhatja felül)
 *
 * @param encoderState enkóder állapot
 * @return true -> ha volt valalami touch vagy rotary esemény kezelés, a screensavert resetelni kell ilyenkor
 */
bool DisplayBase::loop(RotaryEncoder::EncoderState encoderState) {

    // Az ős loop hívása a squelch kezelésére
    Si4735Utils::loop();

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
            if (handleButtonTouch(verticalScreenButtons, verticalScreenButtonsCount, touched, tx, ty)) {
                // Ha volt esemény a függőleges gombokon, akkor nem vizsgáljuk a vízszintes gombokat
            } else {
                // Elküldjük a touch adatokat a vízszintes gomboknak
                handleButtonTouch(horizontalScreenButtons, horizontalScreenButtonsCount, touched, tx, ty);
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