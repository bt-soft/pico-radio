#include "utils.h"

namespace Utils {

/**
 * Várakozás a soros port megnyitására
 * @param pTft a TFT kijelző példánya
 * @param beeper a Beeper példánya
 */
void debugWaitForSerial(TFT_eSPI *pTft) {
#ifdef __DEBUG
    beepError();
    pTft->setTextColor(TFT_WHITE);
    pTft->drawString("Nyisd meg a soros portot!", 0, 0);
    while (!Serial) {
    }
    pTft->fillScreen(TFT_BLACK);
    beepTick();
#endif
}

/**
 * TFT érintőképernyő kalibráció
 * @param pTft TFT kijelző példánya
 * @param calData kalibrációs adatok
 */
void tftTouchCalibrate(TFT_eSPI *pTft, uint16_t (&calData)[5]) {

    pTft->fillScreen(TFT_BLACK);
    pTft->setTextFont(2);
    pTft->setTextSize(2);
    const __FlashStringHelper *txt = F("TFT touch calibration required\n");
    pTft->setCursor((pTft->width() - pTft->textWidth(txt)) / 2, pTft->height() / 2 - 60);
    pTft->setTextColor(TFT_ORANGE, TFT_BLACK);
    pTft->println(txt);

    pTft->setTextSize(1);
    txt = F("Touch the corners at the indicated places!\n");
    pTft->setCursor((pTft->width() - pTft->textWidth(txt)) / 2, pTft->height() / 2 + 20);
    pTft->setTextColor(TFT_YELLOW, TFT_BLACK);
    pTft->println(txt);

    // TFT_eSPI 'bóti' kalibráció indítása
    pTft->calibrateTouch(calData, TFT_YELLOW, TFT_BLACK, 15);

    txt = F("Kalibracio befejezodott!");
    pTft->fillScreen(TFT_BLACK);
    pTft->setCursor((pTft->width() - pTft->textWidth(txt)) / 2, pTft->height() / 2);
    pTft->setTextColor(TFT_GREEN, TFT_BLACK);
    pTft->setTextSize(1);
    pTft->println(txt);

    DEBUG("// Használd ezt a kalibrációs kódot a setup()-ban:\n");
    DEBUG("  uint16_t calData[5] = { ");
    for (uint8_t i = 0; i < 5; i++) {
        DEBUG("%d", calData[i]);
        if (i < 4) {
            DEBUG(", ");
        }
    }
    DEBUG(" };\n");
    DEBUG("  pTft->setTouch(calData);\n");
}

/**
 *  Pitty hangjelzés
 */
void beepTick() {
    tone(PIN_BEEPER, 800);
    delay(10);
    noTone(PIN_BEEPER);
}

/**
 * Hiba jelzés
 */
void beepError() {
    tone(PIN_BEEPER, 500);
    delay(100);
    tone(PIN_BEEPER, 500);
    delay(100);
    tone(PIN_BEEPER, 500);
    delay(100);
    noTone(PIN_BEEPER);
}

} // namespace Utils
