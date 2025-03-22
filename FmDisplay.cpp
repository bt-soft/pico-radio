#include "FmDisplay.h"

#include <Arduino.h>

#include "MessageDialog.h"
#include "MultiButtonDialog.h"
#include "ValueChangeDialog.h"

/**
 * Konstruktor
 */
FmDisplay::FmDisplay(TFT_eSPI &tft, SI4735 &si4735) : DisplayBase(tft, si4735), pSMeter(nullptr), pRds(nullptr), pSevenSegmentFreq(nullptr) {

    DEBUG("FmDisplay::FmDisplay\n");

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

    // Minden képernyőn megtalálható gombok generálása
    DisplayBase::buildMandatoryButtons();

    // Horizontális Képernyőgombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"Ham", TftButton::ButtonType::Pushable},                                                     //
        {"RDS", TftButton::ButtonType::Toggleable, TFT_TOGGLE_BUTTON_STATE(config.data.rdsEnabled)},  //
        {"AntCap", TftButton::ButtonType::Pushable},                                                  //
        {"Scan", TftButton::ButtonType::Pushable},                                                    //
        {"AM", TftButton::ButtonType::Pushable},                                                      //

        // //----
        // {"Popup", TftButton::ButtonType::Pushable},  //
        // {"Multi", TftButton::ButtonType::Pushable},
        // //
        // {"b-Val", TftButton::ButtonType::Pushable},  //
        // {"i-Val", TftButton::ButtonType::Pushable},  //
        // {"f-Val", TftButton::ButtonType::Pushable},  //
    };

    // Horizontális képernyőgombok legyártása
    DisplayBase::buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData));
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

    DisplayBase::dawStatusLine();

    // RSSI skála kirajzoltatása
    pSMeter->drawSmeterScale();

    // RSSI aktuális érték
    si4735.getCurrentReceivedSignalQuality();
    uint8_t rssi = si4735.getCurrentRSSI();
    uint8_t snr = si4735.getCurrentSNR();
    pSMeter->showRSSI(rssi, snr, band.currentMode == FM);

    // RDS (erőből a 'valamilyen' adatok megjelenítése)
    if (config.data.rdsEnabled) {
        pRds->displayRds(true);
    }

    // Mono/Stereo aktuális érték
    this->showMonoStereo(si4735.getCurrentPilot());

    // Frekvencia
    float currFreq = band.getBandByIdx(config.data.bandIdx).currentFreq;  // A Rotary változtatásakor már eltettük a Band táblába
    pSevenSegmentFreq->freqDraw(currFreq, 0);

    // Gombok kirajzolása
    DisplayBase::drawScreenButtons();
}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FmDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {

    // Ha a közös gomb volt és azt már lekezeltük, akkor nem megyünk tovább
    if (DisplayBase::processMandatoryButtonTouchEvent(event)) {
        return;
    }

    // Vízszintes gombok vizsgálata
    if (STREQ("Ham", event.label)) {

        // Kigyűjtjük a HAM sávok neveit
        int hamBandCount;
        const char **hamBands = band.getBandNames(hamBandCount, true);

        // Multi button Dialog
        DisplayBase::pDialog = new MultiButtonDialog(this, DisplayBase::tft, 400, 180, F("HAM Radio Bands"), hamBands, hamBandCount);

    } else if (STREQ("RDS", event.label)) {

        // Radio Data System
        config.data.rdsEnabled = event.state == TftButton::ButtonState::On;

        if (config.data.rdsEnabled) {
            pRds->displayRds(true);
        } else {
            pRds->clearRds();
        }

    } else if (STREQ("AntCap", event.label)) {

        // If zero, the tuning capacitor value is selected automatically.
        // AM - the tuning capacitance is manually set as 95 fF x ANTCAP + 7 pF.  ANTCAP manual range is 1–6143;
        // FM - the valid range is 0 to 191.

        // Antenna kapacitás állítása
        int maxValue = band.currentMode == FM ? 191 : 6143;
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 270, 150, F("Antenna Tuning capacitor"), F("Capacitor value:"),  //
                                                     &antCapValue, (int)0, (int)maxValue,                                                     //
                                                     (int)1, [this](int newValue) {
                                                         Si4735Utils::si4735.setTuneFrequencyAntennaCapacitor(newValue);  //
                                                     });

    } else if (STREQ("Scan", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::freqScan;

    } else if (STREQ("AM", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::am;

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
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("LED state"), F("Value:"), &ledState, false, true, false,
                                                     [this](double newValue) { this->ledStateChanged(newValue); });

    } else if (STREQ("i-Val", event.label)) {
        // i-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("Volume"), F("Value:"), &volume, (int)0, (int)63, (int)1);

    } else if (STREQ("f-Val", event.label)) {
        // f-ValueChange
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("Temperature"), F("Value:"), &temperature, (float)-15.0, (float)+30.0, (float)0.05);
    }
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
        if (config.data.rdsEnabled) {
            pRds->showRDS(snr);
        }

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
        pSevenSegmentFreq->freqDraw(currFreq, 0);
        lastFreq = currFreq;
    }
}
