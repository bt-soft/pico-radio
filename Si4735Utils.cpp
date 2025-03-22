#include "Si4735Utils.h"

#include "Band.h"
#include "Config.h"
#include "rtVars.h"

int8_t Si4735Utils::currentBandIdx = -1;  // Induláskor nincs kiválasztvba band

/**
 * Manage Squelch
 */
void Si4735Utils::manageSuelch() {

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

            DEBUG("AGC Off\n");
            // A felhasználó az AGC kikapcsolását kérte.
            si4735.setAutomaticGainControl(1, 0);  // disabled

        } else if (config.data.agcGain == static_cast<uint8_t>(AgcGainMode::Manual)) {

            DEBUG("AGC Manual\n");
            // A felhasználó manuális AGC beállítást kért
            si4735.setAutomaticGainControl(1, config.data.currentAGCgain);
        }

    } else if (config.data.agcGain == static_cast<uint8_t>(AgcGainMode::Automatic)) {
        // Ha az AGC nincs engedélyezve az AGC, de a felhasználó az AGC engedélyezését kérte

        DEBUG("AGC Automatic\n");

        // Ez esetben az AGC-t engedélyezzük (0),
        //  és a csillapítást nullára állítjuk (0).
        // Ez a teljesen automatikus AGC működést jelenti.
        si4735.setAutomaticGainControl(0, 0);  // enabled
    }
}

/**
 * Konstruktor
 */
Si4735Utils::Si4735Utils(SI4735& si4735) : si4735(si4735) {

    DEBUG("Si4735Utils::Si4735Utils\n");

    // Band init, ha változott az épp használt band
    if (currentBandIdx != config.data.bandIdx) {
        band.BandInit();
        band.BandSet();

        // Hangerő beállítása
        si4735.setVolume(config.data.currVolume);

        currentBandIdx = config.data.bandIdx;
    }

    // Rögtön be is állítjuk az AGC-t
    checkAGC();
}
