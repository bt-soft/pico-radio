#ifndef __VALUECHANGEDIALOG_H
#define __VALUECHANGEDIALOG_H

#include "MessageDialog.h"

/**
 *
 */
class ValueChangeDialog : public MessageDialog {
   public:
    enum class ValueType { Boolean, Integer, Float };

   private:
    void *valuePtr;
    ValueType valueType;
    int minValue, maxValue, stepInt;
    float minValueF, maxValueF, stepFloat;

    /**
     *
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

        // Ki is rajzoljuk a dialógust
        drawDialog();
    }

    /**
     * Integer
     */
    ValueChangeDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, int *value, int minVal,
                      int maxVal, int step)
        : MessageDialog(pParent, tft, w, h, title, message), valuePtr(value), valueType(ValueType::Integer), minValue(minVal), maxValue(maxVal), stepInt(step) {

        // Ki is rajzoljuk a dialógust
        drawDialog();
    }

    /**
     * Float
     */
    ValueChangeDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, float *value,
                      float minVal, float maxVal, float step)
        : MessageDialog(pParent, tft, w, h, title, message), valuePtr(value), valueType(ValueType::Float), minValueF(minVal), maxValueF(maxVal), stepFloat(step) {

        // Ki is rajzoljuk a dialógust
        drawDialog();
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
};

#endif  // __VALUECHANGEDIALOG_H
