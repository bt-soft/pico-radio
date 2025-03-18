#ifndef __SCREENSAVERDISPLAY_H
#define __SCREENSAVERDISPLAY_H

#include <Arduino.h>

#include "DisplayBase.h"

class ScreenSaverDisplay : public DisplayBase {

   private:
    uint16_t saverX;
    uint16_t saverY;
    uint16_t posSaver = 0;
    uint32_t elapsedSaver = millis();
    uint8_t saverLineColors[63];  // A vonal színeinek tömbje

   protected:
    /**
     * Rotary encoder esemény lekezelése
     * A ScreeSaver nem figyeli a rotary-t, azt már a főprogram figyeli
     */
    bool handleRotary(RotaryEncoder::EncoderState encoderState) override { return false; };

    /**
     * Touch (nem képernyő button) esemény lekezelése
     * A ScreeSaver nem figyeli a touch, azt már a főprogram figyeli
     * Ezt a metódust a ScreenSaver animációjára használjuk
     */
    bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override;

    /**
     * Képernyő menügomb esemény feldolgozása
     * A ScreeSaver-nek nincsenek menügombjai
     */
    void processScreenButtonTouchEvent(TftButton::ButtonTouchEvent &event) override {};

    /**
     * Dialóg Button touch esemény feldolgozása
     * A ScreeSaver-nek nincsenek dialógusai
     */
    void processDialogButtonResponse(TftButton::ButtonTouchEvent &event) override {};

   public:
    /**
     * Konstruktor
     */
    ScreenSaverDisplay(TFT_eSPI &tft);
    /**
     * Destruktor
     */
    ~ScreenSaverDisplay() {};

    /**
     * Képernyő kirajzolása
     * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
     */
    void drawScreen() override {};

    /**
     * Aktuális képernyő típusának lekérdezése
     */
    inline DisplayBase::DisplayType getDisplayType() override { return DisplayBase::DisplayType::screenSaver; };
};

#endif  //__SCREENSAVERDISPLAY_H
