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
};

#endif  //__FREQSCANDISPLAY_H