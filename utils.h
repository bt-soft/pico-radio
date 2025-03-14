#ifndef __UTILS_H
#define __UTILS_H

#include <TFT_eSPI.h>
#include <cstring> // strncpy miatt

//---- Pinouts ------------------------------------------
// I2C si4735
#define PIN_SI4735_I2C_SDA 8
#define PIN_SI4735_I2C_SCL 9
#define PIN_SI4735_RESET 10

// Rotary Encoder
#define PIN_ENCODER_CLK 16
#define PIN_ENCODER_DT 17
#define PIN_ENCODER_SW 18

// Others
#define PIN_DISPLAY_LED 21
#define PIN_AUDIO_MUTE 20
#define PIN_BEEPER 22

//--- Debug ---
#define __DEBUG // Debug mód bekapcsolása

//--- TFT colors ---

#define TFT_COLOR(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
// #define COMPLEMENT_COLOR(color) \
//     (TFT_COLOR((255 - ((color >> 16) & 0xFF)), (255 - ((color >> 8) & 0xFF)), (255 - (color & 0xFF))))
// #define PUSHED_COLOR(color) ((((color & 0xF800) >> 1) & 0xF800) | (((color & 0x07E0) >> 1) & 0x07E0) | (((color & 0x001F) >> 1) & 0x001F))

//--- Array Utils ---
#define ARRAY_ITEM_COUNT(array) (sizeof(array) / sizeof(array[0]))

//--- Debug ---
#ifdef __DEBUG
// #define DEBUG(fmt, ...) Serial.printf_P(PSTR(fmt "\n") __VA_OPT__(, ) __VA_ARGS__)
#define DEBUG(fmt, ...) Serial.printf_P(PSTR(fmt) __VA_OPT__(, ) __VA_ARGS__)
#else
#define DEBUG(fmt, ...) // Üres makró, ha __DEBUG nincs definiálva
#endif

//--- Utils ---
namespace Utils {
/**
 * Biztonságos string másolás
 * @param dest cél string
 * @param src forrás string
 */
template <typename T, size_t N>
void safeStrCpy(T (&dest)[N], const T *src) {
    // A strncpy használata a karakterlánc másolásához
    strncpy(dest, src, N - 1); // Csak N-1 karaktert másolunk, hogy ne lépjük túl a cél tömböt
    dest[N - 1] = '\0';        // Biztosítjuk, hogy a cél tömb nullával legyen lezárva
}

/**
 * Tömb elemei nullák?
 */
template <typename T, size_t N>
bool isZeroArray(T (&arr)[N]) {
    for (size_t i = 0; i < N; ++i) {
        if (arr[i] != 0) {
            return false; // Ha bármelyik elem nem nulla, akkor false-t adunk vissza
        }
    }
    return true; // Ha minden elem nulla, akkor true-t adunk vissza
}

/**
 * Várakozás a soros port megnyitására
 * @param pTft a TFT kijelző példánya
 */
void debugWaitForSerial(TFT_eSPI *pTft);

//--- TFT ---
void tftTouchCalibrate(TFT_eSPI *pTft, uint16_t (&calData)[5]);

//--- Beep ----
/**
 *  Pitty hangjelzés
 */
void beepTick();
void beepError();

} // namespace Utils

#endif // __UTILS_H
