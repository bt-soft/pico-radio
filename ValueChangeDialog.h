#ifndef __VALUECHANGEDIALOG_H
#define __VALUECHANGEDIALOG_H

#include "MessageDialog.h"

/**
 * Adatok (bool, int, float) értékének állítását végző dialóg
 */
class ValueChangeDialog : public MessageDialog {
   public:
    enum class ValueType { Boolean, Integer, Float };

   private:
    void *valuePtr;
    ValueType valueType;
    int minValue, maxValue, stepInt;
    float minValueF, maxValueF, stepFloat;

    // Az eredeti érték tárolása
    void *originalValuePtr;

    /**
     * Ez a metódus segít az eredeti érték másolatának létrehozásában
     */
    template <typename T>
    void copyValue(T *src, T &dst) {
        dst = *src;
    }

    /**
     * Az eredeti érték visszaállítása
     */
    void restoreOriginalValue() {

        switch (valueType) {
            case ValueType::Boolean:
                *reinterpret_cast<bool *>(valuePtr) = *reinterpret_cast<bool *>(originalValuePtr);
                break;

            case ValueType::Integer:
                *reinterpret_cast<int *>(valuePtr) = *reinterpret_cast<int *>(originalValuePtr);
                break;

            case ValueType::Float:
                *reinterpret_cast<float *>(valuePtr) = *reinterpret_cast<float *>(originalValuePtr);
                break;
        }
    }

    /**
     * Aktuális érték megjelenítése
     */
    void drawValue() {

        tft.setTextSize(2);
        tft.setCursor(x + 20, contentY + 30);
        tft.fillRect(x + 20, contentY, w - 40, 40, DLG_BACKGROUND_COLOR);

        switch (valueType) {
            case ValueType::Boolean: {
                auto val = *reinterpret_cast<bool *>(valuePtr);
                tft.print(val ? F("On") : F("Off"));
                break;
            }

            case ValueType::Integer:
                tft.print(*reinterpret_cast<int *>(valuePtr));
                break;

            case ValueType::Float:
                tft.print(*reinterpret_cast<float *>(valuePtr), 2);
                break;
        }
    }

   public:
    /**
     * Boolean
     */
    ValueChangeDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, bool *value)
        : MessageDialog(pParent, tft, w, h, title, message), valuePtr(value), valueType(ValueType::Boolean) {

        /// Tároljuk el az eredeti értéket
        originalValuePtr = new bool(*value);

        // Ki is rajzoljuk a dialógust
        drawDialog();
    }

    /**
     * Integer
     */
    ValueChangeDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, int *value, int minVal,
                      int maxVal, int step)
        : MessageDialog(pParent, tft, w, h, title, message), valuePtr(value), valueType(ValueType::Integer), minValue(minVal), maxValue(maxVal), stepInt(step) {

        // Tároljuk el az eredeti értéket
        originalValuePtr = new int(*value);

        // Ki is rajzoljuk a dialógust
        drawDialog();
    }

    /**
     * Float
     */
    ValueChangeDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, float *value,
                      float minVal, float maxVal, float step)
        : MessageDialog(pParent, tft, w, h, title, message), valuePtr(value), valueType(ValueType::Float), minValueF(minVal), maxValueF(maxVal), stepFloat(step) {

        // Tároljuk el az eredeti értéket
        originalValuePtr = new float(*value);

        // Ki is rajzoljuk a dialógust
        drawDialog();
    }

    /**
     * Destruktor: Töröljük az eredeti érték tárolóját
     */
    virtual ~ValueChangeDialog() {

        // Eredeti érték tárolójának törlése (void pointer a C++-ban nem törölhető, így előtte cast-olni kell)
        if (valueType == ValueType::Boolean) {
            delete reinterpret_cast<bool *>(originalValuePtr);

        } else if (valueType == ValueType::Integer) {
            delete reinterpret_cast<int *>(originalValuePtr);

        } else if (valueType == ValueType::Float) {
            delete reinterpret_cast<float *>(originalValuePtr);
        }
    }

    /**
     *
     */
    void drawDialog() override {
        MessageDialog::drawDialog();
        drawValue();
    }

    /**
     * Rotary encoder esemény lekezelése
     */
    bool handleRotary(RotaryEncoder::EncoderState encoderState) override {

        // A click eseményt az ősre bízzuk, ha lekezelte akkor kilépünk
        if (MessageDialog::handleRotary(encoderState)) {
            return true;
        }

        // Csak a tekergetésre reagálunk
        if (encoderState.direction == RotaryEncoder::Direction::None) {
            return false;
        }

        switch (valueType) {
            case ValueType::Boolean: {
                auto *val = reinterpret_cast<bool *>(valuePtr);
                *val = encoderState.direction == RotaryEncoder::Direction::Up;
                break;
            }

            case ValueType::Integer: {
                auto *val = reinterpret_cast<int *>(valuePtr);
                *val = (encoderState.direction == RotaryEncoder::Direction::Up) ? min(*val + stepInt, maxValue) : max(*val - stepInt, minValue);
                break;
            }

            case ValueType::Float: {
                auto *val = reinterpret_cast<float *>(valuePtr);
                *val = (encoderState.direction == RotaryEncoder::Direction::Up) ? min(*val + stepFloat, maxValueF) : max(*val - stepFloat, minValueF);
                break;
            }
        }
        drawValue();
        return true;
    }

    /**
     * Dialóg Touch esemény lekezelése
     */
    virtual bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override {

        // Ha az ős (MessageDialog az OK gombbal), vagy annak az őse (DialogBase az 'X' bezáró gombbal)
        // Lekezelte már a touch-ot, akkor megvizsgáljuk, hogy milyen gomb is kereült lenyomásra
        //  'X' esetén (ez a cancel esemény) vissza kell-e állítani az eredeti értéket
        if (MessageDialog::handleTouch(touched, tx, ty)) {

            // Lekérjük, hogy az ős mit állított be
            TftButton::ButtonTouchEvent event = DialogBase::pParent->getDialogResponse();

            // Ha 'Cancel'-t vagy 'X'-et nyomtak, akkor visszaállítjuk az eredeti értéket
            if (event.id == DLG_CLOSE_BUTTON_ID or event.id == DLG_CANCEL_BUTTON_ID) {
                // Itt állítjuk vissza az eredeti értéket...
                restoreOriginalValue();
            }

            return true;
        }

        // Ez a dialóg nem kezel touch eseményt, csak a Rotary-t használja
        return false;
    }
};

#endif  // __VALUECHANGEDIALOG_H
