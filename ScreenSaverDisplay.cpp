#include "ScreenSaverDisplay.h"

#include <Arduino.h>

/**
 * Képernyő kirajzolása
 * (Az esetleges dialóg eltűnése után a teljes képernyőt újra rajzoljuk)
 */
void ScreenSaverDisplay::drawScreen() {
    tft.fillScreen(TFT_BLACK);
    // tft.setFreeFont();
    // tft.setTextFont(2);

    // tft.setTextSize(2);
    // tft.setTextDatum(MC_DATUM);  // Középre igazítás
    // tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // tft.drawString("ScreenSaver display", tft.width() / 2, tft.height() / 2);
}

/**
 * Touch (nem képrnyő button) esemény lekezelése
 * A ScreeSaver nem figyeli a touch, azt már a főprogram figyeli
 * Ezt a metódust a ScreenSaver animációjára használjuk
 */
bool ScreenSaverDisplay::handleTouch(bool touched, uint16_t tx, uint16_t ty) {

    int t = posSaver;
    posSaver++;
    if (posSaver == 500) posSaver = 0;

    for (int i = 0; i < 63; i++) {
        int c = 31 - abs(i - 31);
        if (t < 200) {
            tft.drawPixel(saverX - 10 + t, saverY - 5, (c * 64) + c);
        } else if (t >= 200 and t < 250) {
            tft.drawPixel(saverX + 189, saverY - 205 + t, (c * 64) + c);
        } else if (t >= 250 and t < 450) {
            tft.drawPixel(saverX + 439 - t, saverY + 44, (c * 64) + c);
        } else {
            tft.drawPixel(saverX - 10, saverY + 494 - t, (c * 64) + c);
        }
        t += 3;
        if (t > 499) t -= 500;
    }

    if ((elapsedSaver + 15000) < millis()) {  // 15 mp-enként
        elapsedSaver = millis();

        tft.fillScreen(TFT_BLACK);

        // if (screenV) {
        //     saverX = random(40) + 10;
        //     saverY = random(270) + 5;
        // } else {
        saverX = random(120) + 10;
        saverY = random(190) + 5;
        // }

        // FreqDraw(freq, 0);

        // if (batShow) {
        //     // float vsupply = 3.724 * analogRead(PIN_BAT_INFO) / 2047; // 3.3v
        //     float vSupply = readBatterry();
        //     int bat = map(int(vSupply * 100), 270, 405, 0, 100);
        //     if (bat < 0) bat = 0;
        //     if (bat > 100) bat = 100;
        //     int colorBatt = TFT_DARKCYAN;
        //     if (bat < 15) colorBatt = TFT_ORANGE;
        //     if (bat < 5) colorBatt = 64528;
        //     tft.drawRect(saverX + 145, saverY, 38, 18, colorBatt);
        //     tft.drawRect(saverX + 184, saverY + 4, 2, 10, colorBatt);
        //     tftRusSetFont(T1012_T);
        //     tftRusSetSize(1);
        //     tftRusSetColor(TFT_CYAN, TFT_TRANS);
        //     tftRusSetDatum(BC_T);
        //     tftRusSetStyle(NRG_T);
        //     tftRusPrint(String(bat) + "%", saverX + 164, saverY + 15);
        // }
    }

    return false;
}
