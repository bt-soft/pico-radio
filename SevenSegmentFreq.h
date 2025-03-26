#ifndef __SEVENSEGMENTFREQ_H
#define __SEVENSEGMENTFREQ_H

#include <TFT_eSPI.h>

#include "Band.h"
#include "Config.h"
#include "rtVars.h"

/**
 *
 */
class SevenSegmentFreq {

   private:
    TFT_eSPI &tft;
    TFT_eSprite spr;

    Band &band;

    uint16_t freqDispX, freqDispY;
    bool screenSaverActive;

    void segment(String freq, String mask, int d);

    void freqDraw(uint16_t freq, int d);

   public:
    /**
     *
     */
    SevenSegmentFreq(TFT_eSPI &tft, uint16_t freqDispX, uint16_t freqDispY, Band &band, bool screenSaverActive = false)
        : tft(tft), freqDispX(freqDispX), freqDispY(freqDispY), band(band), screenSaverActive(screenSaverActive), spr(&tft) {}

    /**
     *
     */
    void freqDispl(uint16_t freq);

    /**
     * Pozíció beállítása (pl.: a ScreenSaver számára)
     */
    inline void setPositions(uint16_t freqDispX, uint16_t freqDispY) {
        this->freqDispX = freqDispX;
        this->freqDispY = freqDispY;
    }
};

#endif  //__SEVENSEGMENTFREQ_H