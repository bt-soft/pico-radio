#include "Config.h"

#include "Si4735Utils.h"

/**
 * Alapértelmezett readonly konfigurációs adatok
 */
const Config_t DEFAULT_CONFIG = {

    //-- Band
    .bandIdx = 0,         // Default band, FM
    .currentFreq = 9390,  // Petőfi 93.9MHz
    .currentStep = 10,    // Default step, 10kHz

    // BandWidht
    .bwIdxAM = 1,   // BandWidth AM
    .bwIdxFM = 0,   // BandWidth FM
    .bwIdxMW = 9,   // BandWidth MW
    .bwIdxSSB = 1,  // BandWidth SSB

    // Step
    .ssIdxMW = 9,
    .ssIdxAM = 5,
    .ssIdxFM = 10,

    // BFO
    .currentBFO = 0,
    .currentBFOStep = 25,
    .currentBFOmanu = 0,

    // Squelch
    .currentSquelch = 0,
    .squelchUsesRSSI = true,  // A squlech RSSI alapú legyen?

    // FM RDS
    .rdsEnabled = true,

    // Hangerő
    .currVolume = 50,

    // AGC
    .agcGain = static_cast<uint8_t>(Si4735Utils::AgcGainMode::Off),               // -> 0,
    .currentAGCgain = static_cast<uint8_t>(Si4735Utils::AgcGainMode::Automatic),  // -> 1

    //--- TFT
    //.tftCalibrateData = {0, 0, 0, 0, 0},  // TFT touch kalibrációs adatok
    .tftCalibrateData = {213, 3717, 234, 3613, 7},
    .tftBackgroundBrightness = TFT_BACKGROUND_LED_MAX_BRIGHTNESS,  // TFT Háttérvilágítás
    .tftDigitLigth = true,                                         // Inaktív szegmens látszódjon?
};
