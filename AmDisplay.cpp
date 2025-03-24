#include "AmDisplay.h"

#include <Arduino.h>

/**
 * Konstruktor
 */
AmDisplay::AmDisplay(TFT_eSPI &tft, SI4735 &si4735) : DisplayBase(tft, si4735) {

    DEBUG("AmDisplay::AmDisplay\n");

    // SMeter példányosítása
    pSMeter = new SMeter(tft, 0, 80);

    // Frekvencia kijelzés pédányosítása
    pSevenSegmentFreq = new SevenSegmentFreq(tft, rtv::freqDispX, rtv::freqDispY);

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
    pSMeter->showRSSI(rssi, snr, band.currentMode == FM);

    // Frekvencia
    float currFreq = band.getBandByIdx(config.data.bandIdx).currentFreq;  // A Rotary változtatásakor már eltettük a Band táblába
    pSevenSegmentFreq->freqDraw(currFreq, 0);

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
        int maxValue = band.currentMode == FM ? 191 : 6143;

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
bool AmDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) { return false; }

/**
 * Rotary encoder esemény lekezelése
 */
bool AmDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) {

    BandTable currentBand = band.getBandByIdx(config.data.bandIdx);

    if (band.currentMode == LSB or band.currentMode == USB or band.currentMode == CW) {

        if (encoderState.direction == RotaryEncoder::Direction::Up) {
            rtv::freqDec = rtv::freqDec - rtv::freqstep;
            int freqTot = (si4735.getFrequency() * 1000) + (rtv::freqDec * -1);
            if (freqTot > (currentBand.maximumFreq * 1000)) {
                si4735.setFrequency(currentBand.maximumFreq);
                rtv::freqDec = 0;
            }
            if (rtv::freqDec <= -16000) {
                rtv::freqDec = rtv::freqDec + 16000;
                int freqPlus16 = config.data.currentFreq + 16;
                ////////////////////////////////////////////////                MuteAudOn();
                si4735.setFrequency(freqPlus16);
            }
            config.data.currentBFO = rtv::freqDec;

        } else {
            rtv::freqDec = rtv::freqDec + rtv::freqstep;
            int freqTot = (si4735.getFrequency() * 1000) - rtv::freqDec;
            if (freqTot < (currentBand.minimumFreq * 1000)) {
                si4735.setFrequency(currentBand.minimumFreq);
                rtv::freqDec = 0;
            }

            if (rtv::freqDec >= 16000) {
                rtv::freqDec = rtv::freqDec - 16000;
                int freqMin16 = config.data.currentFreq - 16;
                ////////////////////////////////////////////////                MuteAudOn();
                si4735.setFrequency(freqMin16);
            }

            config.data.currentBFO = rtv::freqDec;
        }
        currentBand.lastBFO = config.data.currentBFO;
        checkAGC();

    } else {
        switch (encoderState.direction) {
            case RotaryEncoder::Direction::Up:
                si4735.frequencyUp();
                break;
            case RotaryEncoder::Direction::Down:
                si4735.frequencyDown();
                break;
        }
    }

    // Elmentjük a band táblába az aktuális frekvencia értékét
    currentBand.currentFreq = si4735.getFrequency();

    DEBUG("AmDisplay::handleRotary -> config.data.bandIdx: %d, currentBand.currentFreq = %d, si4735.getFrequency() = %d\n", config.data.bandIdx, currentBand.currentFreq,
          si4735.getFrequency());

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

    // Néhány adatot csak ritkábban frissítünk
    static uint32_t elapsedTimedValues = 0;  // Kezdőérték nulla

    if ((millis() - elapsedTimedValues) >= SCREEN_COMPS_REFRESH_TIME_MSEC) {

        // RSSI
        si4735.getCurrentReceivedSignalQuality();
        uint8_t rssi = si4735.getCurrentRSSI();
        uint8_t snr = si4735.getCurrentSNR();
        pSMeter->showRSSI(rssi, snr, band.currentMode == FM);

        // Frissítjük az időbélyeget
        elapsedTimedValues = millis();
    }

    // A Frekvenciát azonnal frisítjuk, de csak ha változott
    uint16_t lastFreq = 0;
    uint16_t currFreq = band.getBandByIdx(config.data.bandIdx).currentFreq;  // A Rotary változtatásakor már eltettük a Band táblába
    if (lastFreq != currFreq) {

        // pSevenSegmentFreq->freqDraw(currFreq, 0);

        pSevenSegmentFreq->freqDispl(currFreq);

        DEBUG("AmDisplay::displayLoop -> config.data.bandIdx: %d, currentBand.currentFreq = %d, si4735.getFrequency() = %d\n", config.data.bandIdx,
              band.getBandByIdx(config.data.bandIdx).currentFreq, si4735.getFrequency());

        lastFreq = currFreq;
    }
}
