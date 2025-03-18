#include <Arduino.h>

#include "utils.h"

//------------------ TFT
#include <TFT_eSPI.h>  // TFT_eSPI könyvtár
TFT_eSPI tft;          // TFT objektum

//------------------- Rotary Encoder (Nem használunk hardware timert, az interrupt védelmekkel nem akarunk foglalkozni)
#include "RotaryEncoder.h"
RotaryEncoder rotaryEncoder = RotaryEncoder(PIN_ENCODER_CLK, PIN_ENCODER_DT, PIN_ENCODER_SW);

//------------------- EEPROM Config
#include "Config.h"
Config config;

//------------------- Memória információk megjelenítése
#ifdef __DEBUG
#include "PicoMemoryInfo.h"
#endif

//------------------- Képernyő
#include "AmDisplay.h"
#include "FmDisplay.h"
#include "FreqScanDisplay.h"
#include "ScreenSaverDisplay.h"
DisplayBase *pDisplay = nullptr;

/**
 * GLobális változó az aktuális kijelző váltásának előjegyzése
 * Induláskor FM - módban indulunk
 * (Ezt a globális változót a képernyők állítgatják, ha más képernyőt választ a felhasználó)
 */
DisplayBase::DisplayType displayChange = DisplayBase::DisplayType::fm;

// A képernyővédő elindulása előtti screen, majd erre állunk vissza
DisplayBase::DisplayType displayBeforeScreenSaver = DisplayBase::DisplayType::none;

/**
 * Aktuális kijelző váltása
 * a loop()-ból hívjuk, ha van váltási igény
 */
void changeDisplay() {

    // Ha volt aktuális képernyő, akkor azt töröljük
    if (pDisplay) {
        delete pDisplay;
    }

    // Létrehozzuk a kijelző példányát
    switch (displayChange) {

        case DisplayBase::DisplayType::fm:
            pDisplay = new FmDisplay(tft);
            break;

        case DisplayBase::DisplayType::am:
            pDisplay = new AmDisplay(tft);
            break;

        case DisplayBase::DisplayType::freqScan:
            pDisplay = new FreqScanDisplay(tft);
            break;

        case DisplayBase::DisplayType::screenSaver:
            pDisplay = new ScreenSaverDisplay(tft);
            break;
    }

    // Megjeleníttetjük a képernyőt
    pDisplay->drawScreen();

    // Jelezzük, hogy nem akarunk képernyőváltást már
    displayChange = DisplayBase::DisplayType::none;
}

/** ----------------------------------------------------------------------------------------------------------------------------------------
 *  Arduino Setup
 */
void setup() {
#ifdef __DEBUG
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    // Beeper
    pinMode(PIN_BEEPER, OUTPUT);
    digitalWrite(PIN_BEEPER, LOW);

    // TFT LED háttérvilágítás kimenet
    pinMode(PIN_DISPLAY_LED, OUTPUT);
    digitalWrite(PIN_DISPLAY_LED, 0);

    // Rotary Encoder beállítása
    rotaryEncoder.setDoubleClickEnabled(true);
    rotaryEncoder.setAccelerationEnabled(true);

    // TFT inicializálása
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Várakozás a soros port megnyitására
    // Utils::debugWaitForSerial(tft);

    // Ha a bekapcsolás alatt nyomva tartjuk a rotary gombját, akkor töröljük a konfigot
    if (digitalRead(PIN_ENCODER_SW) == LOW) {
        Utils::beepTick();
        delay(1500);
        if (digitalRead(PIN_ENCODER_SW) == LOW) {  // Ha még mindig nyomják
            config.loadDefaults();
            Utils::beepTick();
            DEBUG("Default settings resored!\n");
        }
    } else {
        // konfig betöltése
        config.load();
    }
    // Kell kalibrálni a TFT Touch-t?
    if (Utils::isZeroArray(config.data.tftCalibrateData)) {
        Utils::beepError();
        Utils::tftTouchCalibrate(tft, config.data.tftCalibrateData);
    }
    // Beállítjuk a touch scren-t
    tft.setTouch(config.data.tftCalibrateData);

    // Kezdő mód képernyőjének megjelenítése
    changeDisplay();

    // Csippantunk egyet
    Utils::beepTick();
}

/** ----------------------------------------------------------------------------------------------------------------------------------------
 *  Arduino Loop
 */
void loop() {

    // Ha kell display-t váltani, akkor azt itt tesszük meg
    if (::displayChange != DisplayBase::DisplayType::none) {
        changeDisplay();
    }

//------------------- Rotary Encoder Service
#define ROTARY_ENCODER_SERVICE_INTERVAL 5  // 5msec
    static uint32_t lastRotaryEncoderService = 0;
    if (millis() - lastRotaryEncoderService >= ROTARY_ENCODER_SERVICE_INTERVAL) {
        rotaryEncoder.service();
        lastRotaryEncoderService = millis();
    }
//------------------- EEprom mentés figyelése
#define EEPROM_SAVE_CHECK_INTERVAL 5 * 60 * 1000  // 5 perc
    static uint32_t lastEepromSaveCheck = 0;
    if (millis() - lastEepromSaveCheck >= EEPROM_SAVE_CHECK_INTERVAL) {
        config.checkSave();
        lastEepromSaveCheck = millis();
    }
//------------------- Memória információk megjelenítése
#ifdef __DEBUG
#define MEMORY_INFO_INTERVAL 20 * 1000  // 20mp
    static uint32_t lasDebugMemoryInfo = 0;
    if (millis() - lasDebugMemoryInfo >= MEMORY_INFO_INTERVAL) {
        debugMemoryInfo();
        lasDebugMemoryInfo = millis();
    }
#endif

    // Rotary Encoder olvasása
    RotaryEncoder::EncoderState encoderState = rotaryEncoder.read();
    // Ha folyamatosan nyomva tartják a rotary gombját akkor kikapcsolunk
    if (encoderState.buttonState == RotaryEncoder::ButtonState::Held) {
        // TODO: Kikapcsolás figyelését még implementálni
        DEBUG("Ki kellene kapcsolni...\n");
        Utils::beepError();
        delay(1000);
        return;
    }

    try {

        // Aktuális Display loopja
        bool touched = pDisplay->loop(encoderState);

#define SCREEN_SAVER_TIME 1000 * 60 * 1  // 5 perc
        static uint32_t lastScreenSaver = millis();
        // Ha volt touch valamelyik képernyőn, vagy volt rotary esemény...
        // Volt felhasználói interakció?
        bool userInteraction = (touched or encoderState.buttonState != RotaryEncoder::Open or encoderState.direction != RotaryEncoder::Direction::None);

        if (userInteraction) {

            // Ha van interakció, megnézzük, hogy képernyővédőn történt-e
            if (pDisplay->getDisplayType() == DisplayBase::DisplayType::screenSaver) {
                // Ha képernyővédőn volt az interakció, visszaállítjuk az előző képernyőt
                ::displayChange = ::displayBeforeScreenSaver;                 // Bejegyezzük visszaállításra a korábbi képernyőt
                ::displayBeforeScreenSaver = DisplayBase::DisplayType::none;  // töröljük a korábbi képernyő mentését
            }

            // Minden esetben frissítjük a timeoutot
            lastScreenSaver = millis();

        } else {

            // Ha nincs interakció, megnézzük, hogy lejárt-e a timeout
            if (millis() - lastScreenSaver >= SCREEN_SAVER_TIME) {

                // Ha letelt a timeout és nem a képernyővédőn vagyunk, elindítjuk a képernyővédőt
                if (pDisplay->getDisplayType() != DisplayBase::DisplayType::screenSaver) {
                    displayBeforeScreenSaver = pDisplay->getDisplayType();
                    displayChange = DisplayBase::DisplayType::screenSaver;

                } else {
                    // ha a screen saver már fut, akkor a timeout-ot frissítjük
                    lastScreenSaver = millis();
                }
            }
        }

    } catch (const std::exception &e) {
        Utils::beepError();
        String msg = "root::pDisplay->handleLoop() függvényben: '";
        msg += e.what();
        msg += "'";
        Utils::displayException(tft, msg.c_str());
    } catch (...) {
        Utils::displayException(tft, "root::pDisplay->handleLoop() függvényben: 'ismeretlen hiba'");
    }
}
