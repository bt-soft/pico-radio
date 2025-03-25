#include "Si4735Utils.h"

#include "Band.h"
#include "Config.h"
#include "rtVars.h"

int8_t Si4735Utils::currentBandIdx = -1;  // Induláskor nincs kiválasztvba band

/**
 * Manage Squelch
 */
void Si4735Utils::manageSquelch() {

    // squelchIndicator(pCfg->vars.currentSquelch);
    if (!rtv::muteStat) {

        si4735.getCurrentReceivedSignalQuality();
        uint8_t rssi = si4735.getCurrentRSSI();
        uint8_t snr = si4735.getCurrentSNR();

        uint8_t signalQuality = config.data.squelchUsesRSSI ? rssi : snr;
        if (signalQuality >= config.data.currentSquelch) {

            if (rtv::SCANpause == true) {

                si4735.setAudioMute(false);
                rtv::squelchDecay = millis();
                DEBUG("Si4735Utils::manageSuelch ->  si4735.setAudioMute(false)\n");
            }
        } else {
            if (millis() > (rtv::squelchDecay + SQUELCH_DECAY_TIME)) {
                si4735.setAudioMute(true);
                DEBUG("Si4735Utils::manageSuelch -> si4735.setAudioMute(true)\n");
            }
        }
    }
}

/**
 * AGC beállítása
 */
void Si4735Utils::checkAGC() {

    // Először lekérdezzük az SI4735 chip aktuális AGC állapotát.
    //  Ez a hívás frissíti az SI4735 objektum belső állapotát az AGC-vel kapcsolatban (pl. hogy engedélyezve van-e vagy sem).
    si4735.getAutomaticGainControl();

    // Ha az AGC engedélyezve van
    if (si4735.isAgcEnabled()) {

        if (config.data.agcGain == static_cast<uint8_t>(AgcGainMode::Off)) {

            DEBUG("Si4735Utils::checkAGC() -> AGC Off\n");
            // A felhasználó az AGC kikapcsolását kérte.
            si4735.setAutomaticGainControl(1, 0);  // disabled

        } else if (config.data.agcGain == static_cast<uint8_t>(AgcGainMode::Manual)) {

            DEBUG("Si4735Utils::checkAGC() -> AGC Manual\n");
            // A felhasználó manuális AGC beállítást kért
            si4735.setAutomaticGainControl(1, config.data.currentAGCgain);
        }

    } else if (config.data.agcGain == static_cast<uint8_t>(AgcGainMode::Automatic)) {
        // Ha az AGC nincs engedélyezve az AGC, de a felhasználó az AGC engedélyezését kérte

        DEBUG("Si4735Utils::checkAGC() -> AGC Automatic\n");

        // Ez esetben az AGC-t engedélyezzük (0),
        //  és a csillapítást nullára állítjuk (0).
        // Ez a teljesen automatikus AGC működést jelenti.
        si4735.setAutomaticGainControl(0, 0);  // enabled
    }
}

/**
 * Loop függvény
 */
void Si4735Utils::loop() {
    //
    // manageSquelch();
}

/**
 * Konstruktor
 */
Si4735Utils::Si4735Utils(SI4735& si4735) : si4735(si4735), audioMut(false), elapsedAudMut(millis()) {

    DEBUG("Si4735Utils::Si4735Utils\n");

    // Band init, ha változott az épp használt band
    if (currentBandIdx != config.data.bandIdx) {

        // A Band  visszaállítása a konfiogból
        band.BandInit(currentBandIdx == -1);  // Rendszer induláskor -1 a currentBandIdx változást figyelő flag
        band.BandSet();

        // Hangerő beállítása
        si4735.setVolume(config.data.currVolume);

        currentBandIdx = config.data.bandIdx;
    }

    // Rögtön be is állítjuk az AGC-t
    checkAGC();
}

/**
 *
 */
void Si4735Utils::setStep() {

    // This command should work only for SSB mode
    BandTable& currentBand = band.getCurrentBand();
    uint8_t currMod = currentBand.varData.currMod;

    if (rtv::bfoOn && (currMod == LSB or currMod == USB or currMod == CW)) {
        if (config.data.currentBFOStep == 1)
            config.data.currentBFOStep = 10;
        else if (config.data.currentBFOStep == 10)
            config.data.currentBFOStep = 25;
        else
            config.data.currentBFOStep = 1;
    }

    if (!rtv::SCANbut) {
        band.useBand();
        checkAGC();
    }
}

/**
 *
 */
void Si4735Utils::MuteAudOn() {

    si4735.setHardwareAudioMute(1);
    audioMut = true;
    elapsedAudMut = millis();
}

/**
 *
 */
void Si4735Utils::MuteAud() {
#define MIN_ELAPSED_AudMut_TIME 0  // Noise surpression SSB in mSec. 0 mSec = off //Was 0 (LWH)

    // Stop muting only if this condition has changed
    if (((millis() - elapsedAudMut) > MIN_ELAPSED_AudMut_TIME) and (audioMut = true)) {
        audioMut = false;
        si4735.setHardwareAudioMute(0);
    }
}
