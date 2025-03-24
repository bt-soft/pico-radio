#ifndef __SI4735UTILS_H
#define __SI4735UTILS_H

#include <SI4735.h>

/**
 * si4735 utilities
 */
class Si4735Utils {
   private:
    static int8_t currentBandIdx;
    bool audioMut;
    uint32_t elapsedAudMut;

   protected:
    // SI4735
    SI4735 &si4735;

    /**
     * Manage Squelch
     */
    void manageSquelch();

    /**
     * AGC beállítása
     */
    void checkAGC();

    /**
     * Arduino loop
     */
    void loop();

   public:
    // AGC beállítási lehetőségek
    enum class AgcGainMode : uint8_t {
        Off = 0,        // AGC kikapcsolva (de technikailag aktív marad, csak a csillapítás 0)
        Automatic = 1,  // AGC engedélyezve (teljesen automatikus működés)
        Manual = 2      // AGC manuális beállítással (a config.data.currentAGCgain értékével)
    };

    /**
     * Konstruktor
     */
    Si4735Utils(SI4735 &si4735);

    /**
     * Frequency Step set
     */
    void setStep();

    void MuteAud();
    void MuteAudOn();
};

#endif  //__SI4735UTILS_H
