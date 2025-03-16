#ifndef __MULTIBUTTONDIALOG_H
#define __MULTIBUTTONDIALOG_H

#define MULTI_BTN_W 80  // Multibutton dialog gombjainak szélessége
#define MULTI_BTN_H 30  // Multibutton dialog gombjainak magassága

/**
 * Multi buttons Dialóg
 */
class MultiButtonDialog : public DialogBase {
   private:
    /**
     * Kiszámítja a gombok elrendezését.
     *
     * @param maxRowWidth A rendelkezésre álló szélesség
     * @param buttonsPerRow Hány gomb fér el egy sorban
     * @param rowCount Hány sorra van szükség
     */
    void calculateButtonLayout(uint16_t maxRowWidth, uint8_t &buttonsPerRow, uint8_t &rowCount) {
        buttonsPerRow = 0;
        uint16_t totalWidth = 0;

        // Próbáljuk feltölteni egy sort, amíg elférnek a gombok
        for (uint8_t i = 0; i < buttonCount; i++) {
            uint16_t nextWidth = totalWidth + buttons[i]->getWidth() + (buttonsPerRow > 0 ? DLG_BTN_GAP : 0);

            if (nextWidth > maxRowWidth) {
                break;  // Ha már nem fér el, kilépünk
            }

            totalWidth = nextWidth;
            buttonsPerRow++;
        }

        rowCount = (buttonCount + buttonsPerRow - 1) / buttonsPerRow;  // Felkerekítés
    }

    /**
     * Gombok elhelyezése a dialóguson belül.
     *
     * @param buttonsPerRow Hány gomb fér el egy sorban
     * @param rowCount Hány sorra van szükség
     */
    void positionButtons(uint8_t buttonsPerRow, uint8_t rowCount) {
        uint16_t buttonHeight = DLG_BTN_H;
        uint16_t totalHeight = rowCount * buttonHeight + (rowCount - 1) * DLG_BTN_GAP;

        uint16_t startY = contentY;
        uint8_t row = 0, col = 0;
        uint16_t startX = 0;

        for (uint8_t i = 0; i < buttonCount; i++) {
            if (col == 0) {
                uint16_t rowWidth = 0;
                uint8_t itemsInRow = min(buttonsPerRow, buttonCount - i);
                for (uint8_t j = 0; j < itemsInRow; j++) {
                    rowWidth += buttons[i + j]->getWidth();
                }
                rowWidth += (itemsInRow - 1) * DLG_BTN_GAP;
                startX = x + (w - rowWidth) / 2;
            }

            buttons[i]->setPosition(startX, startY);
            startX += buttons[i]->getWidth() + DLG_BTN_GAP;
            col++;

            if (col >= buttonsPerRow) {
                col = 0;
                row++;
                startY += buttonHeight + DLG_BTN_GAP;
            }
        }
    }

   protected:
    TftButton **buttons;  // A megjelenítendő gombok mutatóinak tömbje.
    uint8_t buttonCount;  // A párbeszédpanelen lévő gombok száma.

    /**
     * Gombok legyártása a feliratok alapján
     */
    void buildButtonArray(const char *buttonLabels[], uint8_t buttonCount) {
        if (!buttonLabels || buttonCount == 0) {
            return;
        }

        this->buttonCount = buttonCount;
        buttons = new TftButton *[buttonCount];

        // Kezdő multiButton ID érték
        uint8_t id = DLG_MULTI_BTN_ID_START;

        // Button array feltöltése a gombokkal
        for (uint8_t i = 0; i < buttonCount; i++) {
            buttons[i] = new TftButton(id++, tft, MULTI_BTN_W, MULTI_BTN_H, buttonLabels[i], TftButton::ButtonType::Pushable);
        }
    }

    /**
     * @brief A gombok elhelyezésének fő metódusa.
     */
    virtual void placeButtons() {
        if (!buttons || buttonCount == 0) {
            return;
        }

        uint16_t maxRowWidth = w - 20;
        uint8_t buttonsPerRow, rowCount;
        calculateButtonLayout(maxRowWidth, buttonsPerRow, rowCount);
        positionButtons(buttonsPerRow, rowCount);
    }

   public:
    /**
     * @brief MultiButtonDialog létrehozása gombokkal, üzenet nélkül
     *
     * @param pTft Az TFT_eSPI objektumra mutató referencia.
     * @param w A párbeszédpanel szélessége.
     * @param h A párbeszédpanel magassága.
     * @param title A dialógus címe (opcionális).
     * @param buttons A gombok mutatóinak tömbje.
     */
    MultiButtonDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const char *buttonLabels[] = nullptr,
                      uint8_t buttonCount = 0)
        : DialogBase(pParent, tft, w, h, title) {

        // Legyártjuk a gombok tömbjét
        buildButtonArray(buttonLabels, buttonCount);

        // Elrendezzük a gombokat, ha vannak
        placeButtons();
    }

    /**
     * Dialóg destruktor
     * Töröljük a gombokat
     */
    ~MultiButtonDialog() {
        for (uint8_t i = 0; i < buttonCount; i++) {
            delete buttons[i];
        }
        delete[] buttons;
    }

    /**
     * A dialóg kirajzolása a TFT képernyőn.
     *
     * Ez a metódus beállítja a szöveg színét, szöveg helyzetét, és megrajzolja az üzenetet és a gombokat a képernyőn.
     */
    virtual void drawDialog() override {
        // Kirajzoljuk a dialógot
        DialogBase::drawDialog();

        // Gombok kirajzolása, ha vannak
        if (buttons) {
            for (uint8_t i = 0; i < buttonCount; i++) {
                buttons[i]->draw();
            }
        }
    }

    /**
     *  Dialóg Touch esemény lekezelése
     */
    bool dialogHandleTouch(bool touched, uint16_t tx, uint16_t ty) override {

        // Végigmegyünk az összes gombon és meghívjuk a touch kezeléseiket
        for (uint8_t i = 0; i < buttonCount; i++) {

            // Ha valamelyiket megíhvták annak a touch adatait visszaírjuk a képernyőbe, és kilépünk
            if (buttons[i]->handleTouch(touched, tx, ty)) {
                DialogBase::pParent->setDialogResponse(buttons[i]->buildButtonTouchEvent());
                return true;
            }
        }

        return false;
    }
};

#endif  // __MULTIBUTTONDIALOG_H