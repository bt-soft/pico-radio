#include "FmDisplay.h"

#include <Arduino.h>

#include "MessageDialog.h"
#include "MultiButtonDialog.h"
#include "ValueChangeDialog.h"

/**
 * Konstruktor
 */
FmDisplay::FmDisplay(TFT_eSPI &tft, SI4735 &si4735) : DisplayBase(tft, si4735), pSMeter(nullptr), pRds(nullptr), pSevenSegmentFreq(nullptr) {

    // SMeter példányosítása
    pSMeter = new SMeter(tft, 0, 80);

    // RDS példányosítása
    pRds = new Rds(tft, si4735, 80, 62,  // Station x,y
                   0, 80,                // Message x,y
                   2, 42,                // Time x,y
                   0, 140                // program type x,y
    );

    // Frekvencia kijelzés pédányosítása
    pSevenSegmentFreq = new SevenSegmentFreq(tft, rtv::freqDispX, rtv::freqDispY);

    // Képernyőgombok definiálása
    DisplayBase::BuildButtonData buttonsData[] = {
        {"AM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},     //
        {"Scan", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},   //
        {"Popup", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"Multi", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //

        {"b-Val", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"i-Val", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"f-Val", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //

        {"Pause", TftButton::ButtonType::Toggleable, TftButton::ButtonState::On},     //
        {"Reset", TftButton::ButtonType::Pushable, TftButton::ButtonState::Disabled}  //
    };

    // Képernyőgombok legyártása
    DisplayBase::buildScreenButtons(buttonsData, ARRAY_ITEM_COUNT(buttonsData), SCRN_MENU_BTN_ID_START);
}

/**
 *
 */
FmDisplay::~FmDisplay() {

    // SMeter trölése
    if (pSMeter) {
        delete pSMeter;
    }

    // RDS trölése
    if (pRds) {
        delete pRds;
    }

    // Frekvencia kijelző törlése
    if (pSevenSegmentFreq) {
        delete pSevenSegmentFreq;
    }
}

/**
 * Képernyő kirajzolása
 * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
 */
void FmDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_COLOR_BACKGROUND);

    // RSSI skála kirajzoltatása
    pSMeter->drawSmeterScale();

    // RSSI aktuális érték
    si4735.getCurrentReceivedSignalQuality();
    uint8_t rssi = si4735.getCurrentRSSI();
    uint8_t snr = si4735.getCurrentSNR();
    pSMeter->showRSSI(rssi, snr, band.currentMode == FM);

    // RDS (erőből a 'valamilyen' adatok megjelenítése)
    pRds->displayRds(true);

    // Mono/Stereo aktuális érték
    this->showMonoStereo(si4735.getCurrentPilot());

    // Frekvencia
    float currFreq = band.getBandByIdx(config.data.bandIdx).currentFreq;  // A Rotary változtatásakor már eltettük a Band táblába
    pSevenSegmentFreq->FreqDraw(currFreq, 0);

    // Gombok kirajzolása
    DisplayBase::drawScreenButtons();
}

/**
 * Rotary encoder esemény lekezelése
 */
bool FmDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) {

    switch (encoderState.direction) {
        case RotaryEncoder::Direction::Up:
            si4735.frequencyUp();
            break;
        case RotaryEncoder::Direction::Down:
            si4735.frequencyDown();
            break;
    }

    // Elmentjük a band táblába az aktuális frekvencia értékét
    band.getBandByIdx(config.data.bandIdx).currentFreq = si4735.getFrequency();

    // RDS törlés
    pRds->clearRds();

    return true;
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool FmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) { return false; }

/**
 * Mono/Stereo vétel megjelenítése
 */
void FmDisplay::showMonoStereo(bool stereo) {

    // STEREO/MONO háttér
    uint32_t backGroundColor = stereo ? TFT_RED : TFT_BLUE;
    tft.fillRect(rtv::freqDispX + 191, rtv::freqDispY + 60, 38, 12, backGroundColor);

    // Felirat
    tft.setFreeFont();
    tft.setTextColor(TFT_WHITE, backGroundColor);
    tft.setTextSize(1);
    tft.setTextDatum(BC_DATUM);
    tft.setTextPadding(0);
    char buffer[10];  // Useful to handle string
    sprintf(buffer, "%s", stereo ? "STEREO" : "MONO");
    tft.drawString(buffer, rtv::freqDispX + 210, rtv::freqDispY + 71);
}

/**
 * Esemény nélküli display loop -> Adatok periódikus megjelenítése
 */
void FmDisplay::displayLoop() {

    // Ha van dialóg, akkor nem frissítjük a komponenseket
    if (DisplayBase::pDialog != nullptr) {
        return;
    }

    // Néhány adatot csak ritkábban frissítünk
    static long elapsedTimedValues = 0;  // Kezdőérték nulla
    if ((millis() - elapsedTimedValues) >= SCREEN_COMPS_REFRESH_TIME_MSEC) {

        // RSSI
        si4735.getCurrentReceivedSignalQuality();
        uint8_t rssi = si4735.getCurrentRSSI();
        uint8_t snr = si4735.getCurrentSNR();
        pSMeter->showRSSI(rssi, snr, band.currentMode == FM);

        // RDS
        pRds->showRDS(snr);

        // Mono/Stereo
        static bool prevStereo = false;
        bool stereo = si4735.getCurrentPilot();
        // Ha változott, akkor frissítünk
        if (stereo != prevStereo) {
            this->showMonoStereo(stereo);
            prevStereo = stereo;  // Frissítsük az előző értéket
        }

        // Frissítjük az időbélyeget
        elapsedTimedValues = millis();
    }

    // A Freqkvenciát azonnal frisítjuk, de csak ha változott
    static float lastFreq = 0;
    float currFreq = band.getBandByIdx(config.data.bandIdx).currentFreq;  // A Rotary változtatásakor már eltettük a Band táblába
    if (lastFreq != currFreq) {
        pSevenSegmentFreq->FreqDraw(currFreq, 0);
        lastFreq = currFreq;
    }
}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FmDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {

    DEBUG("FmDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    if (STREQ("AM", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::am;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!

    } else if (STREQ("Scan", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::freqScan;  // <<<--- ITT HÍVJUK MEG A changeDisplay-t!

    } else if (STREQ("Popup", event.label)) {
        // Popup
        DisplayBase::pDialog = new MessageDialog(this, DisplayBase::tft, 280, 130, F("Dialog title"), F("Folytassuk?"), "Aha", "Ne!!");

    } else if (STREQ("Multi", event.label)) {
        // Multi button Dialog
        const char *buttonLabels[] = {"Opt-1", "Opt-2", "Opt-3", "Opt-4", "Opt-5", "Opt-6", "Opt-7", "Opt-8", "Opt-9", "Opt-10", "Opt-11", "Opt-12"};
        int buttonsCount = ARRAY_ITEM_COUNT(buttonLabels);

        DisplayBase::pDialog = new MultiButtonDialog(this, DisplayBase::tft, 400, 180, F("Valasszon opciot!"), buttonLabels, buttonsCount);

    } else if (STREQ("b-Val", event.label)) {
        // b-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("LED state"), F("Value:"), &ledState);

    } else if (STREQ("i-Val", event.label)) {
        // i-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("Volume"), F("Value:"), &volume, 0, 63, 1);

    } else if (STREQ("f-Val", event.label)) {
        // f-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("Temperature"), F("Value:"), &temperature, -15.0, +30.0, 0.05);
    }
}

/**
 * Dialóg Button touch esemény feldolgozása
 */
void FmDisplay::processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {

    DEBUG("FmDisplay::processDialogButtonResponse() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));
    DEBUG("FmDisplay::processDialogButtonResponse() -> ledState: %s, volume: %d, state: %.02f\n", ledState ? "ON" : "OFF", volume, temperature);

    // Töröljük a dialógot
    delete DisplayBase::pDialog;
    DisplayBase::pDialog = nullptr;

    // Újrarajzoljuk a képernyőt
    drawScreen();
};
