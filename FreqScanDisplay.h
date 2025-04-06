#ifndef __FREQSCANDISPLAY_H
#define __FREQSCANDISPLAY_H

#include "DisplayBase.h"

class FreqScanDisplay : public DisplayBase {

   protected:
    /**
     * Rotary encoder esemény lekezelése
     */
    bool handleRotary(RotaryEncoder::EncoderState encoderState) override;

    /**
     * Touch (nem képrnyő button) esemény lekezelése
     */
    bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override;

    /**
     * Esemény nélküli display loop
     */
    void displayLoop() override;

    /**
     * Képernyő menügomb esemény feldolgozása
     */
    void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) override;

   public:
    FreqScanDisplay(TFT_eSPI &tft, SI4735 &si4735, Band &band);
    ~FreqScanDisplay();

    /**
     * Képernyő kirajzolása
     * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
     */
    void drawScreen() override;

    /**
     * Aktuális képernyő típusának lekérdezése
     */
    inline DisplayBase::DisplayType getDisplayType() override { return DisplayBase::DisplayType::freqScan; };

   private:
    static constexpr int spectrumWidth = 400;                    // Spektrum szélessége
    static constexpr int spectrumHeight = 150;                   // Spektrum magassága
    static constexpr int spectrumX = (480 - spectrumWidth) / 2;  // Spektrum vízszintes középre igazítása
    static constexpr int spectrumY = 50;                         // Spektrum teteje 50 pixellel a képernyő tetejétől

    bool scanning = false;            // Szkennelés állapota
    uint16_t currentFrequency = 0;    // Jelenlegi frekvencia
    uint16_t startFrequency = 0;      // Kezdő frekvencia
    uint16_t endFrequency = 0;        // Végfrekvencia
    uint16_t stepFrequency = 0;       // Lépésköz
    std::vector<uint8_t> rssiValues;  // RSSI értékek tárolása
    int prevTouchedX = -1;            // Előző érintett oszlop pozíciója

    void updateSpectrumScan();                 // Spektrum frissítése
    int calculateBarHeight(uint8_t rssi);      // Oszlopmagasság kiszámítása
    uint16_t calculateBarColor(uint8_t rssi);  // Oszlop színének kiszámítása
};

#endif  //__FREQSCANDISPLAY_H