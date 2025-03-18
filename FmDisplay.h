#ifndef __FMDISPLAY_H
#define __FMDISPLAY_H

#include "DisplayBase.h"

class FmDisplay : public DisplayBase {
   private:
    bool ledState = false;
    int volume = 5;
    float temperature = 22.5;

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
    FmDisplay(TFT_eSPI &tft);
    ~FmDisplay();

    /**
     * Képernyő kirajzolása
     * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
     */
    void drawScreen() override;

    /**
     * Aktuális képernyő típusának lekérdezése
     */
    inline DisplayBase::DisplayType getDisplayType() override { return DisplayBase::DisplayType::fm; };
};

#endif  //__FMDISPLAY_H