#ifndef __FMDISPLAY_H
#define __FMDISPLAY_H

#include "DisplayBase.h"

class FmDisplay : public DisplayBase {

protected:
    /**
     * Rotary encoder esemény lekezelése
     */
    void handleRotary(RotaryEncoder::EncoderState encoderState) override;

    /**
     * Touch (nem képrnyő button) esemény lekezelése
     */
    void handleTouch(bool touched, uint16_t tx, uint16_t ty) override;

    /**
     * Képernyő menügomb esemény feldolgozása
     */
    void handleScreenButtonTouchEvent(TftButton::ButtonTouchEvent &screenButtonTouchEvent) override;

public:
    FmDisplay(TFT_eSPI &tft);
    ~FmDisplay();

    void drawScreen() override;
};

#endif //__FMDISPLAY_H