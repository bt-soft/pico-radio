#include "FreqScanDisplay.h"

#include <Arduino.h>

/**
 * Konstruktor
 */
FreqScanDisplay::FreqScanDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band) : DisplayBase(tft, si4735, band) {

    // Horizontális képernyőgombok definiálása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"FM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
        {"AM", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},  //
    };

    // Horizontális képernyőgombok legyártása
    DisplayBase::buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData));
}

/**
 * Destruktor
 */
FreqScanDisplay::~FreqScanDisplay() {}

/**
 * Rotary encoder esemény lekezelése
 */
bool FreqScanDisplay::handleRotary(RotaryEncoder::EncoderState encoderState) { return false; }

/**
 * Képernyő kirajzolása
 * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
 */
void FreqScanDisplay::drawScreen() {
    startFrequency = band.getCurrentBand().pConstData->minimumFreq;
    endFrequency = band.getCurrentBand().pConstData->maximumFreq;
    stepFrequency = band.getCurrentBand().pConstData->defStep;

    tft.setFreeFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);

    // Gombok kirajzolása
    DisplayBase::BuildButtonData horizontalButtonsData[] = {
        {"Start Scan", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},
        {"Stop Scan", TftButton::ButtonType::Pushable, TftButton::ButtonState::Off},
    };
    DisplayBase::buildHorizontalScreenButtons(horizontalButtonsData, ARRAY_ITEM_COUNT(horizontalButtonsData));
    DisplayBase::drawScreenButtons();

    // Spektrumkép keretének kirajzolása
    tft.drawRect(spectrumX - 1, spectrumY - 1, spectrumWidth + 10, spectrumHeight + 2, TFT_WHITE);
}

/**
 * Esemény nélküli display loop -> Adatok periódikus megjelenítése
 */
void FreqScanDisplay::displayLoop() {
    if (scanning) {
        updateSpectrumScan();
    }
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A további gui elemek vezérléséhez
 */
bool FreqScanDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {
    if (!touched || rssiValues.empty()) return false;

    // Az érintett oszlop alapján számítsuk ki a frekvenciát
    uint16_t touchedFrequency = map(tx, spectrumX, spectrumX + spectrumWidth, startFrequency, endFrequency);
    int touchedX = map(touchedFrequency, startFrequency, endFrequency, spectrumX, spectrumX + spectrumWidth);

    // Töröljük az előző sárga csíkot és állítsuk vissza az eredeti oszlopot
    if (prevTouchedX != -1) {
        uint8_t prevRssi = rssiValues[prevTouchedX / 2];
        int prevBarHeight = calculateBarHeight(prevRssi);
        uint16_t prevColor = calculateBarColor(prevRssi);

        // Töröljük a sárga csíkot
        tft.fillRect(prevTouchedX, spectrumY, 2, spectrumHeight, TFT_BLACK);

        // Rajzoljuk újra az eredeti oszlopot
        tft.fillRect(prevTouchedX, spectrumY + spectrumHeight - prevBarHeight, 2, prevBarHeight, prevColor);
    }

    // Rajzoljuk ki az új sárga csíkot
    tft.fillRect(touchedX, spectrumY, 2, spectrumHeight, TFT_YELLOW);

    // Hangoljuk a rádiót az adott frekvenciára
    si4735.setFrequency(touchedFrequency);
    currentFrequency = touchedFrequency;

    // Frissítsük a bal felső sarokban az aktuális frekvenciát
    tft.fillRect(0, 0, 100, 20, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Fehér szöveg fekete háttérrel
    tft.setTextDatum(TL_DATUM);              // Bal felső sarokhoz igazítás
    tft.drawString(String(touchedFrequency) + " kHz", 10, 10);

    prevTouchedX = touchedX;
    return true;
}

/**
 * Képernyő menügomb esemény feldolgozása
 */
void FreqScanDisplay::processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) {
    if (STREQ("Start Scan", event.label)) {
        scanning = true;
        rssiValues.clear();
        currentFrequency = startFrequency;
        tft.fillRect(spectrumX, spectrumY, spectrumWidth, spectrumHeight, TFT_BLACK);  // Spektrum törlése

    } else if (STREQ("Stop Scan", event.label)) {
        scanning = false;
    }
}

/**
 * Spektrum szkennelés frissítése
 */
void FreqScanDisplay::updateSpectrumScan() {
    if (!scanning) return;

    si4735.setFrequency(currentFrequency);
    delay(50);  // Stabilizációs idő

    // Frissítsük a bal felső sarokban az aktuális frekvenciát
    tft.fillRect(0, 0, 100, 20, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Fehér szöveg fekete háttérrel
    tft.setTextDatum(TL_DATUM);              // Bal felső sarokhoz igazítás
    tft.drawString(String(currentFrequency) + " kHz", 10, 10);

    si4735.getCurrentReceivedSignalQuality();
    uint8_t rssi = si4735.getCurrentRSSI();
    rssiValues.push_back(rssi);

    uint8_t barHeight = calculateBarHeight(rssi);
    uint16_t barColor = calculateBarColor(rssi);

    // Oszlop pozíciója
    uint32_t x = map(currentFrequency, startFrequency, endFrequency, spectrumX, spectrumX + spectrumWidth);

    // Oszlop kirajzolása
    uint8_t barWidth = (rssi < 10.0f) ? 1 : 2;  // Zajszint oszlopai vékonyabbak
    tft.fillRect(x, spectrumY + spectrumHeight - barHeight, barWidth, barHeight, barColor);

    // Következő frekvencia
    currentFrequency += stepFrequency;
    if (currentFrequency > endFrequency) {
        scanning = false;

        // Szöveg balra igazítása
        tft.setTextDatum(TL_DATUM);              // Bal felső sarokhoz igazítás
        tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Fehér szöveg fekete háttérrel
        tft.drawString("Scan complete", spectrumX, spectrumY + spectrumHeight + 10);
    }
}

/**
 * Oszlop magasságának kiszámítása
 */
int FreqScanDisplay::calculateBarHeight(uint8_t rssi) {
    constexpr float rssiMin = 5.0f;
    constexpr float rssiMax = 127.0f;
    constexpr float noiseThreshold = 10.0f;
    constexpr float signalThreshold = 50.0f;

    if (rssi < noiseThreshold) {
        rssi = 0;  // Zajszint elnyomása
    }

    float constrainedRssi = constrain((float)rssi, rssiMin, rssiMax);
    float normalizedRssi = pow((constrainedRssi - rssiMin) / (rssiMax - rssiMin), 3.0f);

    // Adók további kiemelése
    if (rssi >= signalThreshold) {
        float signalBoost = (rssi - signalThreshold) / (rssiMax - signalThreshold);
        normalizedRssi += signalBoost * 0.8f;
    }

    return (int)(normalizedRssi * spectrumHeight);
}

/**
 * Oszlop színének kiszámítása
 */
uint16_t FreqScanDisplay::calculateBarColor(uint8_t rssi) {
    constexpr float rssiMin = 5.0f;
    constexpr float rssiMax = 127.0f;

    uint8_t red = map(rssi, rssiMin, rssiMax, 0, 255);
    uint8_t green = map(rssi, rssiMin, rssiMax, 255, 0);
    uint8_t blue = map(rssi, rssiMin, rssiMax, 255, 128);

    return tft.color565(red, green, blue);
}
