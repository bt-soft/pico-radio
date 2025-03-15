#ifndef __DIALOGBASE_H
#define __DIALOGBASE_H

#include "IGuiEvents.h"
#include "TftButton.h"
#include "utils.h"

#define DLG_HEADER_H 30            // Fejléc magassága
#define DLG_CLOSE_BTN_SIZE 20      // Az 'X' gomb mérete
#define DLG_CLOSE_BUTTON_ID 254    // Jobb felső sarok bezáró gomb ID-je
#define DLG_CLOSE_BUTTON_LABEL "X" // Jobb felső sarok bezáró gomb felirata

/**
 *
 */
class DialogBase : public IGuiEvents {
private:
    const __FlashStringHelper *title;    // Flash memóriában tárolt title szöveg
    const __FlashStringHelper *message;  // Flash memóriában tárolt dialóg szöveg
    uint16_t y;                          // A leszármazottak nem láthatják az y pozíciót, csak a contentY alapján pozíciónálhatnak
    uint16_t messageY;                   // Az üzenet Y koordinátája
    uint16_t closeButtonX, closeButtonY; // X gomb pozíciója

protected:
    TFT_eSPI &tft;
    uint16_t x, w, h;  // a dialógus koordinátái az y érték nélkül
    uint16_t contentY; // Ezt az y értéket láthatják a leszármazottak

public:
    /**
     *
     */
    DialogBase(TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message = nullptr)
        : tft(tft), w(w), h(h), title(title), message(message) {

        // Dialóg bal felső sarkának kiszámítása a képernyő középre igzaításához
        x = (tft.width() - w) / 2;
        y = (tft.height() - h) / 2;

        messageY = y + (title ? DLG_HEADER_H + 15 : 5);      // Az üzenet a fejléc utánkezdődjön, ha van fejléc
        contentY = messageY + (message != nullptr ? 15 : 0); // A belső tér az üzenet után kezdődjön, ha van üzenet
    }

    /**
     *
     */
    virtual ~DialogBase() {
    }

    /**
     *
     */
    virtual void drawDialog() {

        // Kirajzoljuk a dialógot
        tft.fillRect(x, y, w, h, TFT_DARKGREY); // háttér

        // Title kiírása
        if (title != nullptr) {
            // Fejléc háttér kitöltése
            tft.fillRect(x, y, w, DLG_HEADER_H, TFT_NAVY);

            // Title kiírása
            tft.setTextColor(TFT_WHITE);
            tft.setTextDatum(TL_DATUM);                                                   // Bal felső sarokhoz igazítva
            tft.drawString(title, x + 10, y + 5 + (DLG_HEADER_H - tft.fontHeight()) / 2); // Bal oldali margó 10px

            // Fejléc vonala
            tft.drawFastHLine(x, y + DLG_HEADER_H, w, TFT_WHITE);
        }

        // Dialógus kerete
        tft.drawRect(x, y, w, h, TFT_WHITE); // keret

        // A header "X" gomb kirajzolása
        closeButtonX = x + w - DLG_CLOSE_BTN_SIZE - 5; // Az "X" gomb pozíciója a title jobb oldalán, kis margóval a jobb szélre
        closeButtonY = y + 5;                          // Fejléc tetejéhez igazítva
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(MC_DATUM); // Középre igazítva az "X"-et
        tft.drawString(F(DLG_CLOSE_BUTTON_LABEL), closeButtonX + DLG_CLOSE_BTN_SIZE / 2, closeButtonY + DLG_CLOSE_BTN_SIZE / 2);

        // Üzenet kirajzolása, ha van üzenet
        if (message != nullptr) {
            tft.setTextColor(TFT_WHITE);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(message, x + w / 2, messageY);
        }
    }
};

#endif // __DIALOGBASE_H