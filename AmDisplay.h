#ifndef __AMDISPLAY_H
#define __AMDISPLAY_H

#include "DisplayBase.h"

class AmDisplay : public DisplayBase {

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
     * Képernyő menügomb esemény feldolgozása
     */
    void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) override;

    /**
     * Dialóg Button touch esemény feldolgozása
     */
    void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) override;

    /**
     * Esemény nélküli display loop
     */
    void displayLoop() override;

   public:
    AmDisplay(TFT_eSPI &tft);
    ~AmDisplay();

    /**
     * Képernyő kirajzolása
     * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
     */
    void drawScreen() override;

    /**
     * Aktuális képernyő típusának lekérdezése
     */
    inline DisplayBase::DisplayType getDisplayType() override { return DisplayBase::DisplayType::am; };
};

#endif  //__AMDISPLAY_H