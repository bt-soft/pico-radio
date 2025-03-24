#include "DisplayBase.h"

#include "ValueChangeDialog.h"

// Vízszintes gombok definíciói
#define SCREEN_HBTNS_X_START 5    // Horizontális gombok kezdő X koordinátája
#define SCREEN_HBTNS_Y_MARGIN 5   // Horizontális gombok alsó margója
#define SCREEN_BTN_ROW_SPACING 5  // Gombok sorai közötti távolság

// Vertical gombok definíciói
#define SCREEN_VBTNS_X_MARGIN 0  // A vertikális gombok jobb oldali margója

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
 *
 * @param orientation orientáció (Horizontal/Vertical)
 * @param index hányadik gomb?
 * @param isXpos az X pozíciót számoljuk ki?
 */
uint16_t DisplayBase::getAutoButtonPosition(ButtonOrientation orientation, uint8_t index, bool isXpos) {

    if (orientation == ButtonOrientation::Horizontal) {

        if (isXpos) {
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

    } else {

        // Vertical
        if (isXpos) {
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
 * Képernyő menügombok legyártása (vertikális)
 */
void DisplayBase::buildVerticalScreenButtons(BuildButtonData screenVButtonsData[], uint8_t screenVButtonsDataLength) {

    // Kötelező vertikális Képernyőgombok definiálása
    DisplayBase::BuildButtonData mandatoryVButtons[] = {
        {"Mute", TftButton::ButtonType::Toggleable, TFT_TOGGLE_BUTTON_STATE(rtv::muteStat)},         //
        {"Vol", TftButton::ButtonType::Pushable},                                                    //
        {"AGC", TftButton::ButtonType::Toggleable, TFT_TOGGLE_BUTTON_STATE(si4735.isAgcEnabled())},  //
        {"Att", TftButton::ButtonType::Pushable},                                                    //
        {"Setup", TftButton::ButtonType::Pushable},                                                  //
        //{"Test-1", TftButton::ButtonType::Pushable},                                                  //
        //{"Test-2", TftButton::ButtonType::Pushable},                                                  //
        //{"Test-3", TftButton::ButtonType::Pushable},                                                  //
    };
    // Vertikális képernyőgombok legyártása
    uint8_t mandatoryVButtonsLength = ARRAY_ITEM_COUNT(mandatoryVButtons);

    // Eredmény tömb
    BuildButtonData mergedButtons[mandatoryVButtonsLength + screenVButtonsDataLength];
    uint8_t mergedLength = 0;

    // Tömbök összefűzése
    Utils::mergeArrays(mandatoryVButtons, mandatoryVButtonsLength, screenVButtonsData, screenVButtonsDataLength, mergedButtons, mergedLength);

    // Összefűzött gombok legyártása
    verticalScreenButtons = buildScreenButtons(ButtonOrientation::Vertical, mergedButtons, mergedLength, SCRN_VBTNS_ID_START, verticalScreenButtonsCount);
}

/**
 * Horizontális képernyő menügombok legyártása
 */
void DisplayBase::buildHorizontalScreenButtons(BuildButtonData screenHButtonsData[], uint8_t screenHButtonsDataLength) {

    // Kötelező vertikális Képernyőgombok definiálása
    DisplayBase::BuildButtonData mandatoryHButtons[] = {
        {"Ham", TftButton::ButtonType::Pushable},   //
        {"Band", TftButton::ButtonType::Pushable},  //

        // FM mód esetén nem működik a BW állítás
        {"BndW", TftButton::ButtonType::Pushable, band.currentMode == FM ? TftButton::ButtonState::Disabled : TftButton::ButtonState::Off},
        {"Step", TftButton::ButtonType::Pushable},  //

        {"Scan", TftButton::ButtonType::Pushable},  //
        //{"Test-1", TftButton::ButtonType::Pushable},                                                  //
        //{"Test-2", TftButton::ButtonType::Pushable},                                                  //
        //{"Test-3", TftButton::ButtonType::Pushable},                                                  //
    };
    // Vertikális képernyőgombok legyártása
    uint8_t mandatoryHButtonsLength = ARRAY_ITEM_COUNT(mandatoryHButtons);

    // Eredmény tömb
    BuildButtonData mergedButtons[mandatoryHButtonsLength + screenHButtonsDataLength];
    uint8_t mergedLength = 0;

    // Tömbök összefűzése
    Utils::mergeArrays(mandatoryHButtons, mandatoryHButtonsLength, screenHButtonsData, screenHButtonsDataLength, mergedButtons, mergedLength);

    horizontalScreenButtons = buildScreenButtons(ButtonOrientation::Horizontal, mergedButtons, mergedLength, SCRN_HBTNS_ID_START, horizontalScreenButtonsCount);
}

/**
 *  Minden képernyőn látható közös ()kötelező) gombok eseményeinek kezelése
 */
bool DisplayBase::processMandatoryButtonTouchEvent(TftButton::ButtonTouchEvent &event) {

    bool processed = false;

    //
    //-- Kötelező függőleges gombok vizsgálata
    //
    if (STREQ("Mute", event.label)) {
        // Némítás
        rtv::muteStat = event.state == TftButton::ButtonState::On;
        Si4735Utils::si4735.setAudioMute(rtv::muteStat);
        processed = true;
    } else if (STREQ("Volume", event.label)) {
        // Hangerő állítása
        this->pDialog = new ValueChangeDialog(this, this->tft, 250, 150, F("Volume"), F("Value:"),           //
                                              &config.data.currVolume, (uint8_t)0, (uint8_t)63, (uint8_t)1,  //
                                              [this](uint8_t newValue) { si4735.setVolume(newValue); });
        processed = true;
    } else if (STREQ("AGC", event.label)) {  // Automatikus AGC

        bool stateOn = event.state == TftButton::ButtonState::On;
        config.data.agcGain = stateOn ? static_cast<uint8_t>(Si4735Utils::AgcGainMode::Automatic) : static_cast<uint8_t>(Si4735Utils::AgcGainMode::Off);

        Si4735Utils::checkAGC();

        // Kijelzés frissítése
        DisplayBase::drawAgcAttStatus(true);

        processed = true;

    } else if (STREQ("Att", event.label)) {  // Kézi AGC

        // Kikapcsoljuk az automatikus AGC gombot
        TftButton *agcButton = DisplayBase::findButtonByLabel("AGC");
        if (agcButton != nullptr) {
            agcButton->setState(TftButton::ButtonState::Off);
        }

        // AGCDIS This param selects whether the AGC is enabled or disabled (0 = AGC enabled; 1 = AGC disabled);
        // AGCIDX AGC Index (0 = Minimum attenuation (max gain); 1 – 36 = Intermediate attenuation);
        //  if >greater than 36 - Maximum attenuation (min gain) ).

#define MX_FM_AGC_GAIN 26
#define MX_AM_AGC_GAIN 37
        uint8_t maxValue = si4735.isCurrentTuneFM() ? MX_FM_AGC_GAIN : MX_AM_AGC_GAIN;
        config.data.agcGain = static_cast<uint8_t>(Si4735Utils::AgcGainMode::Manual);  // 2

        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 270, 150, F("RF Attennuator"), F("Value:"),      //
                                                     &config.data.currentAGCgain, (uint8_t)1, (uint8_t)maxValue, (uint8_t)1,  //
                                                     [this](uint8_t currentAGCgain) {
                                                         si4735.setAutomaticGainControl(1, currentAGCgain);
                                                         DisplayBase::drawAgcAttStatus(true);
                                                     });
        processed = true;

    } else if (STREQ("Setup", event.label)) {            // Beállítások
        ::newDisplay = DisplayBase::DisplayType::setup;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!
        processed = true;
    }
    //
    //--- Kötelező vízszintes gombok vizsgálata
    //
    else if (STREQ("Ham", event.label)) {

        // Kigyűjtjük a HAM sávok neveit
        uint8_t hamBandCount;
        const char **hamBands = band.getBandNames(hamBandCount, true);

        // Multi button Dialog
        DisplayBase::pDialog = new MultiButtonDialog(this, DisplayBase::tft, 400, 180, F("HAM Radio Bands"), hamBands, hamBandCount,  //
                                                     [this](TftButton::ButtonTouchEvent event) {
                                                         // Átállítjuk a használni kívánt BAND indexet
                                                         config.data.bandIdx = band.getBandIdxByBandName(event.label);

                                                         // Megkeressük, hogy ez FM vagy AM-e és arra állítjuk a display-t
                                                         BandTable bandRecord = band.getBandByIdx(config.data.bandIdx);
                                                         ::newDisplay = bandRecord.bandType == FM_BAND_TYPE ? DisplayBase::DisplayType::fm : DisplayBase::DisplayType::am;
                                                     });
        processed = true;
    } else if (STREQ("Band", event.label)) {

        // Kigyűjtjük az összes NEM HAM sáv nevét
        uint8_t bandCount;
        const char **bandNames = band.getBandNames(bandCount, false);

        // Multi button Dialog
        DisplayBase::pDialog = new MultiButtonDialog(this, DisplayBase::tft, 400, 250, F("All Radio Bands"), bandNames, bandCount,  //
                                                     [this](TftButton::ButtonTouchEvent event) {
                                                         // Átállítjuk a használni kívánt BAND indexet
                                                         config.data.bandIdx = band.getBandIdxByBandName(event.label);

                                                         // Megkeressük, hogy ez FM vagy AM-e és arra állítjuk a display-t
                                                         BandTable bandRecord = band.getBandByIdx(config.data.bandIdx);
                                                         ::newDisplay = bandRecord.bandType == FM_BAND_TYPE ? DisplayBase::DisplayType::fm : DisplayBase::DisplayType::am;
                                                     });
        processed = true;

    } else if (STREQ("BndW", event.label)) {
        // Megállapítjuk a lehetséges sávszélességek tömbjét
        const __FlashStringHelper *title;
        int bwValuesCount;
        const char **bwValues;
        uint16_t w = 250;
        uint16_t h = 170;

        if (band.currentMode == FM) {
            title = F("FM Filter in kHz");
            bwValues = Band::bandWidthFM;
            bwValuesCount = ARRAY_ITEM_COUNT(Band::bandWidthFM);
        } else if (band.currentMode == AM) {
            title = F("AM Filter in kHz");
            bwValues = Band::bandWidthAM;
            bwValuesCount = ARRAY_ITEM_COUNT(Band::bandWidthAM);
            w = 300;
            h = 180;
        } else {
            title = F("SSB Filter in kHz");
            bwValues = Band::bandWidthSSB;
            bwValuesCount = ARRAY_ITEM_COUNT(Band::bandWidthSSB);
            w = 300;
            h = 150;
        }

        // Multi button Dialog
        DisplayBase::pDialog = new MultiButtonDialog(this, DisplayBase::tft, w, h, title, bwValues, bwValuesCount,  //
                                                     [this](TftButton::ButtonTouchEvent event) {
                                                         // A megnyomott gomb indexe
                                                         uint8_t bwIdx = event.id - DLG_MULTI_BTN_ID_START;

                                                         if (band.currentMode == AM) {
                                                             config.data.bwIdxAM = bwIdx;
                                                         } else if (band.currentMode == FM) {
                                                             config.data.bwIdxFM = bwIdx;
                                                         } else {
                                                             config.data.bwIdxSSB = bwIdx;
                                                         }
                                                         band.BandSet();
                                                     });
        processed = true;

    } else if (STREQ("Step", event.label)) {
        // Megállapítjuk a lehetséges lépések méretét
        const __FlashStringHelper *title;
        int stepCount;
        const char **stepValues;
        uint16_t w = 310;
        uint16_t h = 100;

        if (band.currentMode == FM) {
            title = F("Step tune FM");
            stepValues = Band::stepSizeFM;
            stepCount = ARRAY_ITEM_COUNT(Band::stepSizeFM);
        } else {
            title = F("Step tune AM/SSB");
            stepValues = Band::stepSizeAM;
            stepCount = ARRAY_ITEM_COUNT(Band::stepSizeAM);
            w = 250;
            h = 140;
        }

        DisplayBase::pDialog = new MultiButtonDialog(this, DisplayBase::tft, w, h, title, stepValues, stepCount,  //
                                                     [this](TftButton::ButtonTouchEvent event) {
                                                         // A megnyomott gomb indexe
                                                         uint8_t btnIdx = event.id - DLG_MULTI_BTN_ID_START;

                                                         // Kikeressük az aktuális Band rekordot
                                                         BandTable bandRecord = band.getBandByIdx(config.data.bandIdx);

                                                         // Beállítjuk a konfigban a stepSize-t
                                                         if (bandRecord.bandType == MW_BAND_TYPE or bandRecord.bandType == LW_BAND_TYPE) {
                                                             config.data.ssIdxMW = btnIdx;
                                                         } else if (band.currentMode == FM) {
                                                             config.data.ssIdxFM = btnIdx;
                                                         } else {
                                                             config.data.ssIdxAM = btnIdx;
                                                         }
                                                         Si4735Utils::setStep();
                                                     });
        processed = true;

    } else if (STREQ("Scan", event.label)) {
        // Képernyő váltás !!!
        ::newDisplay = DisplayBase::DisplayType::freqScan;
        processed = true;
    }

    return processed;
}

/**
 * Konstruktor
 */
DisplayBase::DisplayBase(TFT_eSPI &tft, SI4735 &si4735) : Si4735Utils(si4735), tft(tft), pDialog(nullptr) {
    DEBUG("DisplayBase::DisplayBase\n");  //
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

        // Ha a kötelező gombok NEM kezelték le az eseményt, akkor ...
        if (!this->processMandatoryButtonTouchEvent(screenButtonTouchEvent)) {

            // Továbbítjuk a touch eseményt a képernyő gomboknak, hogy ők kezeljék le
            this->processScreenButtonTouchEvent(screenButtonTouchEvent);
        }

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