#ifndef __SCREENSAVERDISPLAY_H
#define __SCREENSAVERDISPLAY_H

#include "DisplayBase.h"

class ScreenSaverDisplay : public DisplayBase {

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

   public:
    ScreenSaverDisplay(TFT_eSPI &tft);
    ~ScreenSaverDisplay();

    /**
     * Képernyő kirajzolása
     * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
     */
    void drawScreen() override;

    /**
     * Aktuális képernyő típusának lekérdezése
     */
    inline DisplayBase::DisplayType getDisplayType() override { return DisplayBase::DisplayType::screenSaver; };
};

#endif  //__SCREENSAVERDISPLAY_H