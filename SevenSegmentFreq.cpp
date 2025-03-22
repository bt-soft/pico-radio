#include "SevenSegmentFreq.h"

#include "DSEG7_Classic_Mini_Regular_34.h"
#include "rtVars.h"

#define COLOR_ACTIVE_SEGMENT TFT_GOLD                 // Aktív szegmens színe
#define COLOR_ACTIVE_SEGMENT_SCREENSAVER TFT_SKYBLUE  // Aktív szegmens színe a képernyővédő alatt
#define COLOR_INACTIVE_SEGMENT TFT_COLOR(50, 50, 50)  // Nem aktív szegmens színe

#define BFO_COLOR_ACTIVE_SEGMENT TFT_ORANGE   // Aktív szegmens színe BFO esetén
#define BFO_COLOR_INACTIVE_SEGMENT TFT_BROWN  // Inaktív szegmens színe BFO esetén

/**
 *
 */
void SevenSegmentFreq::segment(String freq, String mask, int d) {

    if (!config.data.tftDigitLigth) {
        mask = "";
    }

    if (rtv::SEEK) {
        spr.createSprite(194, 38);
        d = 46;
    } else {

        if (rtv::bfoOn) {
            spr.createSprite(110, 38);
        } else {
            spr.createSprite(240, 38);
        }
    }
    spr.fillScreen(TFT_COLOR_BACKGROUND);
    spr.setTextSize(1);
    spr.setTextPadding(0);
    spr.setFreeFont(&DSEG7_Classic_Mini_Regular_34);
    spr.setTextDatum(BR_DATUM);
    int x = 222;

    if (rtv::bfoOn) {
        x = 110;
        spr.setTextColor(BFO_COLOR_INACTIVE_SEGMENT);
        spr.drawString(mask, x, 38);

        spr.setTextColor(BFO_COLOR_ACTIVE_SEGMENT);
        spr.drawString(freq, x, 38);

    } else {

        if (band.currentMode == AM || band.currentMode == FM) {
            x = 190;
        }

        if (rtv::SEEK) {
            x = 144;
        }

        // Nem aktív szegmens
        spr.setTextColor(rtv::bfoOn ? BFO_COLOR_INACTIVE_SEGMENT : COLOR_INACTIVE_SEGMENT);
        spr.drawString(mask, x, 38);

        // Aktív szegmens - ha a screensaver aktív, akkor más színnel rajzoljuk ki
        spr.setTextColor(rtv::bfoOn ? BFO_COLOR_ACTIVE_SEGMENT : screenSaverActive ? COLOR_ACTIVE_SEGMENT_SCREENSAVER : COLOR_ACTIVE_SEGMENT);
        spr.drawString(freq, x, 38);
    }

    spr.pushSprite(freqDispX + d, freqDispY + 20);
    spr.setFreeFont();
    spr.deleteSprite();
}

/**
 *
 */
void SevenSegmentFreq::freqDraw(float freq, int d) {

    const __FlashStringHelper* unit = F("MHz");
    float displayFreq = 0;

    // ELőző érték törlése, screesaver esetén nem
    if (!screenSaverActive) {
        tft.fillRect(freqDispX + 46 + d, freqDispY + 20, 194, 40, TFT_COLOR_BACKGROUND);
    }

    // FM?
    if (band.currentMode == FM) {
        displayFreq = freq / 100;
        segment(String(displayFreq, 2), "188.88", d - 10);

    } else {
        // AM vagy LW?
        uint8_t bandType = band.getBandByIdx(config.data.bandIdx).bandType;
        if (bandType == MW_BAND_TYPE or bandType == LW_BAND_TYPE) {
            displayFreq = freq;
            segment(String(displayFreq, 0), "1888", d);
            unit = F("kHz");

        } else {  // SW !
            displayFreq = freq / 1000;
            segment(String(displayFreq, 3), "88.888", d);
        }
    }

    // Mértékegység kirajzolása
    tft.setTextDatum(BC_DATUM);
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(screenSaverActive ? COLOR_ACTIVE_SEGMENT_SCREENSAVER : COLOR_ACTIVE_SEGMENT, TFT_COLOR_BACKGROUND);

    uint16_t xOffset = screenSaverActive ? 205 : 215;
    tft.drawString(unit, freqDispX + xOffset + d, freqDispY + 60);
}
