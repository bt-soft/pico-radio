#ifndef __SI4735UTILS_H
#define __SI4735UTILS_H

#include <SI4735.h>

#include "Config.h"
#include "rtVars.h"

/**
 * si4735 utilities
 */
class Si4735Utils {

   public:
    // AGC beállítási lehetőségek
    enum class AgcGainMode : uint8_t {
        Off = 0,        // AGC kikapcsolva (de technikailag aktív marad, csak a csillapítás 0)
        Automatic = 1,  // AGC engedélyezve (teljesen automatikus működés)
        Manual = 2      // AGC manuális beállítással (a config.data.currentAGCgain értékével)
    };

   protected:
    // SI4735
    SI4735 &si4735;

    /**
     * Manage Squelch
     */
    void manageSuelch() {

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
    void checkAGC() {

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
     * Arduino loop
     */
    inline void loop() {
        // manageSuelch();
    }

   public:
    /**
     * Konstruktor
     */
    Si4735Utils(SI4735 &si4735) : si4735(si4735) {
        // Rögtön be is állítjuk az AGC-t
        checkAGC();
    }
};

#endif  //__SI4735UTILS_H
