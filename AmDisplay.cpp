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

        DisplayBase::pDialog = new ValueChangeDialog(this, DisplayBase::tft, 270, 150, F("Antenna Tuning capacitor"), F("Capacitor value:"), &antCapValue, (int)0, (int)maxValue,
                                                     (int)0,  // A rotary encoder értéke lesz a step
                                                     [this](int newValue) {
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
    uint16_t currentFrequency = si4735.getFrequency();

    // SSB vagy CW módban a BFO-val finomhangolunk, de csak akkor ha a lépés nem 1000Hz, amit az si4735 amúgy is támogat
    if (rtv::freqstep != 1000 && (currMod == LSB or currMod == USB or currMod == CW)) {

        if (encoderState.direction == RotaryEncoder::Direction::Up) {

            // Felfelé hangolásnál
            rtv::freqDec = rtv::freqDec - rtv::freqstep;
            uint32_t freqTot = (uint32_t)(currentFrequency * 1000) + (rtv::freqDec * -1);
            if (freqTot > (uint32_t)(currentBand.pConstData->maximumFreq * 1000)) {
                si4735.setFrequency(currentBand.pConstData->maximumFreq);
                rtv::freqDec = 0;
            }

            if (rtv::freqDec <= -16000) {
                rtv::freqDec = rtv::freqDec + 16000;
                int16_t freqPlus16 = currentFrequency + 16;
                Si4735Utils::hardwareAudioMuteOn();
                si4735.setFrequency(freqPlus16);
            }

        } else {

            // Lefelé hangolásnál
            rtv::freqDec = rtv::freqDec + rtv::freqstep;
            uint32_t freqTot = (uint32_t)(currentFrequency * 1000) - rtv::freqDec;
            if (freqTot < (uint32_t)(currentBand.pConstData->minimumFreq * 1000)) {
                si4735.setFrequency(currentBand.pConstData->minimumFreq);
                rtv::freqDec = 0;
            }
            if (rtv::freqDec >= 16000) {
                rtv::freqDec = rtv::freqDec - 16000;
                int16_t freqMin16 = currentFrequency - 16;
                Si4735Utils::hardwareAudioMuteOn();
                si4735.setFrequency(freqMin16);
            }
        }

        config.data.currentBFO = rtv::freqDec;                 // freqDec a finomhangolás mértéke
        currentBand.varData.lastBFO = config.data.currentBFO;  // Mentsük el a finomhangolást

        // BFO beállítása: CW esetén alap offset + finomhangolás, SSB esetén csak finomhangolás
        const int16_t cwBaseOffset = (currMod == CW) ? DEFAULT_SW_SHIFT_FREQUENCY : 0;         // CW alap offset
        si4735.setSSBBfo(cwBaseOffset + config.data.currentBFO + config.data.currentBFOmanu);  // <- Itt állítjuk be a BFO-t Hz-ben!

        checkAGC();

    } else {

        // AM - sima frekvencia léptetés sávhatár ellenőrzéssel
        uint16_t step = currentBand.varData.currStep;
        uint16_t minFreq = currentBand.pConstData->minimumFreq;
        uint16_t maxFreq = currentBand.pConstData->maximumFreq;

        if (encoderState.direction == RotaryEncoder::Direction::Up) {
            // Csak akkor léptetünk felfelé, ha az új frekvencia nem lépi túl a maximumot
            if (currentFrequency < maxFreq && (currentFrequency + step) <= maxFreq) {
                si4735.frequencyUp();
            } else if (currentFrequency != maxFreq) {
                // Ha túllépné, de még nem a maximumon vagyunk, beállítjuk a maximumra
                si4735.setFrequency(maxFreq);
            }
        } else {  // Lefelé léptetés (encoderState.direction == RotaryEncoder::Direction::Down)
            // Csak akkor léptetünk lefelé, ha az új frekvencia nem megy a minimum alá
            if (currentFrequency > minFreq && (currentFrequency - step) >= minFreq) {
                si4735.frequencyDown();
            } else if (currentFrequency != minFreq) {
                // Ha alálépné, de még nem a minimumon vagyunk, beállítjuk a minimumra
                si4735.setFrequency(minFreq);
            }
        }
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
    uint8_t currMod = currentBand.varData.currMod;  // Aktuális mód lekérdezése

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
    static int16_t lastBfo = INT16_MIN;  // <-- ÚJ: Előző BFO érték tárolása (kezdetben érvénytelen érték)

    uint16_t currFreq = currentBand.varData.currFreq;
    int16_t currentBfo = 0;  // Alapértelmezett érték nem SSB/CW módhoz

    // Ha SSB vagy CW módban vagyunk, olvassuk ki a BFO-t is
    if (currMod == LSB || currMod == USB || currMod == CW) {
        currentBfo = currentBand.varData.lastBFO;
    }

    // Frissítés, ha az alapfrekvencia VAGY (SSB/CW módban) a BFO változott
    if (lastFreq != currFreq || ((currMod == LSB || currMod == USB || currMod == CW) && lastBfo != currentBfo)) {
        pSevenSegmentFreq->freqDispl(currFreq);
        lastFreq = currFreq;
        lastBfo = currentBfo;  // <-- ÚJ: Mentsük el az aktuális BFO-t is
    }
}
