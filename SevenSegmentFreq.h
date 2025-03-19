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
    uint16_t freqDispX, freqDispY;

    void Segment(String freq, String mask, int d);

   public:
    SevenSegmentFreq(TFT_eSPI &tft, uint16_t freqDispX, uint16_t freqDispY) : tft(tft), freqDispX(freqDispX), freqDispY(freqDispY), spr(&tft) {}

    void FreqDraw(float freq, int d);
};

#endif  //__SEVENSEGMENTFREQ_H