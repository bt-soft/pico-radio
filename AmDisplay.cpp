#include "AmDisplay.h"

#include <Arduino.h>

/**
 * Konstruktor
 */
AmDisplay::AmDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band) : DisplayBase(tft, si4735, band) {

    DEBUG("AmDisplay::AmDisplay\n");

    // SMeter példányosítása
    pSMeter = new SMeter(tft, 0, 80);

    // Frekvencia kijelzés pédányosítása
    pSevenSegmentFreq = new SevenSegmentFreq(tft, rtv::freqDispX, rtv::freqDispY, band);

    // Függőleges gombok legyártása, nincs saját függőleges gombsor
    DisplayBase::buildVerticalScreenButtons(nullptr, 0);

    // Horizontális képernyőgombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"AntC", TftButton::ButtonType::Pushable},  //
    };

    // Horizontális képernyőgombok legyártása
    DisplayBase::buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData));
}

/**
 * Destruktor
 */
AmDisplay::~AmDisplay() {
    // SMeter trölése
    if (pSMeter) {
        delete pSMeter;
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
void AmDisplay::drawScreen() {
    tft.setFreeFont();
    tft.fillScreen(TFT_COLOR_BACKGROUND);

    DisplayBase::dawStatusLine();

    // RSSI skála kirajzoltatása
    pSMeter->drawSmeterScale();

    // RSSI aktuális érték
    si4735.getCurrentReceivedSignalQuality();
    uint8_t rssi = si4735.getCurrentRSSI();
    uint8_t snr = si4735.getCurrentSNR();
    pSMeter->showRSSI(rssi, snr, band.getCurrentBand().varData.currMod == FM);

    // Frekvencia
    float currFreq = band.getCurrentBand().varData.currFreq;  // A Rotary változtatásakor már eltettük a Band táblába
    pSevenSegmentFreq->freqDispl(currFreq);

    // Gombok kirajzolása
    DisplayBase::drawScreenButtons();
}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void AmDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {
    DEBUG("AmDisplay::processScreenButtonTouchEvent() -> id: %d, label: %s, state: %s\n", event.id, event.label, TftButton::decodeState(event.state));

    if (STREQ("AntC", event.label)) {
        // If zero, the tuning capacitor value is selected automatically.
        // AM - the tuning capacitance is manually set as 95 fF x ANTCAP + 7 pF.  ANTCAP manual range is 1–6143;
        // FM - the valid range is 0 to 191.

        // Antenna kapacitás állítása
        int maxValue = band.getCurrentBand().varData.currMod == FM ? 191 : 6143;

        int antCapValue = 0;

        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 270, 150, F("Antenna Tuning capacitor"), F("Capacitor value:"),  //
                                                     &antCapValue, (int)0, (int)maxValue,                                                     //
                                                     (int)1, [this](int newValue) {
                                                         Si4735Utils::si4735.setTuneFrequencyAntennaCapacitor(newValue);  //
                                                     });
    }
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool AmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {

    uint8_t currMod = band.getCurrentBand().varData.currMod;

    // A frekvencia kijelző kezeli a touch eseményeket SSB/CW módban
    if (currMod == LSB or currMod == USB or currMod == CW) {
        return pSevenSegmentFreq->handleTouch(touched, tx, ty);
    }

    return false;
}

/**
 * Rotary encoder esemény lekezelése
 */
bool AmDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) {

    BandTable &currentBand = band.getCurrentBand();
    uint8_t currMod = currentBand.varData.currMod;

    if (currMod == LSB or currMod == USB or currMod == CW) {

        // rtv::freqDec = rtv::freqDec - (encoderState.direction == RotaryEncoder::Direction::Up ? rtv::freqstep : -rtv::freqstep);
        // rtv::freqDec = constrain(rtv::freqDec, -16000, 16000);  // Tartomány korlátozás

        // int freqTot = (si4735.getFrequency() * 1000) + (rtv::freqDec * -1);

        // // Frekvencia korlátozás
        // freqTot = constrain(freqTot, currentBand.pConstData->minimumFreq * 1000, currentBand.pConstData->maximumFreq * 1000);

        // // Zajszűrés: az SSB frekvenciaváltás huppogásának elkerülésére
        // Si4735Utils::hardwareAudioMuteOn();

        // // Egész számra kerekítés
        // freqTot = freqTot / 1000;

        // // A frekvenciát beállítjuk
        // si4735.setFrequency(freqTot);

        uint16_t currentFrequency = si4735.getFrequency();

        if (encoderState.direction == RotaryEncoder::Direction::Up) {
            rtv::freqDec = rtv::freqDec - rtv::freqstep;
            int freqTot = (si4735.getFrequency() * 1000) + (rtv::freqDec * -1);
            if (freqTot > (currentBand.pConstData->maximumFreq * 1000)) {
                si4735.setFrequency(currentBand.pConstData->maximumFreq);
                rtv::freqDec = 0;
            }

            int freqPlus16;
            if (rtv::freqDec <= -16000) {
                rtv::freqDec = rtv::freqDec + 16000;
                freqPlus16 = currentFrequency + 16;
                Si4735Utils::hardwareAudioMuteOn();
                si4735.setFrequency(freqPlus16);
            }
            config.data.currentBFO = rtv::freqDec;

            DEBUG("AmDisplay::handleRotary -> freqTot: %d, rtv::freqDec: %d, currentFrequency: %d, freqPlus16: %d\n", freqTot, rtv::freqDec, currentFrequency, freqPlus16);

        } else {
            rtv::freqDec = rtv::freqDec + rtv::freqstep;
            int freqTot = (si4735.getFrequency() * 1000) - rtv::freqDec;
            if (freqTot < (currentBand.pConstData->minimumFreq * 1000)) {
                si4735.setFrequency(currentBand.pConstData->minimumFreq);
                rtv::freqDec = 0;
            }

            int freqMin16;
            if (rtv::freqDec >= 16000) {
                rtv::freqDec = rtv::freqDec - 16000;
                freqMin16 = currentFrequency - 16;
                Si4735Utils::hardwareAudioMuteOn();
                si4735.setFrequency(freqMin16);
            }
            config.data.currentBFO = rtv::freqDec;

            DEBUG("AmDisplay::handleRotary -> freqTot: %d, rtv::freqDec: %d, currentFrequency: %d, freqMin16: %d\n", freqTot, rtv::freqDec, currentFrequency, freqMin16);
        }

        currentBand.varData.lastBFO = config.data.currentBFO;
        checkAGC();

    } else {
        (encoderState.direction == RotaryEncoder::Direction::Up) ? si4735.frequencyUp() : si4735.frequencyDown();
    }

    // Elmentjük a beállított frekvenciát a Band táblába
    currentBand.varData.currFreq = si4735.getFrequency();

    return true;
}

/**
 * Esemény nélküli display loop -> Adatok periódikus megjelenítése
 */
void AmDisplay::displayLoop() {

    // Ha van dialóg, akkor nem frissítjük a komponenseket
    if (DisplayBase::pDialog != nullptr) {
        return;
    }

    BandTable &currentBand = band.getCurrentBand();

    // Néhány adatot csak ritkábban frissítünk
    static uint32_t elapsedTimedValues = 0;  // Kezdőérték nulla
    if ((millis() - elapsedTimedValues) >= SCREEN_COMPS_REFRESH_TIME_MSEC) {

        // RSSI
        si4735.getCurrentReceivedSignalQuality();
        uint8_t rssi = si4735.getCurrentRSSI();
        uint8_t snr = si4735.getCurrentSNR();
        pSMeter->showRSSI(rssi, snr, currentBand.varData.currMod == FM);

        // Frissítjük az időbélyeget
        elapsedTimedValues = millis();
    }

    // A Frekvenciát azonnal frissítjuk, de csak ha változott
    static uint16_t lastFreq = 0;
    uint16_t currFreq = currentBand.varData.currFreq;  // A Rotary változtatásakor már eltettük a Band táblába
    if (lastFreq != currFreq) {
        pSevenSegmentFreq->freqDispl(currFreq);
        lastFreq = currFreq;
    }
}
