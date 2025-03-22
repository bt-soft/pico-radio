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

    // Vertikális Képernyőgombok definiálása
    DisplayBase::BuildButtonData verticalButtonsData[] = {
        {"RDS", TftButton::ButtonType::Toggleable, TFT_TOGGLE_BUTTON_STATE(config.data.rdsEnabled)},  //
        {"Vol", TftButton::ButtonType::Pushable},                                                     //
        {"Mute", TftButton::ButtonType::Toggleable, TftButton::ButtonState::Off},                     //
        {"AGC", TftButton::ButtonType::Toggleable, TFT_TOGGLE_BUTTON_STATE(si4735.isAgcEnabled())},   //
        {"Att", TftButton::ButtonType::Pushable},                                                     //
        {"AntCap", TftButton::ButtonType::Pushable},                                                  //
        {"Bright", TftButton::ButtonType::Pushable},                                                  //
        {"Test-1", TftButton::ButtonType::Pushable},                                                  //
        {"Test-2", TftButton::ButtonType::Pushable},                                                  //
        {"Test-3", TftButton::ButtonType::Pushable},                                                  //
    };
    // Vertikális képernyőgombok legyártása
    DisplayBase::buildVerticalScreenButtons(verticalButtonsData, ARRAY_ITEM_COUNT(verticalButtonsData));

    // Horizontális Képernyőgombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"AM", TftButton::ButtonType::Pushable},     //
        {"Scan", TftButton::ButtonType::Pushable},   //
        {"Popup", TftButton::ButtonType::Pushable},  //
        {"Multi", TftButton::ButtonType::Pushable},  //

        {"b-Val", TftButton::ButtonType::Pushable},  //
        {"i-Val", TftButton::ButtonType::Pushable},  //
        {"f-Val", TftButton::ButtonType::Pushable},  //

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

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FmDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {

    if (STREQ("RDS", event.label)) {

        // Radio Data System
        config.data.rdsEnabled = event.state == TftButton::ButtonState::On;

        if (config.data.rdsEnabled) {
            pRds->displayRds(true);
        } else {
            pRds->clearRds();
        }

    } else if (STREQ("Vol", event.label)) {
        // Hangerő állítása
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 250, 150, F("Volume"), F("Value:"),    //
                                                     &config.data.currVolume, (uint8_t)0, (uint8_t)63, (uint8_t)1,  //
                                                     [this](uint8_t newValue) { si4735.setVolume(newValue); });

    } else if (STREQ("Mute", event.label)) {
        // Némítás
        rtv::muteStat = event.state == TftButton::ButtonState::On;
        si4735.setAudioMute(rtv::muteStat);

    } else if (STREQ("AntCap", event.label)) {

        // If zero, the tuning capacitor value is selected automatically.
        // AM - the tuning capacitance is manually set as 95 fF x ANTCAP + 7 pF.  ANTCAP manual range is 1–6143;
        // FM - the valid range is 0 to 191.

        // Antenna kapacitás állítása
        int maxValue = band.currentMode == FM ? 191 : 6143;
        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 270, 150, F("Antenna Tuning capacitor"), F("Capacitor value:"),  //
                                                     &antCapValue, (int)0, (int)maxValue,                                                     //
                                                     (int)1, [this](int newValue) {
                                                         si4735.setTuneFrequencyAntennaCapacitor(newValue);  //
                                                     });

    } else if (STREQ("AGC", event.label)) {  // Automatikus AGC

        bool stateOn = event.state == TftButton::ButtonState::On;
        config.data.agcGain = stateOn ? static_cast<uint8_t>(Si4735Utils::AgcGainMode::Automatic) : static_cast<uint8_t>(Si4735Utils::AgcGainMode::Off);

        Si4735Utils::checkAGC();

        // Kijelzés frissítése
        DisplayBase::drawAgcAttStatus(true);

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

    } else if (STREQ("Bright", event.label)) {
        DisplayBase::pDialog =
            new ValueChangeDialog(this, DisplayBase::tft, 270, 150, F("TFT Brightness"), F("Value:"),                                                                         //
                                  &config.data.tftBackgroundBrightness, (uint8_t)TFT_BACKGROUND_LED_MIN_BRIGHTNESS, (uint8_t)TFT_BACKGROUND_LED_MAX_BRIGHTNESS, (uint8_t)10,  //
                                  [this](uint8_t newBrightness) { analogWrite(PIN_TFT_BACKGROUND_LED, newBrightness); });

    } else if (STREQ("AM", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::am;

    } else if (STREQ("Scan", event.label)) {
        ::newDisplay = DisplayBase::DisplayType::freqScan;

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
 * Dialóg Button touch esemény feldolgozása
 */
void FmDisplay::processDialogButtonResponse(TftButton::ButtonTouchEvent &event) {

    DEBUG("FmDisplay::processDialogButtonResponse() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    // Töröljük a dialógot
    delete DisplayBase::pDialog;
    DisplayBase::pDialog = nullptr;

    // Újrarajzoljuk a képernyőt
    drawScreen();
};
