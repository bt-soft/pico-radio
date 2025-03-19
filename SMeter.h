#ifndef __SMETER_H
#define __SMETER_H

#include <TFT_eSPI.h>

/**
 *
 */
class SMeter {

   private:
    TFT_eSPI &tft;
    uint8_t smeterX;
    uint8_t smeterY;

    /**
     * RSS érték manipulálása
     */
    uint8_t rssiConverter(uint8_t rssi, bool isFM) {
        uint8_t spoint;
        if (!isFM) {
            // dBuV to S point conversion HF
            if ((rssi >= 0) and (rssi <= 1)) spoint = 12;                      // S0
            if ((rssi > 1) and (rssi <= 2)) spoint = 24;                       // S1
            if ((rssi > 2) and (rssi <= 3)) spoint = 36;                       // S2
            if ((rssi > 3) and (rssi <= 4)) spoint = 48;                       // S3
            if ((rssi > 4) and (rssi <= 10)) spoint = 48 + (rssi - 4) * 2;     // S4
            if ((rssi > 10) and (rssi <= 16)) spoint = 60 + (rssi - 10) * 2;   // S5
            if ((rssi > 16) and (rssi <= 22)) spoint = 72 + (rssi - 16) * 2;   // S6
            if ((rssi > 22) and (rssi <= 28)) spoint = 84 + (rssi - 22) * 2;   // S7
            if ((rssi > 28) and (rssi <= 34)) spoint = 96 + (rssi - 28) * 2;   // S8
            if ((rssi > 34) and (rssi <= 44)) spoint = 108 + (rssi - 34) * 2;  // S9
            if ((rssi > 44) and (rssi <= 54)) spoint = 124 + (rssi - 44) * 2;  // S9 +10
            if ((rssi > 54) and (rssi <= 64)) spoint = 140 + (rssi - 54) * 2;  // S9 +20
            if ((rssi > 64) and (rssi <= 74)) spoint = 156 + (rssi - 64) * 2;  // S9 +30
            if ((rssi > 74) and (rssi <= 84)) spoint = 172 + (rssi - 74) * 2;  // S9 +40
            if ((rssi > 84) and (rssi <= 94)) spoint = 188 + (rssi - 84) * 2;  // S9 +50
            if (rssi > 94) spoint = 204;                                       // S9 +60
            if (rssi > 95) spoint = 208;                                       //>S9 +60
        } else {
            // dBuV to S point conversion FM
            if (rssi < 1) spoint = 36;
            if ((rssi > 1) and (rssi <= 2)) spoint = 60;                       // S6
            if ((rssi > 2) and (rssi <= 8)) spoint = 84 + (rssi - 2) * 2;      // S7
            if ((rssi > 8) and (rssi <= 14)) spoint = 96 + (rssi - 8) * 2;     // S8
            if ((rssi > 14) and (rssi <= 24)) spoint = 108 + (rssi - 14) * 2;  // S9
            if ((rssi > 24) and (rssi <= 34)) spoint = 124 + (rssi - 24) * 2;  // S9 +10
            if ((rssi > 34) and (rssi <= 44)) spoint = 140 + (rssi - 34) * 2;  // S9 +20
            if ((rssi > 44) and (rssi <= 54)) spoint = 156 + (rssi - 44) * 2;  // S9 +30
            if ((rssi > 54) and (rssi <= 64)) spoint = 172 + (rssi - 54) * 2;  // S9 +40
            if ((rssi > 64) and (rssi <= 74)) spoint = 188 + (rssi - 64) * 2;  // S9 +50
            if (rssi > 74) spoint = 204;                                       // S9 +60
            if (rssi > 76) spoint = 208;                                       //>S9 +60
        }

        return spoint;
    }

    /**
     * kirajzolja az aktuális RSSI értélek
     */
    void smeter(uint8_t rssi, bool isFMMode) {
        static uint8_t prev_spoint = -1;  // Tárolja az előző S-pont értéket
        uint8_t spoint = rssiConverter(rssi, isFMMode);

        // Ha nem változott, nem frissítünk
        if (spoint == prev_spoint) {
            return;
        }
        prev_spoint = spoint;  // Frissítsük az előző értéket

        int tik = 0;
        int met = spoint + 2;
        while (met > 11 and tik < 9) {
            if (tik)
                tft.fillRect(smeterX + 20 + (tik * 12), smeterY + 38, 10, 6, TFT_ORANGE);
            else
                tft.fillRect(smeterX + 15, smeterY + 38, 15, 6, TFT_RED);
            met -= 12;
            tik++;
        }

        while (met > 15 and tik < 15) {
            tft.fillRect(smeterX + 20 + ((tik - 9) * 16) + 108, smeterY + 38, 14, 6, TFT_GREEN);
            met -= 16;
            tik++;
        }

        if (tik == 15 and met > 4) {
            tft.fillRect(smeterX + 20 + 204, smeterY + 38, 3, 6, TFT_ORANGE);
        } else {
            tft.fillRect(smeterX + 22 + spoint - met, smeterY + 38, 207 - (2 + spoint) + met, 6, TFT_BLACK);
        }
    }

   public:
    /**
     * Konstruktor
     */
    SMeter(TFT_eSPI &tft, uint8_t smeterX, uint8_t smeterY) : tft(tft), smeterX(smeterX), smeterY(smeterY) {}

    /**
     * S-meter skála kirajzolása
     */
    void drawSmeterScale() {
        String IStr;
        tft.setFreeFont();
        tft.setTextSize(1);
        tft.fillRect(smeterX + 2, smeterY + 6, 236, 46, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextDatum(BC_DATUM);
        for (int i = 0; i < 10; i++) {
            tft.fillRect(smeterX + 15 + (i * 12), smeterY + 24, 2, 8, TFT_WHITE);
            IStr = String(i);
            tft.setCursor((smeterX + 14 + (i * 12)), smeterY + 13);
            tft.print(i);
        }
        for (int i = 1; i < 7; i++) {
            tft.fillRect((smeterX + 123 + (i * 16)), smeterY + 24, 3, 8, TFT_RED);
            IStr = String(i * 10);
            tft.setCursor((smeterX + 117 + (i * 16)), smeterY + 13);
            if ((i == 2) or (i == 4) or (i == 6)) {
                tft.print("+");
                tft.print(i * 10);
            }
        }
        tft.fillRect(smeterX + 15, smeterY + 32, 112, 3, TFT_WHITE);
        tft.fillRect(smeterX + 127, smeterY + 32, 100, 3, TFT_RED);
    }

    /**
     * S-Meter + RSSI/SNR kiírás (csak nem FM esetén)
     */
    void showRSSI(uint8_t rssi, uint8_t snr, bool isFMMode) {

        smeter(rssi, isFMMode);

        if (isFMMode) {
            return;
        }

        // RSSI + SNR szöveges megjelenítése ha nem FM módban vagyunk
        tft.setTextSize(1);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);

        // dBuV and dB at freq. display
        tft.setTextDatum(TL_DATUM);
        tft.drawString("RSSI " + String(rssi) + " dBuV ", smeterX + 20, smeterY + 50);
        tft.setTextDatum(TR_DATUM);
        tft.drawString(" SNR " + String(snr) + " dB", smeterX + 180, smeterY + 50);
    }
};

#endif