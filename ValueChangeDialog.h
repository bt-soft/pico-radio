#ifndef __VALUECHANGEDIALOG_H
#define __VALUECHANGEDIALOG_H

#include <functional>

#include "MessageDialog.h"

/**
 *
 */
class ValueChangeDialog : public MessageDialog {
   public:
    enum class ValueType { Boolean, Uint8, Integer, Float };

   private:
    ValueType valueType;                    // A változtatott érték típusa
    void *valuePtr;                         // A recegtettett érték
    void *originalValuePtr;                 // Eredeti érték
    int minValue, maxValue, stepInt;        // int step- és határértékek
    float minValueF, maxValueF, stepFloat;  // float step- és határértékek

    // Általános callback függvény
    std::function<void()> onValueChanged;

    /**
     * Típushelyes értékmásolás
     */
    template <typename T>
    inline void copyValue(T *src, T &dst) {
        dst = *src;
    }

    /**
     * Ha eltérő az eredeti és a jelenlegi érték, akkor azt más színnel jelenítjük meg
     */
    template <typename T>
    inline uint16_t getTextColor(T *original, T *current) {
        return (*original == *current) ? TFT_COLOR(40, 64, 128) : TFT_WHITE;
    }

    /**
     * Eredeti érték visszaállítása
     */
    void restoreOriginalValue() {
        switch (valueType) {
            case ValueType::Boolean:
                *reinterpret_cast<bool *>(valuePtr) = *reinterpret_cast<bool *>(originalValuePtr);
                break;
            case ValueType::Uint8:
                *reinterpret_cast<uint8_t *>(valuePtr) = *reinterpret_cast<uint8_t *>(originalValuePtr);
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
     * Érték kirajzolása
     */
    void drawValue() {
        tft.setTextSize(2);

#define VALUECHANGE_X_OFFSET 50
        tft.setCursor(x + VALUECHANGE_X_OFFSET, contentY + 30);
        tft.fillRect(x + VALUECHANGE_X_OFFSET, contentY, w - VALUECHANGE_X_OFFSET - 10, 40, DLG_BACKGROUND_COLOR);
#undef VALUECHANGE_X_OFFSET  // már nem kell

        switch (valueType) {
            case ValueType::Boolean:
                tft.setTextColor(getTextColor(reinterpret_cast<bool *>(originalValuePtr), reinterpret_cast<bool *>(valuePtr)), DLG_BACKGROUND_COLOR);
                tft.print(*reinterpret_cast<bool *>(valuePtr) ? F("On") : F("Off"));
                break;
            case ValueType::Uint8:
                tft.setTextColor(getTextColor(reinterpret_cast<uint8_t *>(originalValuePtr), reinterpret_cast<uint8_t *>(valuePtr)), DLG_BACKGROUND_COLOR);
                tft.print(*reinterpret_cast<uint8_t *>(valuePtr));
                break;
            case ValueType::Integer:
                tft.setTextColor(getTextColor(reinterpret_cast<int *>(originalValuePtr), reinterpret_cast<int *>(valuePtr)), DLG_BACKGROUND_COLOR);
                tft.print(*reinterpret_cast<int *>(valuePtr));
                break;
            case ValueType::Float:
                tft.setTextColor(getTextColor(reinterpret_cast<float *>(originalValuePtr), reinterpret_cast<float *>(valuePtr)), DLG_BACKGROUND_COLOR);
                tft.print(*reinterpret_cast<float *>(valuePtr), 2);
                break;
        }
    }

   public:
    /**
     * Generikus konstruktor
     */
    template <typename T>
    ValueChangeDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, T *value, T minVal,
                      T maxVal, T step, std::function<void()> callback = nullptr)  // Módosítva: std::function<void()>
        : MessageDialog(pParent, tft, w, h, title, message, "OK", "Cancel"),
          valuePtr(value),
          minValue(static_cast<int>(minVal)),
          maxValue(static_cast<int>(maxVal)),
          stepInt(static_cast<int>(step)),
          minValueF(static_cast<float>(minVal)),
          maxValueF(static_cast<float>(maxVal)),
          stepFloat(static_cast<float>(step)) {

        if constexpr (std::is_same_v<T, bool>) {
            valueType = ValueType::Boolean;
            originalValuePtr = new bool(*value);
        } else if constexpr (std::is_same_v<T, uint8_t>) {
            valueType = ValueType::Uint8;
            originalValuePtr = new uint8_t(*value);
        } else if constexpr (std::is_same_v<T, int>) {
            valueType = ValueType::Integer;
            originalValuePtr = new int(*value);
        } else if constexpr (std::is_same_v<T, float>) {
            valueType = ValueType::Float;
            originalValuePtr = new float(*value);
        }

        // Callbackot általánosítjuk
        if (callback) {
            onValueChanged = [value, callback]() {  // Módosítva: Nincs paraméter
                if constexpr (std::is_same_v<T, bool>) {
                    callback();
                } else if constexpr (std::is_same_v<T, uint8_t>) {
                    callback();
                } else if constexpr (std::is_same_v<T, int>) {
                    callback();
                } else if constexpr (std::is_same_v<T, float>) {
                    callback();
                }
            };
        }

        drawDialog();
    }

    /**
     * Destruktor
     */
    ~ValueChangeDialog() {
        if (valueType == ValueType::Boolean)
            delete reinterpret_cast<bool *>(originalValuePtr);
        else if (valueType == ValueType::Uint8)
            delete reinterpret_cast<uint8_t *>(originalValuePtr);
        else if (valueType == ValueType::Integer)
            delete reinterpret_cast<int *>(originalValuePtr);
        else if (valueType == ValueType::Float)
            delete reinterpret_cast<float *>(originalValuePtr);
    }

    /**
     * Dialog kirajzolása
     */
    void drawDialog() override {
        // Nem kell meghívni az ősök metódusát!!
        // MessageDialog::drawDialog();
        drawValue();
    }

    /**
     * Rotary handler
     */
    bool handleRotary(RotaryEncoder::EncoderState encoderState) override {

        // Ős rotary handlerének meghívása
        if (MessageDialog::handleRotary(encoderState)) return true;

        // Ha nincs változás akkor nem megyünk tovább
        if (encoderState.direction == RotaryEncoder::Direction::None) return false;

        // Az érték változtatása a Rotary irányának megfelelően
        switch (valueType) {
            case ValueType::Boolean:
                *reinterpret_cast<bool *>(valuePtr) = encoderState.direction == RotaryEncoder::Direction::Up;
                break;
            case ValueType::Uint8:
                *reinterpret_cast<uint8_t *>(valuePtr) = (encoderState.direction == RotaryEncoder::Direction::Up)
                                                             ? min(*reinterpret_cast<uint8_t *>(valuePtr) + stepInt, static_cast<uint8_t>(maxValue))
                                                             : max(*reinterpret_cast<uint8_t *>(valuePtr) - stepInt, static_cast<uint8_t>(minValue));
                break;
            case ValueType::Integer:
                *reinterpret_cast<int *>(valuePtr) = (encoderState.direction == RotaryEncoder::Direction::Up) ? min(*reinterpret_cast<int *>(valuePtr) + stepInt, maxValue)
                                                                                                              : max(*reinterpret_cast<int *>(valuePtr) - stepInt, minValue);
                break;
            case ValueType::Float:
                *reinterpret_cast<float *>(valuePtr) = (encoderState.direction == RotaryEncoder::Direction::Up) ? min(*reinterpret_cast<float *>(valuePtr) + stepFloat, maxValueF)
                                                                                                                : max(*reinterpret_cast<float *>(valuePtr) - stepFloat, minValueF);
                break;
        }

        // Kiírjuk az új értéket
        drawValue();

        // Ha van callback, akkor azt meghívjuk
        if (onValueChanged) onValueChanged();  // Módosítva: Nincs paraméter

        return true;
    }

    /**
     * Touch esemény lekezelése
     */
    bool handleTouch(bool touched, uint16_t tx, uint16_t ty) override {

        if (MessageDialog::handleTouch(touched, tx, ty)) {

            // Az 'X'-etnyomták meg?
            if (DialogBase::pParent->isDialogResponseCancelOrCloseX()) {
                // Visszaállítani az eredeti értéket
                restoreOriginalValue();

                // Ha van callback, akkor azt meghívjuk
                if (onValueChanged) onValueChanged();  // Módosítva: Nincs paraméter
            }
            return true;
        }
        return false;
    }
};

#endif  // __VALUECHANGEDIALOG_H
