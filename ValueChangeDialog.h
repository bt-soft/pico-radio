#ifndef __VALUECHANGEDIALOG_H
#define __VALUECHANGEDIALOG_H

#include <functional>
#include <variant>

#include "MessageDialog.h"

/**
 *
 */
class ValueChangeDialog : public MessageDialog {
   public:
    enum class ValueType { Uint8, Integer, Float, Boolean };

   private:
    ValueType valueType;                                    // A változtatott érték típusa
    std::variant<uint8_t, int, float, bool> value;          // A kezelt érték
    std::variant<uint8_t, int, float, bool> originalValue;  // Eredeti érték
    double minVal, maxVal, step;                            // step- és határértékek

    // Általános callback függvény
    std::function<void(double)> onValueChanged;

    /**
     * Ha eltérő az eredeti és a jelenlegi érték, akkor azt más színnel jelenítjük meg
     */
    uint16_t getTextColor() { return (value == originalValue) ? TFT_COLOR(40, 64, 128) : TFT_WHITE; }

    /**
     * Eredeti érték visszaállítása
     */
    void restoreOriginalValue() { value = originalValue; }

    /**
     * Érték kirajzolása
     */
    void drawValue() {
        tft.setTextSize(2);
        tft.setCursor(x + 50, contentY + 30);
        tft.fillRect(x + 30, contentY, w - 40, 40, DLG_BACKGROUND_COLOR);  // Korábbi érték törlése
        tft.setTextColor(getTextColor(), DLG_BACKGROUND_COLOR);

        if (std::holds_alternative<uint8_t>(value)) {
            tft.print(std::get<uint8_t>(value));
        } else if (std::holds_alternative<int>(value)) {
            tft.print(std::get<int>(value));
        } else if (std::holds_alternative<float>(value)) {
            tft.print(std::get<float>(value), 2);
        } else if (std::holds_alternative<bool>(value)) {
            tft.print(std::get<bool>(value) ? "ON" : "OFF");
        }
    }

   public:
    /**
     * Generikus konstruktor
     */
    template <typename T>
    ValueChangeDialog(IDialogParent *pParent, TFT_eSPI &tft, uint16_t w, uint16_t h, const __FlashStringHelper *title, const __FlashStringHelper *message, T *valuePtr, T minVal,
                      T maxVal, T step, std::function<void(double)> callback = nullptr)
        : MessageDialog(pParent, tft, w, h, title, message, "OK", "Cancel"),
          minVal(static_cast<double>(minVal)),
          maxVal(static_cast<double>(maxVal)),
          step(static_cast<double>(step)) {

        if constexpr (std::is_same_v<T, uint8_t>) {
            valueType = ValueType::Uint8;
            value = *valuePtr;
            originalValue = *valuePtr;
        } else if constexpr (std::is_same_v<T, int>) {
            valueType = ValueType::Integer;
            value = *valuePtr;
            originalValue = *valuePtr;
        } else if constexpr (std::is_same_v<T, float>) {
            valueType = ValueType::Float;
            value = *valuePtr;
            originalValue = *valuePtr;
        } else if constexpr (std::is_same_v<T, bool>) {  // Hozzáadva: bool kezelése
            valueType = ValueType::Boolean;
            value = *valuePtr;
            originalValue = *valuePtr;
            this->step = 1;    // a bool-nak nincs lépésköze, de a kód igényli
            this->minVal = 0;  // a bool-nak nincs minimuma, de a kód igényli
            this->maxVal = 1;  // a bool-nak nincs maximuma, de a kód igényli
        }

        // Callbackot általánosítjuk
        onValueChanged = callback;

        drawDialog();
    }

    /**
     * Destruktor
     */
    ~ValueChangeDialog() {}

    /**
     * Dialog kirajzolása
     */
    void drawDialog() override { drawValue(); }

    /**
     * Rotary handler
     */
    bool handleRotary(RotaryEncoder::EncoderState encoderState) override {

        // Ha nincs változás akkor nem megyünk tovább
        if (encoderState.direction == RotaryEncoder::Direction::None) return false;

        // Az érték változtatása a Rotary irányának megfelelően
        if (valueType == ValueType::Uint8) {
            uint8_t &val = std::get<uint8_t>(value);
            val = (encoderState.direction == RotaryEncoder::Direction::Up) ? std::min(static_cast<uint8_t>(val + step), static_cast<uint8_t>(maxVal))
                                                                           : std::max(static_cast<uint8_t>(val - step), static_cast<uint8_t>(minVal));
        } else if (valueType == ValueType::Integer) {
            int &val = std::get<int>(value);
            val = (encoderState.direction == RotaryEncoder::Direction::Up) ? std::min(static_cast<int>(val + step), static_cast<int>(maxVal))
                                                                           : std::max(static_cast<int>(val - step), static_cast<int>(minVal));
        } else if (valueType == ValueType::Float) {
            float &val = std::get<float>(value);
            val = (encoderState.direction == RotaryEncoder::Direction::Up) ? std::min(static_cast<float>(val + step), static_cast<float>(maxVal))
                                                                           : std::max(static_cast<float>(val - step), static_cast<float>(minVal));
        } else if (valueType == ValueType::Boolean) {
            bool &val = std::get<bool>(value);
            val = (encoderState.direction == RotaryEncoder::Direction::Up);
        }

        // Kiírjuk az új értéket
        drawValue();

        // Ha van callback, akkor azt meghívjuk
        if (onValueChanged) {
            if (valueType == ValueType::Uint8) {
                onValueChanged(std::get<uint8_t>(value));
            } else if (valueType == ValueType::Integer) {
                onValueChanged(std::get<int>(value));
            } else if (valueType == ValueType::Float) {
                onValueChanged(std::get<float>(value));
            } else if (valueType == ValueType::Boolean) {
                onValueChanged(std::get<bool>(value));
            }
        }

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
            }

            // Ha van callback, akkor azt meghívjuk
            if (onValueChanged) {
                if (valueType == ValueType::Uint8) {
                    onValueChanged(std::get<uint8_t>(value));
                } else if (valueType == ValueType::Integer) {
                    onValueChanged(std::get<int>(value));
                } else if (valueType == ValueType::Float) {
                    onValueChanged(std::get<float>(value));
                } else if (valueType == ValueType::Boolean) {  // Hozzáadva: bool kezelése
                    onValueChanged(std::get<bool>(value));
                }
            }
            return true;
        }
        return false;
    }
};

#endif  // __VALUECHANGEDIALOG_H
