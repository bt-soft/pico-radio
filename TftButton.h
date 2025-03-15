#ifndef __TFTBUTTON_H
#define __TFTBUTTON_H

#include "utils.h"

// Benyomott gomb háttérszín gradienshez, több iterációs lépés -> erősebb hatás
#define TFT_BUTTON_DARKEN_COLORS_STEPS 6
#define TFT_BUTTON_INVALID_ID 0xFF

class TftButton {
public:
    // A gomb típusa
    enum ButtonType {
        Toggleable,
        Pushable
    };

    // Gomb állapotai
    enum ButtonState {
        Off = 0,
        On,
        Disabled,
        //---- technikai állapotok
        Pushed, // Csak az esemény jelzésére a calbback függvénynek, nincs színhez kötve az állapota
        //---
        HOLD,   // Nyomva tartják
        UNKNOWN // ismeretlen
    };

    struct ButtonTouchEvent {
        uint8_t id;
        const char *label;
        ButtonState state;
    };

    // Érvénytelen gomb esemény kezdeti értéke
    static constexpr ButtonTouchEvent noTouchEvent = {TFT_BUTTON_INVALID_ID, nullptr, ButtonState::UNKNOWN};

    // Barát függvények deklarálása
    friend bool operator==(const ButtonTouchEvent &lhs, const ButtonTouchEvent &rhs);
    friend bool operator!=(const ButtonTouchEvent &lhs, const ButtonTouchEvent &rhs);

private:
    // A gomb állapotainak megfelelő háttérszín
    const uint16_t TFT_BUTTON_STATE_BG_COLORS[3] = {
        TFT_COLOR(65, 65, 114), // normal
        TFT_COLOR(65, 65, 114), // pushed
        TFT_COLOR(65, 65, 65)   // disabled
    };

    TFT_eSPI *pTft;       // Itt pointert használunk a dinamikus tömbök miatt (nem lehet null referenciát használni)
    uint16_t x, y, w, h;  // A gomb pozíciója
    uint8_t id;           // A gomb ID-je
    const char *label;    // A gomb felirata
    ButtonState state;    // Állapota
    ButtonState oldState; // Előző állapota
    ButtonType type;      // Típusa
    bool buttonPressed;   // Flag a gomb nyomva tartásának követésére

    /**
     * Ezt nyomták meg?
     */
    inline bool contains(uint16_t tx, uint16_t ty) {
        return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
    }

    /**
     * Lenyomták a gombot
     */
    void pressed() {
        buttonPressed = true;
        oldState = state;
        state = ButtonState::HOLD;
        draw();
    }

    /**
     * Felengedték a gombot
     */
    void released() {
        buttonPressed = false;
        if (type == ButtonType::Toggleable) {
            state = (oldState == ButtonState::Off) ? ButtonState::On : ButtonState::Off;
        } else {
            state = ButtonState::Off;
        }
        oldState = state;

        draw();
    }

    /**
     * Benyomott gomb háttérszín gradiens
     */
    inline uint16_t darkenColor(uint16_t color, uint8_t amount) const {
        // Kivonjuk a piros, zöld és kék színösszetevőket
        uint8_t r = (color & 0xF800) >> 11;
        uint8_t g = (color & 0x07E0) >> 5;
        uint8_t b = (color & 0x001F);

        // Finomítjuk a csökkentési mértéket, figyelembe véve a színösszetevők közötti eltéréseket
        uint8_t darkenAmount = amount > 0 ? (amount >> 3) : 0;

        // A csökkentésnél biztosítjuk, hogy ne menjenek 0 alá az értékek
        r = (r > darkenAmount) ? r - darkenAmount : 0;
        g = (g > darkenAmount) ? g - darkenAmount : 0;
        b = (b > darkenAmount) ? b - darkenAmount : 0;

        // Visszaalakítjuk a színt 16 bites RGB formátumba
        return (r << 11) | (g << 5) | b;
    }

public:
    /**
     * Default konstruktor
     * (pl.: a dinamikus tömb deklarációhoz)
     */
    TftButton() {}

    /**
     * Konstruktor
     */
    TftButton(uint8_t id, TFT_eSPI &tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *label, ButtonType type, ButtonState state = ButtonState::Off)
        : id(id), pTft(&tft), x(x), y(y), w(w), h(h), label(label), type(type), buttonPressed(false), state(state), oldState(state) {}

    /**
     * Konstruktor X/Y pozíció nélkül
     * Automatikus elrendezéshez csak a szélesség és a magasság van megadva
     */
    TftButton(uint8_t id, TFT_eSPI &tft, uint16_t w, uint16_t h, const char *label, ButtonType type, ButtonState state = ButtonState::Off)
        : id(id), pTft(&tft), x(0), y(0), w(w), h(h), label(label), type(type), buttonPressed(false), state(state), oldState(state) {}

    /**
     * Destruktor
     */
    virtual ~TftButton() {
    }

    /**
     * Button szélességének lekérése
     */
    inline uint8_t getWidth() {
        return w;
    }

    /**
     * Button x/y pozíciójának beállítása
     * @param x
     * @param y
     */
    inline void setPosition(uint16_t x, uint16_t y) {
        this->x = x;
        this->y = y;
    }

    /**
     * A button kirajzolása
     */
    void draw() {
        // A gomb teljes szélességét és magasságát kihasználó sötétedés -> benyomás hatás keltés
        if (buttonPressed) {
            uint8_t stepWidth = w / TFT_BUTTON_DARKEN_COLORS_STEPS;
            uint8_t stepHeight = h / TFT_BUTTON_DARKEN_COLORS_STEPS;
            for (uint8_t i = 0; i < TFT_BUTTON_DARKEN_COLORS_STEPS; i++) {
                uint16_t fadedColor = darkenColor(TFT_BUTTON_STATE_BG_COLORS[oldState], i * 30); // Erősebb sötétítés
                pTft->fillRoundRect(x + i * stepWidth / 2, y + i * stepHeight / 2, w - i * stepWidth, h - i * stepHeight, 5, fadedColor);
            }
        } else {
            pTft->fillRoundRect(x, y, w, h, 5, TFT_BUTTON_STATE_BG_COLORS[state]);
        }

        // Ha tiltott, akkor sötétszürke a keret, ha aktív, akkor zöld, narancs ha nyomják
        pTft->drawRoundRect(x, y, w, h, 5, state == ButtonState::Disabled ? TFT_DARKGREY : state == ButtonState::On ? TFT_GREEN
                                                                                       : buttonPressed              ? TFT_ORANGE
                                                                                                                    : TFT_WHITE);

        // zöld a szöveg, ha aktív, narancs ha nyomják
        pTft->setTextColor(state == ButtonState::Disabled ? TFT_DARKGREY : state == ButtonState::On ? TFT_GREEN
                                                                       : buttonPressed              ? TFT_ORANGE
                                                                                                    : TFT_WHITE);

        // Az (x, y) koordináta a szöveg középpontja
        pTft->setTextDatum(MC_DATUM);

        // Fontváltás a gomb feliratozásához
        pTft->setFreeFont(&FreeSansBold9pt7b);
        pTft->setTextSize(1);
        pTft->setTextPadding(0);
        constexpr uint8_t BUTTON_LABEL_MARGIN_TOP = 3; // A felirat a gomb felső részéhez képest
        pTft->drawString(label, x + w / 2, y - BUTTON_LABEL_MARGIN_TOP + h / 2);

        // LED csík kirajzolása ha a gomb aktív vagy push, és nyomják
        uint16_t ledColor = 0;
        if (state == ButtonState::On) {
            // Ha On állapotú, akkor zöld a LED csík
            ledColor = TFT_GREEN;
        } else if (type == ButtonType::Pushable && buttonPressed) {
            // Ha Pushable típusú és épp nyomva tartják, akkor a LED narancs
            ledColor = TFT_ORANGE;
        } else if (type == ButtonType::Toggleable && state == ButtonState::Off) {
            // Ha Toggleable típusú és Off állapotú, akkor a LED sötétzöld
            ledColor = TFT_COLOR(5, 59, 19); // Sötétzöld
        }
        // Ha kell állítani a LED színt
        if (ledColor) {
            constexpr uint8_t BUTTON_LED_HEIGHT = 5;
            pTft->fillRect(x + 10, y + h - BUTTON_LED_HEIGHT - 3, w - 20, BUTTON_LED_HEIGHT, ledColor);
        }
    }

    /**
     * A gomb touch eseményeinek kezelése
     * @param touched Jelzi, hogy történt-e érintési esemény.
     * @param tx Az érintési esemény x-koordinátája.
     * @param ty Az érintési esemény y-koordinátája.
     */
    bool handleTouch(bool touched, uint16_t tx, uint16_t ty) {

        // Ha tiltott a gomb, akkor nem megyünk tovább
        if (state == ButtonState::Disabled) {
            return false;
        }

        // Ha van touch, de még nincs lenyomva a gomb, és erre a gombra jött a touch
        if (touched and !buttonPressed and contains(tx, ty)) {
            pressed();
        } else if (!touched and buttonPressed) {
            // Ha nincs ugyan touch, de ezt a gombot nyomva tartották eddig, akkor esemény van!!
            released();
            return true;
        }

        return false;
    }

    /**
     * ButtonTouchEvent legyártása
     */
    inline ButtonTouchEvent buildButtonTouchEvent() {
        return {id, label, type == ButtonType::Pushable ? ButtonState::Pushed : state};
    }

    /**
     * Button állapotának beállítása
     * @param newState új állapot
     */
    inline void setState(ButtonState newState) {
        if (state != newState) {
            state = newState;
            draw();
        }
    }

    /**
     * Button állapotának lekérése
     * @return állapot
     */
    inline ButtonState getState() const {
        return state;
    }

    /**
     *
     */
    inline uint8_t getId() const {
        return id;
    }

    /**
     *
     */
    inline const char *getLabel() const {
        return label;
    }

    /**
     * Button állapot
     */
    static const char *decodeState(ButtonState state) {

        switch (state) {
        case ButtonState::Off:
            return "Off";
        case ButtonState::On:
            return "On";
        case ButtonState::Disabled:
            return "Disabled";
        case ButtonState::HOLD:
            return "HOLD";
        case ButtonState::Pushed:
            return "Pushed";
        case ButtonState::UNKNOWN:
        default:
            return "UNKNOWN";
        }
    }
};

// == operátor túlterhelése (nem-tagfüggvény)
inline bool operator==(const TftButton::ButtonTouchEvent &lhs, const TftButton::ButtonTouchEvent &rhs) {
    return (lhs.id == rhs.id) && (lhs.state == rhs.state) && (lhs.label == rhs.label);
}

// != operátor túlterhelése (nem-tagfüggvény)
inline bool operator!=(const TftButton::ButtonTouchEvent &lhs, const TftButton::ButtonTouchEvent &rhs) {
    return !(lhs == rhs); // Ha nem egyenlő, akkor a == operátor segítségével negáljuk az eredményt
}

#endif //__TFTBUTTON_H