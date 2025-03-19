#include "SevenSegmentFreq.h"

#include "DSEG7_Classic_Mini_Regular_34.h"
#include "rtVars.h"

#define TFT_COLOR_INACTIVE_SEGMENT TFT_COLOR(50, 50, 50)  // Nem aktív szegmens színe
#define COLOR_INDICATOR_FREQ TFT_GOLD

/**
 *
 */
void SevenSegmentFreq::Segment(String freq, String mask, int d) {

    if (!config.data.digitLigth) {
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
        spr.setTextColor(TFT_BROWN);
        spr.drawString(mask, x, 38);
        spr.setTextColor(TFT_ORANGE);
        spr.drawString(freq, x, 38);

    } else {

        if (band.currentMode == AM || band.currentMode == FM) {
            x = 190;
        }

        if (rtv::SEEK) {
            x = 144;
        }

        if (rtv::bfoOn) {
            spr.setTextColor(TFT_BROWN);
        } else {
            spr.setTextColor(TFT_COLOR_INACTIVE_SEGMENT);  // Nem aktív szegmens színe
        }

        spr.drawString(mask, x, 38);

        if (rtv::bfoOn) {
            spr.setTextColor(TFT_ORANGE);
        } else {
            spr.setTextColor(COLOR_INDICATOR_FREQ);
        }

        spr.drawString(freq, x, 38);
    }

    spr.pushSprite(freqDispX + d, freqDispY + 20);
    spr.setFreeFont();
    spr.deleteSprite();
}

/**
 *
 */
void SevenSegmentFreq::FreqDraw(float freq, int d) {

    String unitStr = "MHz";
    float displayFreq = 0;

    // ELőző érték törlése
    tft.fillRect(freqDispX + 46 + d, freqDispY + 20, 194, 40, TFT_COLOR_BACKGROUND);

    // FM?
    if (band.currentMode == FM) {
        displayFreq = freq / 100;
        Segment(String(displayFreq, 2), "188.88", d - 10);

    } else {
        // AM vagy LW?
        uint8_t bandType = band.getBandByIdx(config.data.bandIdx).bandType;
        if (bandType == MW_BAND_TYPE or bandType == LW_BAND_TYPE) {
            displayFreq = freq;
            Segment(String(displayFreq, 0), "1888", d);
            unitStr = "kHz";

        } else {  // SW !
            displayFreq = freq / 1000;
            Segment(String(displayFreq, 3), "88.888", d);
        }
    }

    // Mértékegység kirajzolása
    tft.setTextDatum(BC_DATUM);
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_COLOR_BACKGROUND);
    tft.drawString(unitStr, freqDispX + 215 + d, freqDispY + 60);
}
