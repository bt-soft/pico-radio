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
    bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override;

    /**
     * Képernyő menügomb esemény feldolgozása
     */
    void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) override;

    /**
     * Dialóg Button touch esemény feldolgozása
     */
    void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) override;

public:
    FmDisplay(TFT_eSPI &tft);
    ~FmDisplay();

    void drawScreen() override;
};

#endif //__FMDISPLAY_H