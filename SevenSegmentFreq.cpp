#include "SevenSegmentFreq.h"

#include "DSEG7_Classic_Mini_Regular_34.h"
#include "rtVars.h"

#define FREQ_7SEGMENT_BFO_WIDTH 110   // BFO kijelzése alatt a kijelző szélessége
#define FREQ_7SEGMENT_SEEK_WIDTH 194  // Seek alatt a kijelző szélessége

// Színek a különböző módokhoz
const SegmentColors normalColors = {TFT_GOLD, TFT_COLOR(50, 50, 50), TFT_YELLOW};
const SegmentColors screenSaverColors = {TFT_SKYBLUE, TFT_COLOR(50, 50, 50), TFT_SKYBLUE};
const SegmentColors bfoColors = {TFT_ORANGE, TFT_BROWN, TFT_ORANGE};

/**
 * @brief Kirajzolja a frekvenciát a megadott formátumban.
 *
 * @param freq A megjelenítendő frekvencia.
 * @param mask A nem aktív szegmensek maszkja.
 * @param d Az X pozíció eltolása.
 * @param colors A szegmensek színei.
 * @param unit A mértékegység.
 */
void SevenSegmentFreq::drawFrequency(const String& freq, const __FlashStringHelper* mask, int d, const SegmentColors& colors, const __FlashStringHelper* unit) {
    uint16_t spriteWidth = rtv::bfoOn ? FREQ_7SEGMENT_BFO_WIDTH : tft.width() / 2;
    if (rtv::SEEK) {
        spriteWidth = FREQ_7SEGMENT_SEEK_WIDTH;
    }
    spr.createSprite(spriteWidth, FREQ_7SEGMENT_HEIGHT);
    spr.fillScreen(TFT_COLOR_BACKGROUND);
    spr.setTextSize(1);
    spr.setTextPadding(0);
    spr.setFreeFont(&DSEG7_Classic_Mini_Regular_34);
    spr.setTextDatum(BR_DATUM);

    int x = 222;
    if (rtv::bfoOn) {
        x = 110;
    } else if (band.getCurrentBand().varData.currMod == AM || band.getCurrentBand().varData.currMod == FM) {
        x = 190;
    } else if (band.getCurrentBandType() == MW_BAND_TYPE || band.getCurrentBandType() == LW_BAND_TYPE) {
        x = 222;
    }
    if (rtv::SEEK) {
        x = 144;
    }

    // Először a maszkot rajzoljuk ki
    if (config.data.tftDigitLigth) {
        spr.setTextColor(colors.inactive);
        spr.drawString(mask, x, FREQ_7SEGMENT_HEIGHT);
    }

    // Majd utána a frekvenciát
    spr.setTextColor(colors.active);
    spr.drawString(freq, x, FREQ_7SEGMENT_HEIGHT);

    // A frekvencia kijelzés szélességének kiszámítása a sprite-on
    frequencyDisplayWidth = spr.textWidth(freq);

    spr.pushSprite(freqDispX + d, freqDispY + 20);
    spr.setFreeFont();
    spr.deleteSprite();

    // Mértékegység kirajzolása
    if (unit != nullptr) {
        tft.setTextDatum(BC_DATUM);
        tft.setFreeFont();
        tft.setTextSize(2);
        tft.setTextColor(colors.indicator, TFT_COLOR_BACKGROUND);
        uint16_t xOffset = screenSaverActive ? 205 : 215;
        tft.drawString(unit, freqDispX + xOffset + d, freqDispY + 60);
    }
}

/**
 * @brief Kirajzolja a BFO frekvenciát.
 *
 * @param bfoValue A BFO frekvencia értéke.
 * @param d Az X pozíció eltolása.
 * @param colors A színek.
 */
void SevenSegmentFreq::drawBfo(int bfoValue, int d, const SegmentColors& colors) {
    drawFrequency(String(bfoValue), F("-888"), d, colors);
    tft.setTextSize(2);
    tft.setTextDatum(BL_DATUM);
    tft.setTextColor(colors.indicator, TFT_BLACK);
    tft.drawString("Hz", freqDispX + 120 + d, freqDispY + 40);
    tft.setTextColor(TFT_BLACK, colors.active);
    tft.fillRect(freqDispX + 156 + d, freqDispY + 21, 42, 20, colors.active);
    tft.drawString("BFO", freqDispX + 160 + d, freqDispY + 40);
    tft.setTextDatum(BR_DATUM);
}

/**
 * @brief Kirajzolja a frekvencia lépésének jelzésére az aláhúzást.
 *
 * @param d Az X pozíció eltolása.
 * @param colors A színek.
 */
void SevenSegmentFreq::drawStepUnderline(int d, const SegmentColors& colors) {
    tft.fillRect(freqDispX + 141 + d, freqDispY + 60, 81, 5, TFT_COLOR_BACKGROUND);
    uint8_t digitUnderLineX;
    switch (rtv::freqstepnr) {
        case 0:  // freqstep = 1 kHz;
            digitUnderLineX = 141;
            break;
        case 1:  // freqstep = 100 Hz;
            digitUnderLineX = 171;
            break;
        case 2:  // freqstep = 10 Hz;
            digitUnderLineX = 200;
            break;
    }
    tft.fillRect(freqDispX + digitUnderLineX + d, freqDispY + 60, 21, 5, colors.indicator);
}

/**
 *
 */
void SevenSegmentFreq::freqDispl(uint16_t currentFrequency) {

    int d = 0;
    const SegmentColors& colors = rtv::bfoOn ? bfoColors : (screenSaverActive ? screenSaverColors : normalColors);

    // ELőző érték törlése
    // Screesaver esetén nem törlünk, ott az egész képernyő törlése van
    if (!screenSaverActive) {
        tft.fillRect(freqDispX + 46 + d, freqDispY + 20, 194, FREQ_7SEGMENT_HEIGHT + 2, TFT_COLOR_BACKGROUND);
    }

    // Lekérjük az aktuális band rekordot-ot
    BandTable& currentBand = band.getCurrentBand();
    uint8_t currMod = currentBand.varData.currMod;
    uint8_t currentBandType = band.getCurrentBandType();

    if (currMod == LSB or currMod == USB or currMod == CW) {

        float Displayfreq = (currentFrequency * 1000) - (currentBand.varData.lastBFO);

        if (rtv::CWShift) {
            Displayfreq = Displayfreq + 700;
        }
        int mhz = trunc(Displayfreq / 1000000);
        int khz = Displayfreq - (mhz * 1000000);
        khz = trunc(khz / 1000);
        int hz = Displayfreq - (mhz * 1000000) - (khz * 1000);

        char s[12] = {'\0'};
        if (mhz > 0) {
            sprintf(s, "%i %03i.%02i", mhz, khz, hz / 10.0f);
        } else {
            sprintf(s, "%i.%02i", khz, hz / 10.0f);
        }

        if (!rtv::bfoOn or rtv::bfoTr) {
            tft.setTextDatum(BR_DATUM);
            tft.setTextColor(colors.indicator, TFT_COLOR_BACKGROUND);

            // A BFO frekvencia kijelzés miatt a frekvencia méretének az animált csökkentése flag
            if (rtv::bfoTr) {
                rtv::bfoTr = false;

                // Itt csökken a mérte a frekvencia kijelzésének, ha van BFO
                for (int i = 4; i > 1; i--) {
                    if (rtv::bfoOn) {
                        tft.setTextSize(i);
                    } else {
                        tft.setTextSize(6 - i);
                    }

                    tft.fillRect(freqDispX + d, freqDispY + 20, 240, 48, TFT_BLACK);
                    tft.drawString(String(s), freqDispX + 230 + d, freqDispY + 62);
                    delay(100);
                }
            }

            if (!rtv::bfoOn) {
                drawFrequency(String(s), F("88 888.88"), d, colors, F("KHz"));
            }

            // Képernyővédő üzemmódban nics aláhúzás a touch-hoz
            if (!screenSaverActive) {
                drawStepUnderline(d, colors);
            }
        }

        if (rtv::bfoOn) {
            drawBfo(config.data.currentBFOmanu, d, colors);
            tft.setTextDatum(BR_DATUM);
            tft.setTextColor(colors.indicator, TFT_COLOR_BACKGROUND);
            tft.drawString(String(s), freqDispX + 230 + d, freqDispY + 62);
        }

        tft.setTextDatum(BC_DATUM);

    } else {
        const __FlashStringHelper* unit = F("MHz");
        float displayFreq = 0;

        // FM?
        if (band.getCurrentBand().varData.currMod == FM) {
            displayFreq = currentFrequency / 100.0f;
            drawFrequency(String(displayFreq, 2), F("188.88"), d - 10, colors, unit);

        } else {
            // AM vagy LW?
            if (currentBandType == MW_BAND_TYPE or currentBandType == LW_BAND_TYPE) {
                displayFreq = currentFrequency;
                drawFrequency(String(displayFreq, 0), F("8888"), d, colors, F("kHz"));  // AM esetén más a maszk

            } else {  // SW !
                displayFreq = currentFrequency / 1000.0f;
                drawFrequency(String(displayFreq, 3), F("88.888"), d, colors, unit);
            }
        }
    }
}
