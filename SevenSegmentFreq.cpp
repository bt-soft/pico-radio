#include "SevenSegmentFreq.h"

#include "DSEG7_Classic_Mini_Regular_34.h"
#include "rtVars.h"

#define COLOR_ACTIVE_SEGMENT TFT_GOLD                 // Aktív szegmens színe
#define COLOR_ACTIVE_SEGMENT_SCREENSAVER TFT_SKYBLUE  // Aktív szegmens színe a képernyővédő alatt
#define COLOR_INACTIVE_SEGMENT TFT_COLOR(50, 50, 50)  // Nem aktív szegmens színe

#define BFO_COLOR_ACTIVE_SEGMENT TFT_ORANGE   // Aktív szegmens színe BFO esetén
#define BFO_COLOR_INACTIVE_SEGMENT TFT_BROWN  // Inaktív szegmens színe BFO esetén

#define COLOR_INDICATOR_FREQ TFT_GOLD  // BFO frekvenciája

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
void SevenSegmentFreq::freqDraw(uint16_t currentFrequency, int d) {

    const __FlashStringHelper* unit = F("MHz");
    float displayFreq = 0;

    // ELőző érték törlése, screesaver esetén nem
    if (!screenSaverActive) {
        tft.fillRect(freqDispX + 46 + d, freqDispY + 20, 194, 40, TFT_COLOR_BACKGROUND);
    }

    // FM?
    if (band.currentMode == FM) {
        displayFreq = currentFrequency / 100.0f;
        segment(String(displayFreq, 2), "188.88", d - 10);

    } else {
        // AM vagy LW?
        uint8_t bandType = band.getCurrentBand().bandType;
        if (bandType == MW_BAND_TYPE or bandType == LW_BAND_TYPE) {
            displayFreq = currentFrequency;
            segment(String(displayFreq, 0), "1888", d);
            unit = F("kHz");

        } else {  // SW !
            displayFreq = currentFrequency / 1000.0f;
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

/**
 *
 */
void SevenSegmentFreq::freqDispl(uint16_t currentFrequency) {

    DEBUG("SevenSegmentFreq::freqDispl -> currentFrequency = %d\n", currentFrequency);

    int d = 0;

    // if ((SCANbut or HamBand or Modebut or STEPbut or BandWidth or MEMObut) and !screenV) d = 40;

    // if (!FREQbut and !HamBand and !Modebut and !BandWidth and !BroadBand and !SCANbut and !MEMObut and !STEPbut) {
    //     AGCfreqdisp();
    //     BFOStepdisp();
    // }

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(4);
    tft.setTextDatum(BC_DATUM);

    // Lekérjük az aktuális band rekordot-ot
    BandTable& currentBand = band.getCurrentBand();

    if (band.currentMode == LSB or band.currentMode == USB or band.currentMode == CW) {

        float Displayfreq = (currentFrequency * 1000) - (currentBand.lastBFO);

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
            tft.setTextColor(COLOR_INDICATOR_FREQ, TFT_COLOR_BACKGROUND);

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
                segment(String(s), "88 888.88", d);
            }

            tft.setTextSize(2);

            // if (FREQbut or HamBand or Modebut or BandWidth or BroadBand or SCANbut or MEMObut or STEPbut) {
            //     tft.fillRect(freqDispX + d, freqDispY + 60, 240, 20, TFT_GREY);
            //     tft.setTextColor(TFT_YELLOW, TFT_GREY);
            //     tft.drawString("KHz", freqDispX + 234 + d, freqDispY + 78);
            // } else {
            tft.setTextColor(TFT_YELLOW, TFT_COLOR_BACKGROUND);
            tft.drawString("KHz", freqDispX + 229 + d, freqDispY + 84);
            // }

            // A digit aláhúzása jelezve a stepsize értékét
            if (rtv::bfoOn == false /*and (FREQbut or HamBand or Modebut or BandWidth or BroadBand or SCANbut or MEMObut) == false*/) {

                // Törölni a korábbi aláhúzást
                tft.fillRect(freqDispX + 141 + d, freqDispY + 60, 81, 5, TFT_COLOR_BACKGROUND);

                uint16_t freqstepnr;

                uint8_t digitUnderLineX;
                switch (freqstepnr) {
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

                tft.fillRect(freqDispX + digitUnderLineX + d, freqDispY + 60, 21, 5, TFT_ORANGE);
            }
        }

        if (rtv::bfoOn) {
            segment(String(config.data.currentBFOmanu), "-888", d);
            tft.setTextSize(2);
            tft.setTextDatum(BL_DATUM);
            tft.setTextColor(TFT_ORANGE, TFT_BLACK);
            tft.drawString("Hz", freqDispX + 120 + d, freqDispY + 40);
            tft.setTextColor(TFT_BLACK, TFT_ORANGE);
            tft.fillRect(freqDispX + 156 + d, freqDispY + 21, 42, 20, TFT_ORANGE);
            tft.drawString("BFO", freqDispX + 160 + d, freqDispY + 40);
            tft.setTextDatum(BR_DATUM);

            // if (PREtap)
            //     tft.setTextColor(TFT_LIGTHYELLOW, COLOR_BACKGROUND);
            // else
            tft.setTextColor(COLOR_INDICATOR_FREQ, TFT_COLOR_BACKGROUND);

            tft.drawString(String(s), freqDispX + 230 + d, freqDispY + 62);
        }

        tft.setTextDatum(BC_DATUM);

    } else {
        freqDraw(currentFrequency, d);
        // if (FREQbut or HamBand or Modebut or BandWidth or BroadBand or SCANbut or MEMObut or STEPbut) {
        //     tft.fillRect(freqDispX + d, freqDispY + 60, 240, 20, TFT_GREY);
        // }
    }
}
